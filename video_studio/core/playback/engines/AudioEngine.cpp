#include "core/playback/engines/AudioEngine.h"

#include <QMediaDevices>
#include <QDebug>

#include <algorithm>

namespace {

bool isSupportedPlaybackFormat(const QAudioDevice& device, const QAudioFormat& format)
{
    return (format.sampleFormat() == QAudioFormat::Float
            || format.sampleFormat() == QAudioFormat::Int16)
        && device.isFormatSupported(format);
}

QAudioFormat makePlaybackFormat(int sampleRate,
                                int channelCount,
                                QAudioFormat::SampleFormat sampleFormat)
{
    QAudioFormat format;
    format.setSampleRate(sampleRate > 0 ? sampleRate : 48000);
    format.setChannelCount(std::clamp(channelCount, 1, 2));
    format.setSampleFormat(sampleFormat);
    return format;
}

} // namespace

AudioEngine::AudioEngine(QObject* parent)
    : QObject(parent)
{
}

AudioEngine::~AudioEngine()
{
    stop();
}

bool AudioEngine::start(int sampleRate, int channelCount)
{
    stop();

    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        qWarning() << "AudioEngine: no default audio output device";
        return false;
    }

    QAudioFormat requestedFormat = makePlaybackFormat(sampleRate, channelCount, QAudioFormat::Float);

    if (!isSupportedPlaybackFormat(device, requestedFormat)) {
        requestedFormat = makePlaybackFormat(sampleRate, channelCount, QAudioFormat::Int16);
    }

    if (!isSupportedPlaybackFormat(device, requestedFormat)) {
        QAudioFormat fallbackFormat = makePlaybackFormat(48000, 2, QAudioFormat::Float);

        if (isSupportedPlaybackFormat(device, fallbackFormat)) {
            requestedFormat = fallbackFormat;
        } else {
            fallbackFormat = makePlaybackFormat(48000, 2, QAudioFormat::Int16);
            if (isSupportedPlaybackFormat(device, fallbackFormat)) {
                requestedFormat = fallbackFormat;
            } else {
                requestedFormat = device.preferredFormat();
            }
        }
    }

    if (requestedFormat.sampleFormat() != QAudioFormat::Int16
        && requestedFormat.sampleFormat() != QAudioFormat::Float) {
        qWarning() << "AudioEngine: unsupported output sample format" << requestedFormat.sampleFormat();
        return false;
    }

    m_format = requestedFormat;
    m_sink = std::make_unique<QAudioSink>(device, m_format, this);
    m_sink->setBufferSize(std::max(4096, bytesPerFrame() * m_format.sampleRate() / 10));
    m_device = m_sink->start();

    if (!m_device) {
        qWarning() << "AudioEngine: failed to start audio sink";
        m_sink.reset();
        return false;
    }

    return true;
}

void AudioEngine::stop()
{
    if (m_sink) {
        m_sink->stop();
    }

    m_device = nullptr;
    m_sink.reset();
}

void AudioEngine::pause()
{
    if (m_sink) {
        m_sink->suspend();
    }
}

void AudioEngine::resume()
{
    if (m_sink) {
        m_sink->resume();
    }
}

bool AudioEngine::isActive() const noexcept
{
    return m_sink && m_device;
}

qint64 AudioEngine::bytesFree() const
{
    return m_sink ? m_sink->bytesFree() : 0;
}

qint64 AudioEngine::write(const QByteArray& pcm)
{
    return write(pcm.constData(), pcm.size());
}

qint64 AudioEngine::write(const char* data, qint64 maxSize)
{
    if (!m_device || !data || maxSize <= 0) {
        return 0;
    }

    return m_device->write(data, maxSize);
}

int AudioEngine::sampleRate() const noexcept
{
    return m_format.sampleRate();
}

int AudioEngine::channelCount() const noexcept
{
    return m_format.channelCount();
}

QAudioFormat::SampleFormat AudioEngine::sampleFormat() const noexcept
{
    return m_format.sampleFormat();
}

int AudioEngine::bytesPerFrame() const noexcept
{
    int bytesPerSample = 0;
    switch (m_format.sampleFormat()) {
    case QAudioFormat::Int16:
        bytesPerSample = 2;
        break;
    case QAudioFormat::Float:
        bytesPerSample = 4;
        break;
    default:
        bytesPerSample = 0;
        break;
    }

    return bytesPerSample * std::max(1, m_format.channelCount());
}

void AudioEngine::setVolume(float volume)
{
    if (m_sink) {
        m_sink->setVolume(std::clamp(volume, 0.0f, 1.0f));
    }
}
