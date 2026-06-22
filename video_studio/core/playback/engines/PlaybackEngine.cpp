#include "core/playback/engines/PlaybackEngine.h"

#include <QDir>
#include <QPainter>
#include <QDebug>
#include "core/effects/processors/EffectProcessor.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

namespace {

QString ffmpegErrorString(int errorCode)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(errorCode, buffer, sizeof(buffer));
    return QString::fromUtf8(buffer);
}

QByteArray ffmpegPath(const QString& filePath)
{
    return QDir::toNativeSeparators(filePath).toUtf8();
}

bool openInput(const QString& path, AVFormatContext** context, QString* error)
{
    const QByteArray nativePath = ffmpegPath(path);
    int result = avformat_open_input(context, nativePath.constData(), nullptr, nullptr);
    if (result < 0) {
        if (error) {
            *error = QStringLiteral("Could not open media: %1").arg(ffmpegErrorString(result));
        }
        return false;
    }

    if (path.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) ||
        path.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive) ||
        path.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive) ||
        path.endsWith(QStringLiteral(".webp"), Qt::CaseInsensitive) ||
        path.endsWith(QStringLiteral(".bmp"), Qt::CaseInsensitive)) {
        (*context)->max_analyze_duration = AV_TIME_BASE / 10;
        (*context)->probesize = 32768;
    }

    result = avformat_find_stream_info(*context, nullptr);
    if (result < 0) {
        if (error) {
            *error = QStringLiteral("Could not read streams: %1").arg(ffmpegErrorString(result));
        }
        avformat_close_input(context);
        return false;
    }

    return true;
}

double contextDurationSeconds(const AVFormatContext* context)
{
    if (!context || context->duration == AV_NOPTS_VALUE || context->duration <= 0) {
        return 0.0;
    }

    return static_cast<double>(context->duration) / AV_TIME_BASE;
}

double rationalToDouble(AVRational rational)
{
    if (rational.num <= 0 || rational.den <= 0) {
        return 0.0;
    }

    return av_q2d(rational);
}

std::int64_t secondsToStreamTimestamp(double seconds, AVRational timeBase)
{
    if (!std::isfinite(seconds) || seconds <= 0.0) {
        return 0;
    }

    const AVRational avTimeBase = {1, AV_TIME_BASE};
    const auto avTimestamp = static_cast<std::int64_t>(
        std::llround(static_cast<long double>(seconds) * AV_TIME_BASE));
    return av_rescale_q(avTimestamp, avTimeBase, timeBase);
}

double frameTimestampSeconds(const AVFrame* frame, const AVStream* stream)
{
    std::int64_t timestamp = frame->best_effort_timestamp;
    if (timestamp == AV_NOPTS_VALUE) {
        timestamp = frame->pts;
    }

    if (timestamp == AV_NOPTS_VALUE || !stream) {
        return -1.0;
    }

    return static_cast<double>(timestamp) * av_q2d(stream->time_base);
}

int bytesPerSample(AVSampleFormat format)
{
    return std::max(1, av_get_bytes_per_sample(format));
}

void applyPlaybackHeadroom(QByteArray* pcm)
{
    if (!pcm || pcm->isEmpty()) {
        return;
    }

    constexpr float playbackHeadroom = 0.70794576f; // -3 dB for AAC overs that would clip as buzz.
    constexpr float playbackLimit = 0.98f;
    auto* samples = reinterpret_cast<float*>(pcm->data());
    const qsizetype sampleCount = pcm->size() / static_cast<qsizetype>(sizeof(float));
    for (qsizetype i = 0; i < sampleCount; ++i) {
        samples[i] = std::clamp(samples[i] * playbackHeadroom, -playbackLimit, playbackLimit);
    }
}

QByteArray floatPcmToInt16(const QByteArray& pcm)
{
    const auto* floatSamples = reinterpret_cast<const float*>(pcm.constData());
    const qsizetype sampleCount = pcm.size() / static_cast<qsizetype>(sizeof(float));
    QByteArray converted(sampleCount * static_cast<qsizetype>(sizeof(std::int16_t)), Qt::Uninitialized);
    auto* intSamples = reinterpret_cast<std::int16_t*>(converted.data());

    for (qsizetype i = 0; i < sampleCount; ++i) {
        const int value = static_cast<int>(std::lround(floatSamples[i] * 32767.0f));
        intSamples[i] = static_cast<std::int16_t>(std::clamp(value, -32768, 32767));
    }

    return converted;
}

} // namespace

PlaybackEngine::PlaybackEngine(QQuickItem* parent)
    : QQuickPaintedItem(parent),
      m_audioEngine(this)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setAntialiasing(false);

    m_timer.setInterval(16);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &PlaybackEngine::tick);
    m_positionNotifyClock.start();
}

PlaybackEngine::~PlaybackEngine()
{
    closeDecoders();
}

bool PlaybackEngine::isVideoImage() const
{
    if (!m_videoFormat) {
        return false;
    }
    if (m_videoFormat->duration <= 0 || m_videoFormat->duration == AV_NOPTS_VALUE) {
        return true;
    }
    if (m_videoStreamIndex >= 0) {
        if (m_videoFormat->streams[m_videoStreamIndex]->nb_frames == 1) {
            return true;
        }
    }
    return false;
}

QString PlaybackEngine::clipName() const
{
    return m_clipName;
}

QString PlaybackEngine::filePath() const
{
    return m_filePath;
}

double PlaybackEngine::duration() const noexcept
{
    return m_duration;
}

double PlaybackEngine::sequenceDuration() const noexcept
{
    return m_sequenceDuration;
}

double PlaybackEngine::position() const noexcept
{
    return m_position;
}

bool PlaybackEngine::isPlaying() const noexcept
{
    return m_playing;
}

bool PlaybackEngine::isMuted() const noexcept
{
    return m_isMuted;
}

bool PlaybackEngine::isInClipRange(double timelineSeconds) const
{
    if (m_clipFileDuration <= 0.0 || m_filePath.isEmpty()) {
        return false;
    }
    const double clipTime = timelineSeconds - m_clipStartOffset;
    return clipTime >= -0.01 && clipTime < m_clipFileDuration + 0.01;
}

double PlaybackEngine::toClipTime(double timelineSeconds) const
{
    return (timelineSeconds - m_clipStartOffset) + m_sourceInPoint;
}

void PlaybackEngine::clearFrameToBlack()
{
    if (!m_frame.isNull() || !m_processedFrame.isNull()) {
        m_frame = {};
        m_processedFrame = {};
        m_lastVideoFrameSeconds = -1.0;
        update();
    }
}

void PlaybackEngine::setClipStartOffset(double offset)
{
    m_clipStartOffset = std::max(0.0, offset);
    if (m_clipFileDuration > 0.0) {
        m_duration = m_clipStartOffset + m_clipFileDuration;
        emit durationChanged();
    }
}

void PlaybackEngine::setSourceInPoint(double inPoint)
{
    m_sourceInPoint = std::max(0.0, inPoint);
}

void PlaybackEngine::setSequenceDuration(double duration)
{
    if (m_sequenceDuration != duration) {
        m_sequenceDuration = std::max(0.0, duration);
        emit sequenceDurationChanged();
    }
}

void PlaybackEngine::setIsMuted(bool muted)
{
    if (m_isMuted != muted) {
        m_isMuted = muted;
        emit isMutedChanged();
        if (m_isMuted) {
            setAudioLevels(0.0, 0.0);
            // Flush any already-decoded audio sitting in the write buffer
            m_pendingAudio.clear();
        }
    }
}

bool PlaybackEngine::hasVideo() const noexcept
{
    return m_hasVideo;
}

bool PlaybackEngine::hasAudio() const noexcept
{
    return m_hasAudio;
}

double PlaybackEngine::frameRate() const noexcept
{
    return m_frameRate;
}

double PlaybackEngine::audioLevelLeft() const noexcept
{
    return m_audioLevelLeft;
}

double PlaybackEngine::audioLevelRight() const noexcept
{
    return m_audioLevelRight;
}

int PlaybackEngine::sourceVideoWidth() const noexcept
{
    if (m_videoCodec && m_hasVideo) {
        return m_videoCodec->width;
    }
    return 0;
}

int PlaybackEngine::sourceVideoHeight() const noexcept
{
    if (m_videoCodec && m_hasVideo) {
        return m_videoCodec->height;
    }
    return 0;
}

QColor PlaybackEngine::backgroundColor() const noexcept
{
    return m_backgroundColor;
}

void PlaybackEngine::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
        update();
    }
}

void PlaybackEngine::paint(QPainter* painter)
{
    painter->fillRect(boundingRect(), m_backgroundColor);
    if (m_processedFrame.isNull()) {
        return;
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QSizeF itemSize = boundingRect().size();
    const qreal imagePixelRatio = std::max<qreal>(1.0, m_processedFrame.devicePixelRatio());
    QSizeF frameSize(static_cast<qreal>(m_processedFrame.width()) / imagePixelRatio,
                     static_cast<qreal>(m_processedFrame.height()) / imagePixelRatio);
    frameSize.scale(itemSize, Qt::KeepAspectRatio);

    const QRectF target((itemSize.width() - frameSize.width()) / 2.0,
                        (itemSize.height() - frameSize.height()) / 2.0,
                        frameSize.width(),
                        frameSize.height());

    painter->drawImage(target, m_processedFrame);
}

bool PlaybackEngine::loadClip(const QString& name,
                              const QString& path,
                              double knownDuration,
                              bool expectedVideo,
                              const QString& originalPath)
{
    const double savedPosition = m_position;
    pause();
    closeDecoders();

    m_clipName = name;
    m_filePath = path;
    m_clipFileDuration = std::max(0.0, knownDuration);
    m_duration = m_clipStartOffset + m_clipFileDuration;
    m_position = savedPosition;
    m_playStartPosition = savedPosition;
    m_lastVideoFrameSeconds = -1.0;
    m_lastPositionNotifyMs = -1;
    m_frameRate = 25.0;
    m_wasInClipRange = false;
    // m_sourceInPoint is set before loadClip via setSourceInPoint()
    setAudioLevels(0.0, 0.0);
    m_hasVideo = false;
    m_hasAudio = false;
    m_frame = {};
    m_processedFrame = {};
    update();

    bool openedAnything = false;
    if (expectedVideo) {
        openedAnything = openVideoDecoder(originalPath.isEmpty() ? path : originalPath);
    }

    const bool openedAudio = openAudioDecoder(path);
    openedAnything = openedAnything || openedAudio;

    if (!openedAnything) {
        reportError(QStringLiteral("Could not open playback streams for %1").arg(path));
        emit clipChanged();
        emit durationChanged();
        emit positionChanged();
        return false;
    }

    if (m_clipFileDuration <= 0.0) {
        if (m_videoFormat) {
            m_clipFileDuration = contextDurationSeconds(m_videoFormat);
        } else if (m_audioFormat) {
            m_clipFileDuration = contextDurationSeconds(m_audioFormat);
        }
        m_duration = m_clipStartOffset + m_clipFileDuration;
    }

    // Only show first frame if position is at the clip start
    if (m_clipStartOffset <= 0.01) {
        if (m_hasVideo) {
            seekVideo(m_sourceInPoint, true);
        }
        if (m_hasAudio) {
            seekAudio(m_sourceInPoint);
            m_audioEngine.pause();
        }
        m_wasInClipRange = true;
    } else {
        // Clip has an offset — don't show frame yet (position 0 is before clip)
        if (m_hasAudio) {
            m_audioEngine.pause();
        }
    }

    emit clipChanged();
    emit durationChanged();
    emit positionChanged();
    return true;
}

void PlaybackEngine::clear()
{
    pause();
    closeDecoders();
    m_clipName.clear();
    m_filePath.clear();
    m_duration = 0.0;
    m_position = 0.0;
    m_playStartPosition = 0.0;
    m_lastVideoFrameSeconds = -1.0;
    m_lastPositionNotifyMs = -1;
    m_clipStartOffset = 0.0;
    m_sourceInPoint = 0.0;
    m_clipFileDuration = 0.0;
    m_wasInClipRange = false;
    m_frame = {};
    m_processedFrame = {};
    setAudioLevels(0.0, 0.0);
    m_hasVideo = false;
    m_hasAudio = false;
    update();
    emit clipChanged();
    emit durationChanged();
    emit positionChanged();
}

void PlaybackEngine::play()
{
    if (m_filePath.isEmpty() || (!m_hasVideo && !m_hasAudio)) {
        return;
    }

    const double effectiveDuration = m_sequenceDuration > 0.0 ? m_sequenceDuration : m_duration;
    if (effectiveDuration > 0.0 && m_position >= effectiveDuration) {
        seek(m_sequenceDuration > 0.0 ? 0.0 : m_clipStartOffset);
    }

    m_playStartPosition = m_position;
    m_clock.restart();
    setPlaying(true);
    m_timer.start();

    // Only start audio if currently in the clip's range
    const double clipTime = toClipTime(m_position);
    if (m_hasAudio && isInClipRange(m_position)) {
        seekAudio(std::max(0.0, clipTime));
        m_audioEngine.resume();
        feedAudio();
    }
}

void PlaybackEngine::pause()
{
    if (m_playing) {
        setPosition(m_playStartPosition + static_cast<double>(m_clock.elapsed()) / 1000.0, true);
    }

    m_timer.stop();
    m_audioEngine.pause();
    setAudioLevels(0.0, 0.0);
    setPlaying(false);
}

void PlaybackEngine::togglePlayback()
{
    if (m_playing) {
        pause();
    } else {
        play();
    }
}

void PlaybackEngine::seek(double seconds)
{
    const bool wasPlaying = m_playing;
    if (wasPlaying) {
        pause();
    }

    const double effectiveDuration = m_sequenceDuration > 0.0 ? m_sequenceDuration : m_duration;
    const double clampedSeconds = effectiveDuration > 0.0
        ? std::clamp(seconds, 0.0, effectiveDuration)
        : std::max(0.0, seconds);

    setPosition(clampedSeconds, true);

    if (isInClipRange(clampedSeconds)) {
        const double clipTime = std::max(0.0, toClipTime(clampedSeconds));
        if (m_hasVideo) {
            seekVideo(clipTime, true);
        }
        if (m_hasAudio) {
            seekAudio(clipTime);
        }
        m_wasInClipRange = true;
    } else {
        // Outside clip range — show black, don't seek decoders
        clearFrameToBlack();
        m_wasInClipRange = false;
    }

    if (wasPlaying) {
        play();
    }
}

void PlaybackEngine::stepFrame(int frames)
{
    const double fps = m_frameRate > 0.0 ? m_frameRate : 25.0;
    seek(m_position + static_cast<double>(frames) / fps);
}

void PlaybackEngine::tick()
{
    if (!m_playing) {
        return;
    }

    const double targetPosition = m_playStartPosition + static_cast<double>(m_clock.elapsed()) / 1000.0;
    const double effectiveDuration = m_sequenceDuration > 0.0 ? m_sequenceDuration : m_duration;
    if (effectiveDuration > 0.0 && targetPosition >= effectiveDuration) {
        setPosition(effectiveDuration, true);
        pause();
        return;
    }

    setPosition(targetPosition);

    const bool inClip = isInClipRange(targetPosition);
    const double clipTime = std::max(0.0, toClipTime(targetPosition));

    // Handle transition from gap → clip
    if (inClip && !m_wasInClipRange) {
        m_wasInClipRange = true;
        // Just entered the clip, seek decoders to the right position
        if (m_hasVideo) {
            seekVideo(clipTime, true);
        }
        if (m_hasAudio) {
            seekAudio(clipTime);
            m_audioEngine.resume();
        }
        feedAudio();
        return;
    }

    // Handle transition from clip → gap
    if (!inClip && m_wasInClipRange) {
        m_wasInClipRange = false;
        clearFrameToBlack();
        m_audioEngine.pause();
        setAudioLevels(0.0, 0.0);
        return;
    }

    // In a gap — just advance position, show black
    if (!inClip) {
        return;
    }

    // In the clip — normal decode logic using clip-relative time
    if (m_hasVideo) {
        if (isVideoImage()) {
            m_lastVideoFrameSeconds = clipTime;
        } else {
            const double frameInterval = 1.0 / std::max(1.0, m_frameRate);
            if (m_lastVideoFrameSeconds >= 0.0
                && clipTime - m_lastVideoFrameSeconds > 1.0) {
                seekVideo(clipTime, true);
            } else {
                const double lagSeconds = m_lastVideoFrameSeconds >= 0.0
                    ? clipTime - m_lastVideoFrameSeconds
                    : 0.0;
                const double resyncThreshold = std::max(0.35, frameInterval * 12.0);
                if (lagSeconds > resyncThreshold) {
                    seekVideo(clipTime, true);
                } else {
                    int decodedFrames = 0;
                    QImage latestImage;
                    double latestFrameSeconds = -1.0;
                    while (decodedFrames < 4
                       && (m_lastVideoFrameSeconds < 0.0
                           || m_lastVideoFrameSeconds + frameInterval * 0.55 < clipTime)) {
                        QImage image;
                        double frameSeconds = 0.0;
                        if (!decodeNextVideoFrame(&image, &frameSeconds)) {
                            break;
                        }

                        latestFrameSeconds = frameSeconds >= 0.0 ? frameSeconds : clipTime;
                        latestImage = std::move(image);
                        m_lastVideoFrameSeconds = latestFrameSeconds;
                        ++decodedFrames;
                    }

                    if (!latestImage.isNull()) {
                        setFrame(std::move(latestImage), latestFrameSeconds);
                    }
                }
            }
        }
    }

    feedAudio();
}

void PlaybackEngine::closeDecoders()
{
    m_timer.stop();
    m_audioEngine.reset();
    m_pendingAudio.clear();

    if (m_sws) {
        sws_freeContext(m_sws);
        m_sws = nullptr;
    }
    if (m_swr) {
        swr_free(&m_swr);
    }
    if (m_videoPacket) {
        av_packet_free(&m_videoPacket);
    }
    if (m_audioPacket) {
        av_packet_free(&m_audioPacket);
    }
    if (m_videoFrame) {
        av_frame_free(&m_videoFrame);
    }
    if (m_audioFrame) {
        av_frame_free(&m_audioFrame);
    }
    if (m_videoCodec) {
        avcodec_free_context(&m_videoCodec);
    }
    if (m_audioCodec) {
        avcodec_free_context(&m_audioCodec);
    }
    if (m_videoFormat) {
        avformat_close_input(&m_videoFormat);
    }
    if (m_audioFormat) {
        avformat_close_input(&m_audioFormat);
    }

    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
    m_videoEof = false;
    m_audioEof = false;
}

bool PlaybackEngine::openVideoDecoder(const QString& path)
{
    QString error;
    if (!openInput(path, &m_videoFormat, &error)) {
        reportError(error);
        return false;
    }

    const int streamIndex = av_find_best_stream(m_videoFormat, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        return false;
    }

    AVStream* stream = m_videoFormat->streams[streamIndex];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        reportError(QStringLiteral("Could not find FFmpeg video decoder"));
        return false;
    }

    m_videoCodec = avcodec_alloc_context3(decoder);
    if (!m_videoCodec) {
        reportError(QStringLiteral("Could not allocate video decoder"));
        return false;
    }

    int result = avcodec_parameters_to_context(m_videoCodec, stream->codecpar);
    if (result < 0) {
        reportError(QStringLiteral("Could not copy video parameters: %1").arg(ffmpegErrorString(result)));
        return false;
    }

    m_videoCodec->pkt_timebase = stream->time_base;
    m_videoCodec->thread_count = 0;
    m_videoCodec->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
    m_videoCodec->flags2 |= AV_CODEC_FLAG2_FAST;
    result = avcodec_open2(m_videoCodec, decoder, nullptr);
    if (result < 0) {
        reportError(QStringLiteral("Could not open video decoder: %1").arg(ffmpegErrorString(result)));
        return false;
    }

    m_videoStreamIndex = streamIndex;
    m_videoFrame = av_frame_alloc();
    m_videoPacket = av_packet_alloc();
    if (!m_videoFrame || !m_videoPacket) {
        reportError(QStringLiteral("Could not allocate video decode buffers"));
        return false;
    }

    m_frameRate = rationalToDouble(stream->avg_frame_rate);
    if (m_frameRate <= 0.0) {
        m_frameRate = rationalToDouble(stream->r_frame_rate);
    }
    if (m_frameRate <= 0.0) {
        m_frameRate = 25.0;
    }
    m_timer.setInterval(std::clamp(static_cast<int>(std::lround(500.0 / std::max(1.0, m_frameRate))), 8, 24));

    m_hasVideo = true;
    return true;
}

bool PlaybackEngine::openAudioDecoder(const QString& path)
{
    QString error;
    if (!openInput(path, &m_audioFormat, &error)) {
        return false;
    }

    const int streamIndex = av_find_best_stream(m_audioFormat, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        return false;
    }

    AVStream* stream = m_audioFormat->streams[streamIndex];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        reportError(QStringLiteral("Could not find FFmpeg audio decoder"));
        return false;
    }

    m_audioCodec = avcodec_alloc_context3(decoder);
    if (!m_audioCodec) {
        reportError(QStringLiteral("Could not allocate audio decoder"));
        return false;
    }

    int result = avcodec_parameters_to_context(m_audioCodec, stream->codecpar);
    if (result < 0) {
        reportError(QStringLiteral("Could not copy audio parameters: %1").arg(ffmpegErrorString(result)));
        return false;
    }

    m_audioCodec->pkt_timebase = stream->time_base;
    result = avcodec_open2(m_audioCodec, decoder, nullptr);
    if (result < 0) {
        reportError(QStringLiteral("Could not open audio decoder: %1").arg(ffmpegErrorString(result)));
        return false;
    }

    if (!m_audioEngine.start(m_audioCodec->sample_rate, 2)) {
        return false;
    }
    m_audioEngine.setVolume(1.0f);

    AVChannelLayout outputLayout;
    if (m_audioEngine.channelCount() == 1) {
        av_channel_layout_default(&outputLayout, 1);
    } else {
        av_channel_layout_default(&outputLayout, 2);
    }

    const AVSampleFormat outputFormat = AV_SAMPLE_FMT_FLT;
    result = swr_alloc_set_opts2(&m_swr,
                                 &outputLayout,
                                 outputFormat,
                                 m_audioEngine.sampleRate(),
                                 &m_audioCodec->ch_layout,
                                 m_audioCodec->sample_fmt,
                                 m_audioCodec->sample_rate,
                                 0,
                                 nullptr);
    av_channel_layout_uninit(&outputLayout);

    if (result < 0 || !m_swr) {
        reportError(QStringLiteral("Could not allocate audio resampler"));
        return false;
    }

    result = swr_init(m_swr);
    if (result < 0) {
        reportError(QStringLiteral("Could not initialize audio resampler: %1").arg(ffmpegErrorString(result)));
        return false;
    }

    m_audioStreamIndex = streamIndex;
    m_audioFrame = av_frame_alloc();
    m_audioPacket = av_packet_alloc();
    if (!m_audioFrame || !m_audioPacket) {
        reportError(QStringLiteral("Could not allocate audio decode buffers"));
        return false;
    }

    m_hasAudio = true;
    return true;
}

bool PlaybackEngine::seekVideo(double seconds, bool publishFrame)
{
    if (!m_videoFormat || !m_videoCodec || m_videoStreamIndex < 0) {
        return false;
    }

    AVStream* stream = m_videoFormat->streams[m_videoStreamIndex];
    const std::int64_t timestamp = secondsToStreamTimestamp(seconds, stream->time_base);
    int result = av_seek_frame(m_videoFormat, m_videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    if (result < 0) {
        result = av_seek_frame(m_videoFormat, -1,
                               static_cast<std::int64_t>(seconds * AV_TIME_BASE),
                               AVSEEK_FLAG_BACKWARD);
    }

    if (result < 0) {
        return false;
    }

    avcodec_flush_buffers(m_videoCodec);
    av_packet_unref(m_videoPacket);
    av_frame_unref(m_videoFrame);
    m_videoEof = false;

    QImage bestImage;
    double bestTimestamp = -1.0;
    const double tolerance = 0.5 / std::max(1.0, m_frameRate);

    for (int i = 0; i < 300; ++i) {
        QImage image;
        double frameSeconds = 0.0;
        if (!decodeNextVideoFrame(&image, &frameSeconds, seconds - tolerance)) {
            break;
        }

        if (image.isNull()) {
            bestTimestamp = frameSeconds >= 0.0 ? frameSeconds : seconds;
            continue;
        }

        bestImage = std::move(image);
        bestTimestamp = frameSeconds >= 0.0 ? frameSeconds : seconds;
        if (bestTimestamp + tolerance >= seconds) {
            break;
        }
    }

    if (publishFrame && !bestImage.isNull()) {
        setFrame(std::move(bestImage), bestTimestamp);
    }

    return !bestImage.isNull();
}

bool PlaybackEngine::seekAudio(double seconds)
{
    if (!m_audioFormat || !m_audioCodec || m_audioStreamIndex < 0) {
        return false;
    }

    AVStream* stream = m_audioFormat->streams[m_audioStreamIndex];
    const std::int64_t timestamp = secondsToStreamTimestamp(seconds, stream->time_base);
    int result = av_seek_frame(m_audioFormat, m_audioStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    if (result < 0) {
        result = av_seek_frame(m_audioFormat, -1,
                               static_cast<std::int64_t>(seconds * AV_TIME_BASE),
                               AVSEEK_FLAG_BACKWARD);
    }

    if (result < 0) {
        return false;
    }

    avcodec_flush_buffers(m_audioCodec);
    av_packet_unref(m_audioPacket);
    av_frame_unref(m_audioFrame);
    m_pendingAudio.clear();
    swr_init(m_swr);
    m_audioEof = false;
    m_audioSkipUntilSeconds = std::max(0.0, seconds);
    return true;
}

bool PlaybackEngine::decodeNextVideoFrame(QImage* image, double* timestampSeconds, double minConvertedTimestampSeconds)
{
    if (!m_videoFormat || !m_videoCodec || !m_videoFrame || !m_videoPacket || m_videoEof) {
        return false;
    }

    while (true) {
        int result = avcodec_receive_frame(m_videoCodec, m_videoFrame);
        if (result == 0) {
            const int width = m_videoFrame->width > 0 ? m_videoFrame->width : m_videoCodec->width;
            const int height = m_videoFrame->height > 0 ? m_videoFrame->height : m_videoCodec->height;
            const auto sourceFormat = static_cast<AVPixelFormat>(m_videoFrame->format);

            if (width <= 0 || height <= 0 || sourceFormat == AV_PIX_FMT_NONE) {
                av_frame_unref(m_videoFrame);
                continue;
            }

            const double frameSeconds = frameTimestampSeconds(m_videoFrame, m_videoFormat->streams[m_videoStreamIndex]);
            if (timestampSeconds) {
                *timestampSeconds = frameSeconds;
            }

            if (!isVideoImage()
                && minConvertedTimestampSeconds >= 0.0
                && frameSeconds >= 0.0
                && frameSeconds < minConvertedTimestampSeconds) {
                if (image) {
                    *image = {};
                }
                av_frame_unref(m_videoFrame);
                return true;
            }

            const QSize outputSize = playbackFrameSize(width, height);
            const int outputWidth = std::max(1, outputSize.width());
            const int outputHeight = std::max(1, outputSize.height());

            m_sws = sws_getCachedContext(m_sws,
                                         width,
                                         height,
                                         sourceFormat,
                                         outputWidth,
                                         outputHeight,
                                         AV_PIX_FMT_BGRA,
                                         SWS_BICUBIC,
                                         nullptr,
                                         nullptr,
                                         nullptr);
            if (!m_sws) {
                av_frame_unref(m_videoFrame);
                return false;
            }

            QImage converted(outputWidth, outputHeight, QImage::Format_ARGB32_Premultiplied);
            std::uint8_t* destinationData[4] = {converted.bits(), nullptr, nullptr, nullptr};
            int destinationLineSize[4] = {static_cast<int>(converted.bytesPerLine()), 0, 0, 0};

            sws_scale(m_sws,
                      m_videoFrame->data,
                      m_videoFrame->linesize,
                      0,
                      height,
                      destinationData,
                      destinationLineSize);

            if (image) {
                *image = converted;
            }

            av_frame_unref(m_videoFrame);
            return true;
        }

        if (result == AVERROR_EOF) {
            m_videoEof = true;
            return false;
        }

        if (result != AVERROR(EAGAIN)) {
            reportError(QStringLiteral("Video decode failed: %1").arg(ffmpegErrorString(result)));
            return false;
        }

        while (true) {
            result = av_read_frame(m_videoFormat, m_videoPacket);
            if (result < 0) {
                avcodec_send_packet(m_videoCodec, nullptr);
                break;
            }

            if (m_videoPacket->stream_index == m_videoStreamIndex) {
                result = avcodec_send_packet(m_videoCodec, m_videoPacket);
                av_packet_unref(m_videoPacket);
                if (result < 0 && result != AVERROR(EAGAIN)) {
                    reportError(QStringLiteral("Could not send video packet: %1").arg(ffmpegErrorString(result)));
                    return false;
                }
                break;
            }

            av_packet_unref(m_videoPacket);
        }
    }
}

bool PlaybackEngine::decodeNextAudioChunk(QByteArray* pcm)
{
    if (!m_audioFormat || !m_audioCodec || !m_audioFrame || !m_audioPacket || !m_swr || m_audioEof) {
        return false;
    }

    while (true) {
        int result = avcodec_receive_frame(m_audioCodec, m_audioFrame);
        if (result == 0) {
            const double frameSeconds = frameTimestampSeconds(m_audioFrame, m_audioFormat->streams[m_audioStreamIndex]);
            if (frameSeconds >= 0.0 && frameSeconds + 0.05 < m_audioSkipUntilSeconds) {
                av_frame_unref(m_audioFrame);
                continue;
            }
            m_audioSkipUntilSeconds = 0.0;

            const AVSampleFormat outputFormat = AV_SAMPLE_FMT_FLT;
            const int outputChannels = std::max(1, m_audioEngine.channelCount());
            const int outputSampleRate = std::max(1, m_audioEngine.sampleRate());
            const int outputSamples = static_cast<int>(
                av_rescale_rnd(swr_get_delay(m_swr, m_audioCodec->sample_rate) + m_audioFrame->nb_samples,
                               outputSampleRate,
                               m_audioCodec->sample_rate,
                               AV_ROUND_UP));

            QByteArray buffer(outputSamples * outputChannels * bytesPerSample(outputFormat), Qt::Uninitialized);
            auto* outputData = reinterpret_cast<std::uint8_t*>(buffer.data());
            const int convertedSamples = swr_convert(m_swr,
                                                     &outputData,
                                                     outputSamples,
                                                     const_cast<const std::uint8_t**>(m_audioFrame->extended_data),
                                                     m_audioFrame->nb_samples);
            av_frame_unref(m_audioFrame);

            if (convertedSamples < 0) {
                reportError(QStringLiteral("Audio resample failed"));
                return false;
            }

            buffer.resize(convertedSamples * outputChannels * bytesPerSample(outputFormat));
            EffectProcessor::processAudio(buffer, m_currentEffects);
            applyPlaybackHeadroom(&buffer);
            if (pcm) {
                *pcm = m_audioEngine.sampleFormat() == QAudioFormat::Float
                    ? buffer
                    : floatPcmToInt16(buffer);
            }
            return !buffer.isEmpty();
        }

        if (result == AVERROR_EOF) {
            m_audioEof = true;
            return false;
        }

        if (result != AVERROR(EAGAIN)) {
            reportError(QStringLiteral("Audio decode failed: %1").arg(ffmpegErrorString(result)));
            return false;
        }

        while (true) {
            result = av_read_frame(m_audioFormat, m_audioPacket);
            if (result < 0) {
                avcodec_send_packet(m_audioCodec, nullptr);
                break;
            }

            if (m_audioPacket->stream_index == m_audioStreamIndex) {
                result = avcodec_send_packet(m_audioCodec, m_audioPacket);
                av_packet_unref(m_audioPacket);
                if (result < 0 && result != AVERROR(EAGAIN)) {
                    reportError(QStringLiteral("Could not send audio packet: %1").arg(ffmpegErrorString(result)));
                    return false;
                }
                break;
            }

            av_packet_unref(m_audioPacket);
        }
    }
}

QSize PlaybackEngine::playbackFrameSize(int sourceWidth, int sourceHeight) const
{
    if (sourceWidth <= 0 || sourceHeight <= 0) {
        return {};
    }

    return QSize(sourceWidth, sourceHeight);
}

void PlaybackEngine::feedAudio()
{
    if (!m_hasAudio || !m_audioEngine.isActive()) {
        return;
    }

    const qint64 minimumChunk = std::max<qint64>(4096, m_audioEngine.bytesPerFrame() * 512);
    int guard = 0;
    while (guard < 8) {
        if (!writePendingAudio()) {
            return;
        }

        if (m_audioEngine.bytesFree() < minimumChunk) {
            return;
        }

        QByteArray pcm;
        if (!decodeNextAudioChunk(&pcm)) {
            break;
        }

        if (m_isMuted) {
            pcm.fill(0);
        }

        m_pendingAudio.append(pcm);
        ++guard;
    }

    writePendingAudio();
}

bool PlaybackEngine::writePendingAudio()
{
    const qint64 bytesPerFrame = std::max<qint64>(1, m_audioEngine.bytesPerFrame());

    while (!m_pendingAudio.isEmpty()) {
        qint64 bytesToWrite = std::min<qint64>(m_audioEngine.bytesFree(), m_pendingAudio.size());
        bytesToWrite -= bytesToWrite % bytesPerFrame;
        if (bytesToWrite <= 0) {
            return false;
        }

        const qint64 written = m_audioEngine.write(m_pendingAudio.constData(), bytesToWrite);
        if (written <= 0) {
            return false;
        }

        const qsizetype writtenBytes = static_cast<qsizetype>(
            std::min<qint64>(written, m_pendingAudio.size()));
        if (writtenBytes <= 0) {
            return false;
        }

        const qsizetype levelBytes = writtenBytes - (writtenBytes % bytesPerFrame);
        if (levelBytes > 0) {
            updateAudioLevels(QByteArray(m_pendingAudio.constData(), levelBytes));
        }
        m_pendingAudio.remove(0, writtenBytes);
    }

    return true;
}

void PlaybackEngine::setPosition(double seconds, bool forceNotify)
{
    // When playing across a multi-clip timeline, allow position to advance
    // beyond the currently-loaded clip's duration so scanClipsAtPosition can
    // detect and swap to the next clip.
    const double clampMax = m_sequenceDuration > 0.0
        ? std::max(m_sequenceDuration, m_duration)
        : m_duration;

    const double nextPosition = clampMax > 0.0
        ? std::clamp(seconds, 0.0, clampMax)
        : std::max(0.0, seconds);

    if (std::abs(m_position - nextPosition) < 0.001) {
        if (forceNotify) {
            emit positionChanged();
        }
        return;
    }

    m_position = nextPosition;

    const qint64 now = m_positionNotifyClock.isValid() ? m_positionNotifyClock.elapsed() : 0;
    if (forceNotify || m_lastPositionNotifyMs < 0 || now - m_lastPositionNotifyMs >= 33) {
        m_lastPositionNotifyMs = now;
        emit positionChanged();
    }
}


void PlaybackEngine::setFrame(QImage image, double timestampSeconds)
{
    if (image.isNull()) {
        return;
    }

    m_frame = std::move(image);
    m_processedFrame = EffectProcessor::processImage(m_frame, m_currentEffects);
    
    if (timestampSeconds >= 0.0) {
        m_lastVideoFrameSeconds = timestampSeconds;
    }
    update();
}

void PlaybackEngine::setClipEffects(const ClipEffects& effects)
{
    m_currentEffects = effects;
    if (!m_frame.isNull()) {
        m_processedFrame = EffectProcessor::processImage(m_frame, m_currentEffects);
        update();
    }
}

void PlaybackEngine::setPlaying(bool playing)
{
    if (m_playing == playing) {
        return;
    }

    m_playing = playing;
    emit playingChanged();
}

void PlaybackEngine::setAudioLevels(double left, double right)
{
    const double nextLeft = std::clamp(left, 0.0, 1.0);
    const double nextRight = std::clamp(right, 0.0, 1.0);

    if (std::abs(m_audioLevelLeft - nextLeft) < 0.005
        && std::abs(m_audioLevelRight - nextRight) < 0.005) {
        return;
    }

    m_audioLevelLeft = nextLeft;
    m_audioLevelRight = nextRight;
    emit audioLevelsChanged();
}

void PlaybackEngine::updateAudioLevels(const QByteArray& pcm)
{
    if (pcm.isEmpty() || m_audioEngine.channelCount() <= 0) {
        setAudioLevels(0.0, 0.0);
        return;
    }

    const int channels = std::max(1, m_audioEngine.channelCount());
    const int bytesPerFrame = m_audioEngine.bytesPerFrame();
    if (bytesPerFrame <= 0) {
        setAudioLevels(0.0, 0.0);
        return;
    }

    double leftPeak = 0.0;
    double rightPeak = 0.0;
    const int frames = pcm.size() / bytesPerFrame;

    if (m_audioEngine.sampleFormat() == QAudioFormat::Float) {
        const auto* samples = reinterpret_cast<const float*>(pcm.constData());
        for (int frame = 0; frame < frames; ++frame) {
            const double left = std::abs(static_cast<double>(samples[frame * channels]));
            const double right = channels > 1
                ? std::abs(static_cast<double>(samples[frame * channels + 1]))
                : left;
            leftPeak = std::max(leftPeak, left);
            rightPeak = std::max(rightPeak, right);
        }
    } else {
        const auto* samples = reinterpret_cast<const std::int16_t*>(pcm.constData());
        constexpr double scale = 1.0 / 32768.0;
        for (int frame = 0; frame < frames; ++frame) {
            const double left = std::abs(static_cast<double>(samples[frame * channels]) * scale);
            const double right = channels > 1
                ? std::abs(static_cast<double>(samples[frame * channels + 1]) * scale)
                : left;
            leftPeak = std::max(leftPeak, left);
            rightPeak = std::max(rightPeak, right);
        }
    }

    setAudioLevels(leftPeak, rightPeak);
}

void PlaybackEngine::reportError(const QString& message)
{
    qWarning().noquote() << "PlaybackEngine:" << message;
    emit playbackError(message);
}
