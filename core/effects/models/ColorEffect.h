#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class ColorEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(double contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(double saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)

public:
    explicit ColorEffect(QObject *parent = nullptr);

    double brightness() const;
    void setBrightness(double val);

    double contrast() const;
    void setContrast(double val);

    double saturation() const;
    void setSaturation(double val);

    void loadData(const ColorEffectData& data);
    ColorEffectData toData() const;

signals:
    void brightnessChanged();
    void contrastChanged();
    void saturationChanged();
    void modified();

private:
    ColorEffectData m_data;
};
