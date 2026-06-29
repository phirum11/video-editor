#include "StylizeEffect.h"

StylizeEffect::StylizeEffect(QObject *parent) : QObject(parent)
{
}

QString StylizeEffect::styleName() const { return m_data.styleName; }
void StylizeEffect::setStyleName(const QString& name) { 
    if (m_data.styleName != name) { 
        m_data.styleName = name; 
        emit styleNameChanged(); 
        emit modified(); 
    } 
}

double StylizeEffect::intensity() const { return m_data.intensity; }
void StylizeEffect::setIntensity(double val) { 
    if (m_data.intensity != val) { 
        m_data.intensity = val; 
        emit intensityChanged(); 
        emit modified(); 
    } 
}

QString StylizeEffect::gifFilePath() const { return m_data.gifFilePath; }
void StylizeEffect::setGifFilePath(const QString& val) {
    if (m_data.gifFilePath != val) {
        m_data.gifFilePath = val;
        emit gifFilePathChanged();
        emit modified();
    }
}

bool StylizeEffect::audioVisualizerEnabled() const { return m_data.audioVisualizerEnabled; }
void StylizeEffect::setAudioVisualizerEnabled(bool val) {
    if (m_data.audioVisualizerEnabled != val) {
        m_data.audioVisualizerEnabled = val;
        emit audioVisualizerEnabledChanged();
        emit modified();
    }
}

double StylizeEffect::audioVisualizerBands() const { return m_data.audioVisualizerBands; }
void StylizeEffect::setAudioVisualizerBands(double val) {
    if (m_data.audioVisualizerBands != val) {
        m_data.audioVisualizerBands = val;
        emit audioVisualizerBandsChanged();
        emit modified();
    }
}

double StylizeEffect::audioVisualizerSmoothing() const { return m_data.audioVisualizerSmoothing; }
void StylizeEffect::setAudioVisualizerSmoothing(double val) {
    if (m_data.audioVisualizerSmoothing != val) {
        m_data.audioVisualizerSmoothing = val;
        emit audioVisualizerSmoothingChanged();
        emit modified();
    }
}

bool StylizeEffect::dropShadowEnabled() const { return m_data.dropShadowEnabled; }
void StylizeEffect::setDropShadowEnabled(bool val) {
    if (m_data.dropShadowEnabled != val) {
        m_data.dropShadowEnabled = val;
        emit dropShadowEnabledChanged();
        emit modified();
    }
}

double StylizeEffect::dropShadowDistance() const { return m_data.dropShadowDistance; }
void StylizeEffect::setDropShadowDistance(double val) {
    if (m_data.dropShadowDistance != val) {
        m_data.dropShadowDistance = val;
        emit dropShadowDistanceChanged();
        emit modified();
    }
}

double StylizeEffect::dropShadowOpacity() const { return m_data.dropShadowOpacity; }
void StylizeEffect::setDropShadowOpacity(double val) {
    if (m_data.dropShadowOpacity != val) {
        m_data.dropShadowOpacity = val;
        emit dropShadowOpacityChanged();
        emit modified();
    }
}

double StylizeEffect::dropShadowSoftness() const { return m_data.dropShadowSoftness; }
void StylizeEffect::setDropShadowSoftness(double val) {
    if (m_data.dropShadowSoftness != val) {
        m_data.dropShadowSoftness = val;
        emit dropShadowSoftnessChanged();
        emit modified();
    }
}

bool StylizeEffect::glitchEnabled() const { return m_data.glitchEnabled; }
void StylizeEffect::setGlitchEnabled(bool val) {
    if (m_data.glitchEnabled != val) {
        m_data.glitchEnabled = val;
        emit glitchEnabledChanged();
        emit modified();
    }
}

double StylizeEffect::glitchWaveWarp() const { return m_data.glitchWaveWarp; }
void StylizeEffect::setGlitchWaveWarp(double val) {
    if (m_data.glitchWaveWarp != val) {
        m_data.glitchWaveWarp = val;
        emit glitchWaveWarpChanged();
        emit modified();
    }
}

double StylizeEffect::glitchRGBSplit() const { return m_data.glitchRGBSplit; }
void StylizeEffect::setGlitchRGBSplit(double val) {
    if (m_data.glitchRGBSplit != val) {
        m_data.glitchRGBSplit = val;
        emit glitchRGBSplitChanged();
        emit modified();
    }
}

double StylizeEffect::glitchLensDistortion() const { return m_data.glitchLensDistortion; }
void StylizeEffect::setGlitchLensDistortion(double val) {
    if (m_data.glitchLensDistortion != val) {
        m_data.glitchLensDistortion = val;
        emit glitchLensDistortionChanged();
        emit modified();
    }
}

void StylizeEffect::loadData(const StylizeEffectData& data) {
    if (m_data.styleName != data.styleName) { m_data.styleName = data.styleName; emit styleNameChanged(); }
    if (m_data.intensity != data.intensity) { m_data.intensity = data.intensity; emit intensityChanged(); }
    if (m_data.gifFilePath != data.gifFilePath) { m_data.gifFilePath = data.gifFilePath; emit gifFilePathChanged(); }
    if (m_data.audioVisualizerEnabled != data.audioVisualizerEnabled) { m_data.audioVisualizerEnabled = data.audioVisualizerEnabled; emit audioVisualizerEnabledChanged(); }
    if (m_data.audioVisualizerBands != data.audioVisualizerBands) { m_data.audioVisualizerBands = data.audioVisualizerBands; emit audioVisualizerBandsChanged(); }
    if (m_data.audioVisualizerSmoothing != data.audioVisualizerSmoothing) { m_data.audioVisualizerSmoothing = data.audioVisualizerSmoothing; emit audioVisualizerSmoothingChanged(); }
    if (m_data.dropShadowEnabled != data.dropShadowEnabled) { m_data.dropShadowEnabled = data.dropShadowEnabled; emit dropShadowEnabledChanged(); }
    if (m_data.dropShadowDistance != data.dropShadowDistance) { m_data.dropShadowDistance = data.dropShadowDistance; emit dropShadowDistanceChanged(); }
    if (m_data.dropShadowOpacity != data.dropShadowOpacity) { m_data.dropShadowOpacity = data.dropShadowOpacity; emit dropShadowOpacityChanged(); }
    if (m_data.dropShadowSoftness != data.dropShadowSoftness) { m_data.dropShadowSoftness = data.dropShadowSoftness; emit dropShadowSoftnessChanged(); }
    if (m_data.glitchEnabled != data.glitchEnabled) { m_data.glitchEnabled = data.glitchEnabled; emit glitchEnabledChanged(); }
    if (m_data.glitchWaveWarp != data.glitchWaveWarp) { m_data.glitchWaveWarp = data.glitchWaveWarp; emit glitchWaveWarpChanged(); }
    if (m_data.glitchRGBSplit != data.glitchRGBSplit) { m_data.glitchRGBSplit = data.glitchRGBSplit; emit glitchRGBSplitChanged(); }
    if (m_data.glitchLensDistortion != data.glitchLensDistortion) { m_data.glitchLensDistortion = data.glitchLensDistortion; emit glitchLensDistortionChanged(); }
}

StylizeEffectData StylizeEffect::toData() const {
    return m_data;
}
