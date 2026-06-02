#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "core/effects/EffectData.h"

class AudioEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double pan READ pan WRITE setPan NOTIFY panChanged)

public:
    explicit AudioEffect(QObject *parent = nullptr);

    double volume() const;
    void setVolume(double val);

    double pan() const;
    void setPan(double val);

    void loadData(const AudioEffectData& data);
    AudioEffectData toData() const;

signals:
    void volumeChanged();
    void panChanged();
    void modified();

private:
    AudioEffectData m_data;
};
