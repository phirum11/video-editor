#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class BlurEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QString blurType READ blurType WRITE setBlurType NOTIFY blurTypeChanged)
    
    Q_PROPERTY(bool isRegionEnabled READ isRegionEnabled WRITE setIsRegionEnabled NOTIFY isRegionEnabledChanged)
    Q_PROPERTY(double regionX READ regionX WRITE setRegionX NOTIFY regionXChanged)
    Q_PROPERTY(double regionY READ regionY WRITE setRegionY NOTIFY regionYChanged)
    Q_PROPERTY(double regionWidth READ regionWidth WRITE setRegionWidth NOTIFY regionWidthChanged)
    Q_PROPERTY(double regionHeight READ regionHeight WRITE setRegionHeight NOTIFY regionHeightChanged)

public:
    explicit BlurEffect(QObject *parent = nullptr);

    double radius() const;
    void setRadius(double val);

    QString blurType() const;
    void setBlurType(const QString& type);

    bool isRegionEnabled() const;
    void setIsRegionEnabled(bool enabled);

    double regionX() const;
    void setRegionX(double val);

    double regionY() const;
    void setRegionY(double val);

    double regionWidth() const;
    void setRegionWidth(double val);

    double regionHeight() const;
    void setRegionHeight(double val);

    Q_INVOKABLE void updateRegion(double x, double y, double width, double height);

    void loadData(const BlurEffectData& data);
    BlurEffectData toData() const;

signals:
    void radiusChanged();
    void blurTypeChanged();
    
    void isRegionEnabledChanged();
    void regionXChanged();
    void regionYChanged();
    void regionWidthChanged();
    void regionHeightChanged();
    
    void modified();

private:
    BlurEffectData m_data;
};
