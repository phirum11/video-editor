#include "core/media/models/MediaItem.h"

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
}

namespace {

struct AVFormatContextDeleter {
    void operator()(AVFormatContext* context) const noexcept
    {
        if (context) {
            avformat_close_input(&context);
        }
    }
};

struct AVCodecContextDeleter {
    void operator()(AVCodecContext* context) const noexcept
    {
        if (context) {
            avcodec_free_context(&context);
        }
    }
};

struct AVPacketDeleter {
    void operator()(AVPacket* packet) const noexcept
    {
        if (packet) {
            av_packet_free(&packet);
        }
    }
};

struct AVFrameDeleter {
    void operator()(AVFrame* frame) const noexcept
    {
        if (frame) {
            av_frame_free(&frame);
        }
    }
};

struct SwsContextDeleter {
    void operator()(SwsContext* context) const noexcept
    {
        sws_freeContext(context);
    }
};

using AVFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;
using AVCodecContextPtr = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
using AVPacketPtr = std::unique_ptr<AVPacket, AVPacketDeleter>;
using AVFramePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;
using SwsContextPtr = std::unique_ptr<SwsContext, SwsContextDeleter>;

QString ffmpegErrorString(int errorCode)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(errorCode, buffer, sizeof(buffer));
    return QString::fromUtf8(buffer);
}

std::runtime_error ffmpegError(const QString& action, int errorCode)
{
    const QString message = QStringLiteral("%1: %2").arg(action, ffmpegErrorString(errorCode));
    return std::runtime_error(message.toStdString());
}

QByteArray ffmpegPath(const QString& filePath)
{
    return QDir::toNativeSeparators(filePath).toUtf8();
}

AVFormatContextPtr openFormatContext(const QString& filePath)
{
    AVFormatContext* rawContext = nullptr;
    const QByteArray path = ffmpegPath(filePath);

    int result = avformat_open_input(&rawContext, path.constData(), nullptr, nullptr);
    if (result < 0) {
        throw ffmpegError(QStringLiteral("Could not open media file '%1'").arg(filePath), result);
    }

    if (filePath.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) ||
        filePath.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive) ||
        filePath.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive) ||
        filePath.endsWith(QStringLiteral(".webp"), Qt::CaseInsensitive) ||
        filePath.endsWith(QStringLiteral(".bmp"), Qt::CaseInsensitive)) {
        rawContext->max_analyze_duration = AV_TIME_BASE / 10;
        rawContext->probesize = 32768;
    }

    AVFormatContextPtr context(rawContext);
    result = avformat_find_stream_info(context.get(), nullptr);
    if (result < 0) {
        throw ffmpegError(QStringLiteral("Could not read media stream info"), result);
    }

    return context;
}

bool isPositiveRational(AVRational rational)
{
    return rational.num > 0 && rational.den > 0;
}

double rationalToDouble(AVRational rational)
{
    if (!isPositiveRational(rational)) {
        return 0.0;
    }

    return av_q2d(rational);
}

qint64 streamDurationUs(const AVStream* stream)
{
    if (!stream || stream->duration == AV_NOPTS_VALUE) {
        return 0;
    }

    const AVRational timeBase = stream->time_base;
    const AVRational avTimeBase = {1, AV_TIME_BASE};
    return static_cast<qint64>(av_rescale_q(stream->duration, timeBase, avTimeBase));
}

qint64 contextDurationUs(const AVFormatContext* context)
{
    if (!context) {
        return 0;
    }

    qint64 duration = context->duration != AV_NOPTS_VALUE
        ? static_cast<qint64>(context->duration)
        : 0;

    for (unsigned int i = 0; i < context->nb_streams; ++i) {
        duration = std::max(duration, streamDurationUs(context->streams[i]));
    }

    return duration;
}

std::int64_t secondsToAvTimeBase(double seconds)
{
    if (!std::isfinite(seconds) || seconds <= 0.0) {
        return 0;
    }

    const long double scaled = static_cast<long double>(seconds) * AV_TIME_BASE;
    const long double maxValue = static_cast<long double>(std::numeric_limits<std::int64_t>::max());
    if (scaled >= maxValue) {
        return std::numeric_limits<std::int64_t>::max();
    }

    return static_cast<std::int64_t>(std::llround(scaled));
}

double normalizedTimestamp(double timestampSeconds, double durationSeconds)
{
    if (!std::isfinite(timestampSeconds) || timestampSeconds <= 0.0) {
        return 0.0;
    }

    if (durationSeconds > 0.0) {
        return std::min(timestampSeconds, durationSeconds);
    }

    return timestampSeconds;
}

QSize scaledFrameSize(int sourceWidth, int sourceHeight, QSize requestedSize)
{
    if (sourceWidth <= 0 || sourceHeight <= 0) {
        return {};
    }

    const QSize sourceSize(sourceWidth, sourceHeight);
    if (!requestedSize.isValid() || requestedSize.isEmpty()) {
        return sourceSize;
    }

    requestedSize.setWidth(std::max(1, requestedSize.width()));
    requestedSize.setHeight(std::max(1, requestedSize.height()));

    QSize outputSize = sourceSize;
    outputSize.scale(requestedSize, Qt::KeepAspectRatioByExpanding);
    outputSize = outputSize.boundedTo(sourceSize);
    return outputSize.isEmpty() ? sourceSize : outputSize;
}

struct VideoDecoder {
    int streamIndex = -1;
    AVStream* stream = nullptr;
    AVCodecContextPtr codecContext;
};

VideoDecoder openVideoDecoder(AVFormatContext* formatContext)
{
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        throw ffmpegError(QStringLiteral("Could not find a video stream"), streamIndex);
    }

    AVStream* stream = formatContext->streams[streamIndex];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        throw std::runtime_error("Could not find a decoder for the video stream");
    }

    AVCodecContextPtr codecContext(avcodec_alloc_context3(decoder));
    if (!codecContext) {
        throw std::runtime_error("Could not allocate a video codec context");
    }

    int result = avcodec_parameters_to_context(codecContext.get(), stream->codecpar);
    if (result < 0) {
        throw ffmpegError(QStringLiteral("Could not copy video codec parameters"), result);
    }

    codecContext->pkt_timebase = stream->time_base;

    result = avcodec_open2(codecContext.get(), decoder, nullptr);
    if (result < 0) {
        throw ffmpegError(QStringLiteral("Could not open video decoder"), result);
    }

    return VideoDecoder{streamIndex, stream, std::move(codecContext)};
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

QImage convertFrameToImage(const AVFrame* frame, const AVCodecContext* codecContext, QSize requestedSize)
{
    const int width = frame->width > 0 ? frame->width : codecContext->width;
    const int height = frame->height > 0 ? frame->height : codecContext->height;
    const auto sourceFormat = static_cast<AVPixelFormat>(frame->format);

    if (width <= 0 || height <= 0 || sourceFormat == AV_PIX_FMT_NONE) {
        return {};
    }

    const QSize outputSize = scaledFrameSize(width, height, requestedSize);
    const int outputWidth = std::max(1, outputSize.width());
    const int outputHeight = std::max(1, outputSize.height());

    SwsContextPtr scaler(sws_getContext(width,
                                        height,
                                        sourceFormat,
                                        outputWidth,
                                        outputHeight,
                                        AV_PIX_FMT_RGBA,
                                        SWS_BICUBIC,
                                        nullptr,
                                        nullptr,
                                        nullptr));
    if (!scaler) {
        throw std::runtime_error("Could not create FFmpeg image scaler");
    }

    QImage image(outputWidth, outputHeight, QImage::Format_RGBA8888);
    if (image.isNull()) {
        throw std::runtime_error("Could not allocate thumbnail image");
    }

    std::uint8_t* destinationData[4] = {image.bits(), nullptr, nullptr, nullptr};
    int destinationLineSize[4] = {static_cast<int>(image.bytesPerLine()), 0, 0, 0};

    const int scaledHeight = sws_scale(scaler.get(),
                                       frame->data,
                                       frame->linesize,
                                       0,
                                       height,
                                       destinationData,
                                       destinationLineSize);
    if (scaledHeight <= 0) {
        return {};
    }

    return image;
}

} // namespace

MediaItem::MediaItem(QString filePath)
    : m_filePath(std::move(filePath))
{
    loadMetadata();
}

MediaItem::MediaItem(const std::string& filePath)
    : MediaItem(QString::fromUtf8(filePath.data(), static_cast<qsizetype>(filePath.size())))
{
}

const QString& MediaItem::filePath() const noexcept
{
    return m_filePath;
}

QString MediaItem::fileName() const
{
    return QFileInfo(m_filePath).fileName();
}

qint64 MediaItem::durationMicroseconds() const noexcept
{
    return m_durationUs;
}

double MediaItem::durationSeconds() const noexcept
{
    return static_cast<double>(m_durationUs) / AV_TIME_BASE;
}

QSize MediaItem::resolution() const noexcept
{
    return m_resolution;
}

int MediaItem::width() const noexcept
{
    return m_resolution.width();
}

int MediaItem::height() const noexcept
{
    return m_resolution.height();
}

double MediaItem::frameRate() const noexcept
{
    return m_frameRate;
}

bool MediaItem::hasAudio() const noexcept
{
    return m_hasAudio;
}

bool MediaItem::hasVideo() const noexcept
{
    return m_hasVideo;
}

int MediaItem::videoStreamIndex() const noexcept
{
    return m_videoStreamIndex;
}

int MediaItem::audioStreamIndex() const noexcept
{
    return m_audioStreamIndex;
}

void MediaItem::loadMetadata()
{
    if (m_filePath.endsWith(QStringLiteral(".srt"), Qt::CaseInsensitive)) {
        m_durationUs = 0;
        m_resolution = {};
        m_frameRate = 0.0;
        m_hasAudio = false;
        m_hasVideo = false;
        m_videoStreamIndex = -1;
        m_audioStreamIndex = -1;
        return;
    }

    AVFormatContextPtr context = openFormatContext(m_filePath);

    m_durationUs = contextDurationUs(context.get());
    m_resolution = {};
    m_frameRate = 0.0;
    m_hasAudio = false;
    m_hasVideo = false;
    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;

    for (unsigned int i = 0; i < context->nb_streams; ++i) {
        const AVStream* stream = context->streams[i];
        const AVCodecParameters* parameters = stream->codecpar;

        if (parameters->codec_type == AVMEDIA_TYPE_VIDEO && !m_hasVideo) {
            m_hasVideo = true;
            m_videoStreamIndex = static_cast<int>(i);
            m_resolution = QSize(parameters->width, parameters->height);
            m_frameRate = rationalToDouble(stream->avg_frame_rate);
            if (m_frameRate <= 0.0) {
                m_frameRate = rationalToDouble(stream->r_frame_rate);
            }
        } else if (parameters->codec_type == AVMEDIA_TYPE_AUDIO && !m_hasAudio) {
            m_hasAudio = true;
            m_audioStreamIndex = static_cast<int>(i);
        }
    }
}

QImage MediaItem::extractFrame(double timestampSeconds, QSize requestedSize) const
{
    if (!m_hasVideo) {
        return {};
    }

    AVFormatContextPtr context = openFormatContext(m_filePath);
    VideoDecoder decoder = openVideoDecoder(context.get());

    const bool isImage = m_filePath.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) ||
                         m_filePath.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive) ||
                         m_filePath.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive) ||
                         m_filePath.endsWith(QStringLiteral(".webp"), Qt::CaseInsensitive) ||
                         m_filePath.endsWith(QStringLiteral(".bmp"), Qt::CaseInsensitive);

    const double targetSeconds = isImage ? 0.0 : normalizedTimestamp(timestampSeconds, durationSeconds());
    const std::int64_t targetAvTimestamp = secondsToAvTimeBase(targetSeconds);
    const AVRational avTimeBase = {1, AV_TIME_BASE};
    const std::int64_t streamSeekTimestamp =
        av_rescale_q(targetAvTimestamp, avTimeBase, decoder.stream->time_base);

    int result = 0;
    if (targetSeconds > 0.0) {
        result = av_seek_frame(context.get(),
                               decoder.streamIndex,
                               streamSeekTimestamp,
                               AVSEEK_FLAG_BACKWARD);
        if (result < 0) {
            result = av_seek_frame(context.get(), -1, targetAvTimestamp, AVSEEK_FLAG_BACKWARD);
        }
    }

    if (result >= 0) {
        avcodec_flush_buffers(decoder.codecContext.get());
    }

    AVPacketPtr packet(av_packet_alloc());
    AVFramePtr frame(av_frame_alloc());
    if (!packet || !frame) {
        throw std::runtime_error("Could not allocate FFmpeg decode buffers");
    }

    const double frameToleranceSeconds = m_frameRate > 0.0 ? 0.5 / m_frameRate : 0.001;
    auto receiveFrames = [&]() -> QImage {
        while (true) {
            const int receiveResult = avcodec_receive_frame(decoder.codecContext.get(), frame.get());
            if (receiveResult == AVERROR(EAGAIN) || receiveResult == AVERROR_EOF) {
                return {};
            }
            if (receiveResult < 0) {
                throw ffmpegError(QStringLiteral("Could not decode video frame"), receiveResult);
            }

            const double frameSeconds = frameTimestampSeconds(frame.get(), decoder.stream);
            QImage image;
            if (frameSeconds < 0.0 || frameSeconds + frameToleranceSeconds >= targetSeconds) {
                image = convertFrameToImage(frame.get(), decoder.codecContext.get(), requestedSize);
                if (image.isNull()) {
                    qDebug() << "convertFrameToImage returned null image!";
                }
                av_frame_unref(frame.get());
                return image;
            }

            av_frame_unref(frame.get());
        }
    };

    while ((result = av_read_frame(context.get(), packet.get())) >= 0) {
        if (packet->stream_index != decoder.streamIndex) {
            av_packet_unref(packet.get());
            continue;
        }
        // qDebug() << "Read packet at pts" << packet->pts;

        result = avcodec_send_packet(decoder.codecContext.get(), packet.get());
        av_packet_unref(packet.get());

        if (result < 0) {
            throw ffmpegError(QStringLiteral("Could not send video packet to decoder"), result);
        }

        QImage image = receiveFrames();
        if (!image.isNull()) {
            return image;
        }
    }

    if (result != AVERROR_EOF) {
        throw ffmpegError(QStringLiteral("Could not read video packet"), result);
    }

    result = avcodec_send_packet(decoder.codecContext.get(), nullptr);
    if (result >= 0) {
        QImage image = receiveFrames();
        if (!image.isNull()) {
            return image;
        } else {
            qDebug() << "receiveFrames returned null after flush!";
        }
    } else if (result != AVERROR_EOF) {
        throw ffmpegError(QStringLiteral("Could not flush video decoder"), result);
    }

    return {};
}
