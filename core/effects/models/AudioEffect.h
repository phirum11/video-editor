#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class AudioEffect : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double pan READ pan WRITE setPan NOTIFY panChanged)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
    Q_PROPERTY(double eq32 READ eq32 WRITE setEq32 NOTIFY eq32Changed)
    Q_PROPERTY(double eq64 READ eq64 WRITE setEq64 NOTIFY eq64Changed)
    Q_PROPERTY(double eq125 READ eq125 WRITE setEq125 NOTIFY eq125Changed)
    Q_PROPERTY(double eq250 READ eq250 WRITE setEq250 NOTIFY eq250Changed)
    Q_PROPERTY(double eq500 READ eq500 WRITE setEq500 NOTIFY eq500Changed)
    Q_PROPERTY(double eq1k READ eq1k WRITE setEq1k NOTIFY eq1kChanged)
    Q_PROPERTY(double eq2k READ eq2k WRITE setEq2k NOTIFY eq2kChanged)
    Q_PROPERTY(double eq4k READ eq4k WRITE setEq4k NOTIFY eq4kChanged)
    Q_PROPERTY(double eq8k READ eq8k WRITE setEq8k NOTIFY eq8kChanged)
    Q_PROPERTY(double eq16k READ eq16k WRITE setEq16k NOTIFY eq16kChanged)

public:
    explicit AudioEffect(QObject *parent = nullptr);

    double volume() const;
    void setVolume(double val);

    double pan() const;
    void setPan(double val);

    double pitch() const;
    void setPitch(double val);

    double eq32() const; void setEq32(double val);
    double eq64() const; void setEq64(double val);
    double eq125() const; void setEq125(double val);
    double eq250() const; void setEq250(double val);
    double eq500() const; void setEq500(double val);
    double eq1k() const; void setEq1k(double val);
    double eq2k() const; void setEq2k(double val);
    double eq4k() const; void setEq4k(double val);
    double eq8k() const; void setEq8k(double val);
    double eq16k() const; void setEq16k(double val);

    void loadData(const AudioEffectData& data);
    AudioEffectData toData() const;

signals:
    void volumeChanged();
    void panChanged();
    void pitchChanged();
    void eq32Changed();
    void eq64Changed();
    void eq125Changed();
    void eq250Changed();
    void eq500Changed();
    void eq1kChanged();
    void eq2kChanged();
    void eq4kChanged();
    void eq8kChanged();
    void eq16kChanged();
    void modified();

private:
    AudioEffectData m_data;
};
