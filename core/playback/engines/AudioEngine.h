#pragma once

#include <QAudioFormat>
#include <QAudioSink>
#include <QByteArray>
#include <QIODevice>
#include <QObject>

#include <memory>

class AudioEngine : public QObject {
    Q_OBJECT

public:
    explicit AudioEngine(QObject* parent = nullptr);
    ~AudioEngine() override;

    bool start(int sampleRate, int channelCount = 2);
    void stop();
    void reset();
    void pause();
    void resume();

    bool isActive() const noexcept;
    qint64 bytesFree() const;
    qint64 write(const QByteArray& pcm);
    qint64 write(const char* data, qint64 maxSize);

    int sampleRate() const noexcept;
    int channelCount() const noexcept;
    QAudioFormat::SampleFormat sampleFormat() const noexcept;
    int bytesPerFrame() const noexcept;
    void setVolume(float volume);

private:
    QAudioFormat m_format;
    std::unique_ptr<QAudioSink> m_sink;
    QIODevice* m_device = nullptr;
};
