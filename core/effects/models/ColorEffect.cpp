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

QString ColorEffect::shadowsColor() const { return m_data.shadowsColor; }
void ColorEffect::setShadowsColor(const QString& val) {
    if (m_data.shadowsColor != val) {
        m_data.shadowsColor = val;
        emit shadowsColorChanged();
        emit modified();
    }
}

QString ColorEffect::midtonesColor() const { return m_data.midtonesColor; }
void ColorEffect::setMidtonesColor(const QString& val) {
    if (m_data.midtonesColor != val) {
        m_data.midtonesColor = val;
        emit midtonesColorChanged();
        emit modified();
    }
}

QString ColorEffect::highlightsColor() const { return m_data.highlightsColor; }
void ColorEffect::setHighlightsColor(const QString& val) {
    if (m_data.highlightsColor != val) {
        m_data.highlightsColor = val;
        emit highlightsColorChanged();
        emit modified();
    }
}

double ColorEffect::vignetteAmount() const { return m_data.vignetteAmount; }
void ColorEffect::setVignetteAmount(double val) {
    if (m_data.vignetteAmount != val) {
        m_data.vignetteAmount = val;
        emit vignetteAmountChanged();
        emit modified();
    }
}

double ColorEffect::vignetteFeather() const { return m_data.vignetteFeather; }
void ColorEffect::setVignetteFeather(double val) {
    if (m_data.vignetteFeather != val) {
        m_data.vignetteFeather = val;
        emit vignetteFeatherChanged();
        emit modified();
    }
}

void ColorEffect::loadData(const ColorEffectData& data) {
    if (m_data.brightness != data.brightness) { m_data.brightness = data.brightness; emit brightnessChanged(); }
    if (m_data.contrast != data.contrast) { m_data.contrast = data.contrast; emit contrastChanged(); }
    if (m_data.saturation != data.saturation) { m_data.saturation = data.saturation; emit saturationChanged(); }
    if (m_data.shadowsColor != data.shadowsColor) { m_data.shadowsColor = data.shadowsColor; emit shadowsColorChanged(); }
    if (m_data.midtonesColor != data.midtonesColor) { m_data.midtonesColor = data.midtonesColor; emit midtonesColorChanged(); }
    if (m_data.highlightsColor != data.highlightsColor) { m_data.highlightsColor = data.highlightsColor; emit highlightsColorChanged(); }
    if (m_data.vignetteAmount != data.vignetteAmount) { m_data.vignetteAmount = data.vignetteAmount; emit vignetteAmountChanged(); }
    if (m_data.vignetteFeather != data.vignetteFeather) { m_data.vignetteFeather = data.vignetteFeather; emit vignetteFeatherChanged(); }
}

ColorEffectData ColorEffect::toData() const {
    return m_data;
}
