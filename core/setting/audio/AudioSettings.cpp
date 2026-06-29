#include "core/setting/audio/AudioSettings.h"

AudioSettings::AudioSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("Audio");
    m_defaultAudioDevice = settings.value("defaultAudioDevice", "System Default").toString();
    m_sampleRate = settings.value("sampleRate", "48000 Hz").toString();
    m_bufferSize = settings.value("bufferSize", "1024 samples").toString();
    m_muteWhileScrubbing = settings.value("muteWhileScrubbing", false).toBool();
    m_audioPeakHold = settings.value("audioPeakHold", true).toBool();
    settings.endGroup();
}

QString AudioSettings::defaultAudioDevice() const { return m_defaultAudioDevice; }
void AudioSettings::setDefaultAudioDevice(const QString& device) {
    if (m_defaultAudioDevice != device) {
        m_defaultAudioDevice = device;
        QSettings settings;
        settings.beginGroup("Audio");
        settings.setValue("defaultAudioDevice", device);
        settings.endGroup();
        emit defaultAudioDeviceChanged();
    }
}

QString AudioSettings::sampleRate() const { return m_sampleRate; }
void AudioSettings::setSampleRate(const QString& rate) {
    if (m_sampleRate != rate) {
        m_sampleRate = rate;
        QSettings settings;
        settings.beginGroup("Audio");
        settings.setValue("sampleRate", rate);
        settings.endGroup();
        emit sampleRateChanged();
    }
}

QString AudioSettings::bufferSize() const { return m_bufferSize; }
void AudioSettings::setBufferSize(const QString& size) {
    if (m_bufferSize != size) {
        m_bufferSize = size;
        QSettings settings;
        settings.beginGroup("Audio");
        settings.setValue("bufferSize", size);
        settings.endGroup();
        emit bufferSizeChanged();
    }
}

bool AudioSettings::muteWhileScrubbing() const { return m_muteWhileScrubbing; }
void AudioSettings::setMuteWhileScrubbing(bool mute) {
    if (m_muteWhileScrubbing != mute) {
        m_muteWhileScrubbing = mute;
        QSettings settings;
        settings.beginGroup("Audio");
        settings.setValue("muteWhileScrubbing", mute);
        settings.endGroup();
        emit muteWhileScrubbingChanged();
    }
}

bool AudioSettings::audioPeakHold() const { return m_audioPeakHold; }
void AudioSettings::setAudioPeakHold(bool peakHold) {
    if (m_audioPeakHold != peakHold) {
        m_audioPeakHold = peakHold;
        QSettings settings;
        settings.beginGroup("Audio");
        settings.setValue("audioPeakHold", peakHold);
        settings.endGroup();
        emit audioPeakHoldChanged();
    }
}
