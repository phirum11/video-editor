#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class ColorEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(double contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(double saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
    
    Q_PROPERTY(QString shadowsColor READ shadowsColor WRITE setShadowsColor NOTIFY shadowsColorChanged)
    Q_PROPERTY(QString midtonesColor READ midtonesColor WRITE setMidtonesColor NOTIFY midtonesColorChanged)
    Q_PROPERTY(QString highlightsColor READ highlightsColor WRITE setHighlightsColor NOTIFY highlightsColorChanged)
    
    Q_PROPERTY(double vignetteAmount READ vignetteAmount WRITE setVignetteAmount NOTIFY vignetteAmountChanged)
    Q_PROPERTY(double vignetteFeather READ vignetteFeather WRITE setVignetteFeather NOTIFY vignetteFeatherChanged)

public:
    explicit ColorEffect(QObject *parent = nullptr);

    double brightness() const;
    void setBrightness(double val);

    double contrast() const;
    void setContrast(double val);

    double saturation() const;
    void setSaturation(double val);

    QString shadowsColor() const;
    void setShadowsColor(const QString& val);

    QString midtonesColor() const;
    void setMidtonesColor(const QString& val);

    QString highlightsColor() const;
    void setHighlightsColor(const QString& val);

    double vignetteAmount() const;
    void setVignetteAmount(double val);

    double vignetteFeather() const;
    void setVignetteFeather(double val);

    void loadData(const ColorEffectData& data);
    ColorEffectData toData() const;

signals:
    void brightnessChanged();
    void contrastChanged();
    void saturationChanged();
    void shadowsColorChanged();
    void midtonesColorChanged();
    void highlightsColorChanged();
    void vignetteAmountChanged();
    void vignetteFeatherChanged();
    void modified();

private:
    ColorEffectData m_data;
};
