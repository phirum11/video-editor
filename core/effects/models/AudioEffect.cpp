#include "AudioEffect.h"

AudioEffect::AudioEffect(QObject *parent) : QObject(parent)
{
}

double AudioEffect::volume() const { return m_data.volume; }
void AudioEffect::setVolume(double val) { 
    if (m_data.volume != val) { 
        m_data.volume = val; 
        emit volumeChanged(); 
        emit modified(); 
    } 
}

double AudioEffect::pan() const { return m_data.pan; }
void AudioEffect::setPan(double val) { 
    if (m_data.pan != val) { 
        m_data.pan = val; 
        emit panChanged(); 
        emit modified(); 
    } 
}

void AudioEffect::loadData(const AudioEffectData& data) {
    if (m_data.volume != data.volume) { m_data.volume = data.volume; emit volumeChanged(); }
    if (m_data.pan != data.pan) { m_data.pan = data.pan; emit panChanged(); }
}

AudioEffectData AudioEffect::toData() const {
    return m_data;
}
