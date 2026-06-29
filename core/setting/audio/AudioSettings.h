#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class AudioSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString defaultAudioDevice READ defaultAudioDevice WRITE setDefaultAudioDevice NOTIFY defaultAudioDeviceChanged)
    Q_PROPERTY(QString sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(QString bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged)
    Q_PROPERTY(bool muteWhileScrubbing READ muteWhileScrubbing WRITE setMuteWhileScrubbing NOTIFY muteWhileScrubbingChanged)
    Q_PROPERTY(bool audioPeakHold READ audioPeakHold WRITE setAudioPeakHold NOTIFY audioPeakHoldChanged)

public:
    explicit AudioSettings(QObject* parent = nullptr);

    QString defaultAudioDevice() const;
    void setDefaultAudioDevice(const QString& device);

    QString sampleRate() const;
    void setSampleRate(const QString& rate);

    QString bufferSize() const;
    void setBufferSize(const QString& size);

    bool muteWhileScrubbing() const;
    void setMuteWhileScrubbing(bool mute);

    bool audioPeakHold() const;
    void setAudioPeakHold(bool peakHold);

signals:
    void defaultAudioDeviceChanged();
    void sampleRateChanged();
    void bufferSizeChanged();
    void muteWhileScrubbingChanged();
    void audioPeakHoldChanged();

private:
    QString m_defaultAudioDevice;
    QString m_sampleRate;
    QString m_bufferSize;
    bool m_muteWhileScrubbing;
    bool m_audioPeakHold;
};
