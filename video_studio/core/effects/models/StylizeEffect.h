#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "core/effects/EffectData.h"

class StylizeEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString styleName READ styleName WRITE setStyleName NOTIFY styleNameChanged)
    Q_PROPERTY(double intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)

public:
    explicit StylizeEffect(QObject *parent = nullptr);

    QString styleName() const;
    void setStyleName(const QString& name);

    double intensity() const;
    void setIntensity(double val);

    void loadData(const StylizeEffectData& data);
    StylizeEffectData toData() const;

signals:
    void styleNameChanged();
    void intensityChanged();
    void modified();

private:
    StylizeEffectData m_data;
};
