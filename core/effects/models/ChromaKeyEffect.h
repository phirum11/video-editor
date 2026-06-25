#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class ChromaKeyEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(double variance READ variance WRITE setVariance NOTIFY varianceChanged)
    Q_PROPERTY(double softness READ softness WRITE setSoftness NOTIFY softnessChanged)
    Q_PROPERTY(bool spillSuppress READ spillSuppress WRITE setSpillSuppress NOTIFY spillSuppressChanged)

public:
    explicit ChromaKeyEffect(QObject *parent = nullptr);

    bool enabled() const;
    void setEnabled(bool val);

    QString color() const;
    void setColor(const QString& val);

    double variance() const;
    void setVariance(double val);

    double softness() const;
    void setSoftness(double val);

    bool spillSuppress() const;
    void setSpillSuppress(bool val);

    void loadData(const ChromaKeyEffectData& data);
    ChromaKeyEffectData toData() const;

signals:
    void enabledChanged();
    void colorChanged();
    void varianceChanged();
    void softnessChanged();
    void spillSuppressChanged();
    void modified();

private:
    ChromaKeyEffectData m_data;
};
