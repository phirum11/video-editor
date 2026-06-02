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

void StylizeEffect::loadData(const StylizeEffectData& data) {
    if (m_data.styleName != data.styleName) { m_data.styleName = data.styleName; emit styleNameChanged(); }
    if (m_data.intensity != data.intensity) { m_data.intensity = data.intensity; emit intensityChanged(); }
}

StylizeEffectData StylizeEffect::toData() const {
    return m_data;
}
