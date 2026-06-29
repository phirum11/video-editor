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

double AudioEffect::pitch() const { return m_data.pitch; }
void AudioEffect::setPitch(double val) { if (m_data.pitch != val) { m_data.pitch = val; emit pitchChanged(); emit modified(); } }

double AudioEffect::eq32() const { return m_data.eq32; } void AudioEffect::setEq32(double val) { if (m_data.eq32 != val) { m_data.eq32 = val; emit eq32Changed(); emit modified(); } }
double AudioEffect::eq64() const { return m_data.eq64; } void AudioEffect::setEq64(double val) { if (m_data.eq64 != val) { m_data.eq64 = val; emit eq64Changed(); emit modified(); } }
double AudioEffect::eq125() const { return m_data.eq125; } void AudioEffect::setEq125(double val) { if (m_data.eq125 != val) { m_data.eq125 = val; emit eq125Changed(); emit modified(); } }
double AudioEffect::eq250() const { return m_data.eq250; } void AudioEffect::setEq250(double val) { if (m_data.eq250 != val) { m_data.eq250 = val; emit eq250Changed(); emit modified(); } }
double AudioEffect::eq500() const { return m_data.eq500; } void AudioEffect::setEq500(double val) { if (m_data.eq500 != val) { m_data.eq500 = val; emit eq500Changed(); emit modified(); } }
double AudioEffect::eq1k() const { return m_data.eq1k; } void AudioEffect::setEq1k(double val) { if (m_data.eq1k != val) { m_data.eq1k = val; emit eq1kChanged(); emit modified(); } }
double AudioEffect::eq2k() const { return m_data.eq2k; } void AudioEffect::setEq2k(double val) { if (m_data.eq2k != val) { m_data.eq2k = val; emit eq2kChanged(); emit modified(); } }
double AudioEffect::eq4k() const { return m_data.eq4k; } void AudioEffect::setEq4k(double val) { if (m_data.eq4k != val) { m_data.eq4k = val; emit eq4kChanged(); emit modified(); } }
double AudioEffect::eq8k() const { return m_data.eq8k; } void AudioEffect::setEq8k(double val) { if (m_data.eq8k != val) { m_data.eq8k = val; emit eq8kChanged(); emit modified(); } }
double AudioEffect::eq16k() const { return m_data.eq16k; } void AudioEffect::setEq16k(double val) { if (m_data.eq16k != val) { m_data.eq16k = val; emit eq16kChanged(); emit modified(); } }

void AudioEffect::loadData(const AudioEffectData& data) {
    if (m_data.volume != data.volume) { m_data.volume = data.volume; emit volumeChanged(); }
    if (m_data.pan != data.pan) { m_data.pan = data.pan; emit panChanged(); }
    if (m_data.pitch != data.pitch) { m_data.pitch = data.pitch; emit pitchChanged(); }
    if (m_data.eq32 != data.eq32) { m_data.eq32 = data.eq32; emit eq32Changed(); }
    if (m_data.eq64 != data.eq64) { m_data.eq64 = data.eq64; emit eq64Changed(); }
    if (m_data.eq125 != data.eq125) { m_data.eq125 = data.eq125; emit eq125Changed(); }
    if (m_data.eq250 != data.eq250) { m_data.eq250 = data.eq250; emit eq250Changed(); }
    if (m_data.eq500 != data.eq500) { m_data.eq500 = data.eq500; emit eq500Changed(); }
    if (m_data.eq1k != data.eq1k) { m_data.eq1k = data.eq1k; emit eq1kChanged(); }
    if (m_data.eq2k != data.eq2k) { m_data.eq2k = data.eq2k; emit eq2kChanged(); }
    if (m_data.eq4k != data.eq4k) { m_data.eq4k = data.eq4k; emit eq4kChanged(); }
    if (m_data.eq8k != data.eq8k) { m_data.eq8k = data.eq8k; emit eq8kChanged(); }
    if (m_data.eq16k != data.eq16k) { m_data.eq16k = data.eq16k; emit eq16kChanged(); }
}

AudioEffectData AudioEffect::toData() const {
    return m_data;
}
