#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "core/effects/EffectData.h"

class TransformEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(double posX READ posX WRITE setPosX NOTIFY posXChanged)
    Q_PROPERTY(double posY READ posY WRITE setPosY NOTIFY posYChanged)
    Q_PROPERTY(double rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(double anchorX READ anchorX WRITE setAnchorX NOTIFY anchorXChanged)
    Q_PROPERTY(double anchorY READ anchorY WRITE setAnchorY NOTIFY anchorYChanged)
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QString blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)

public:
    explicit TransformEffect(QObject *parent = nullptr);

    double scale() const;
    void setScale(double val);

    double posX() const;
    void setPosX(double val);

    double posY() const;
    void setPosY(double val);

    double rotation() const;
    void setRotation(double val);

    double anchorX() const;
    void setAnchorX(double val);

    double anchorY() const;
    void setAnchorY(double val);

    double opacity() const;
    void setOpacity(double val);

    QString blendMode() const;
    void setBlendMode(const QString& mode);

    void loadData(const TransformEffectData& data);
    TransformEffectData toData() const;

signals:
    void scaleChanged();
    void posXChanged();
    void posYChanged();
    void rotationChanged();
    void anchorXChanged();
    void anchorYChanged();
    void opacityChanged();
    void blendModeChanged();
    
    // Emitted whenever any property changes so the main controller can save back to TimelineClipModel
    void modified();

private:
    TransformEffectData m_data;
};
