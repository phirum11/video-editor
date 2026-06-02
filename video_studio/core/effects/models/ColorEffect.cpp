#include "ColorEffect.h"

ColorEffect::ColorEffect(QObject *parent) : QObject(parent)
{
}

double ColorEffect::brightness() const { return m_data.brightness; }
void ColorEffect::setBrightness(double val) { 
    if (m_data.brightness != val) { 
        m_data.brightness = val; 
        emit brightnessChanged(); 
        emit modified(); 
    } 
}

double ColorEffect::contrast() const { return m_data.contrast; }
void ColorEffect::setContrast(double val) { 
    if (m_data.contrast != val) { 
        m_data.contrast = val; 
        emit contrastChanged(); 
        emit modified(); 
    } 
}

double ColorEffect::saturation() const { return m_data.saturation; }
void ColorEffect::setSaturation(double val) { 
    if (m_data.saturation != val) { 
        m_data.saturation = val; 
        emit saturationChanged(); 
        emit modified(); 
    } 
}

void ColorEffect::loadData(const ColorEffectData& data) {
    if (m_data.brightness != data.brightness) { m_data.brightness = data.brightness; emit brightnessChanged(); }
    if (m_data.contrast != data.contrast) { m_data.contrast = data.contrast; emit contrastChanged(); }
    if (m_data.saturation != data.saturation) { m_data.saturation = data.saturation; emit saturationChanged(); }
}

ColorEffectData ColorEffect::toData() const {
    return m_data;
}
