#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class StylizeEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString styleName READ styleName WRITE setStyleName NOTIFY styleNameChanged)
    Q_PROPERTY(double intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
    Q_PROPERTY(QString gifFilePath READ gifFilePath WRITE setGifFilePath NOTIFY gifFilePathChanged)

    Q_PROPERTY(bool audioVisualizerEnabled READ audioVisualizerEnabled WRITE setAudioVisualizerEnabled NOTIFY audioVisualizerEnabledChanged)
    Q_PROPERTY(double audioVisualizerBands READ audioVisualizerBands WRITE setAudioVisualizerBands NOTIFY audioVisualizerBandsChanged)
    Q_PROPERTY(double audioVisualizerSmoothing READ audioVisualizerSmoothing WRITE setAudioVisualizerSmoothing NOTIFY audioVisualizerSmoothingChanged)

    Q_PROPERTY(bool dropShadowEnabled READ dropShadowEnabled WRITE setDropShadowEnabled NOTIFY dropShadowEnabledChanged)
    Q_PROPERTY(double dropShadowDistance READ dropShadowDistance WRITE setDropShadowDistance NOTIFY dropShadowDistanceChanged)
    Q_PROPERTY(double dropShadowOpacity READ dropShadowOpacity WRITE setDropShadowOpacity NOTIFY dropShadowOpacityChanged)
    Q_PROPERTY(double dropShadowSoftness READ dropShadowSoftness WRITE setDropShadowSoftness NOTIFY dropShadowSoftnessChanged)

    Q_PROPERTY(bool glitchEnabled READ glitchEnabled WRITE setGlitchEnabled NOTIFY glitchEnabledChanged)
    Q_PROPERTY(double glitchWaveWarp READ glitchWaveWarp WRITE setGlitchWaveWarp NOTIFY glitchWaveWarpChanged)
    Q_PROPERTY(double glitchRGBSplit READ glitchRGBSplit WRITE setGlitchRGBSplit NOTIFY glitchRGBSplitChanged)
    Q_PROPERTY(double glitchLensDistortion READ glitchLensDistortion WRITE setGlitchLensDistortion NOTIFY glitchLensDistortionChanged)

public:
    explicit StylizeEffect(QObject *parent = nullptr);

    QString styleName() const;
    void setStyleName(const QString& name);

    double intensity() const;
    void setIntensity(double val);

    QString gifFilePath() const;
    void setGifFilePath(const QString& val);

    bool audioVisualizerEnabled() const;
    void setAudioVisualizerEnabled(bool val);

    double audioVisualizerBands() const;
    void setAudioVisualizerBands(double val);

    double audioVisualizerSmoothing() const;
    void setAudioVisualizerSmoothing(double val);

    bool dropShadowEnabled() const;
    void setDropShadowEnabled(bool val);

    double dropShadowDistance() const;
    void setDropShadowDistance(double val);

    double dropShadowOpacity() const;
    void setDropShadowOpacity(double val);

    double dropShadowSoftness() const;
    void setDropShadowSoftness(double val);

    bool glitchEnabled() const;
    void setGlitchEnabled(bool val);

    double glitchWaveWarp() const;
    void setGlitchWaveWarp(double val);

    double glitchRGBSplit() const;
    void setGlitchRGBSplit(double val);

    double glitchLensDistortion() const;
    void setGlitchLensDistortion(double val);

    void loadData(const StylizeEffectData& data);
    StylizeEffectData toData() const;

signals:
    void styleNameChanged();
    void intensityChanged();
    void gifFilePathChanged();
    void audioVisualizerEnabledChanged();
    void audioVisualizerBandsChanged();
    void audioVisualizerSmoothingChanged();
    void dropShadowEnabledChanged();
    void dropShadowDistanceChanged();
    void dropShadowOpacityChanged();
    void dropShadowSoftnessChanged();
    void glitchEnabledChanged();
    void glitchWaveWarpChanged();
    void glitchRGBSplitChanged();
    void glitchLensDistortionChanged();
    void modified();

private:
    StylizeEffectData m_data;
};
