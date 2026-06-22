#include "core/effects/models/ChromaKeyEffect.h"

ChromaKeyEffect::ChromaKeyEffect(QObject *parent) : QObject(parent)
{
}

bool ChromaKeyEffect::enabled() const
{
    return m_data.enabled;
}

void ChromaKeyEffect::setEnabled(bool val)
{
    if (m_data.enabled == val)
        return;
    m_data.enabled = val;
    emit enabledChanged();
    emit modified();
}

QString ChromaKeyEffect::color() const
{
    return m_data.color;
}

void ChromaKeyEffect::setColor(const QString& val)
{
    if (m_data.color == val)
        return;
    m_data.color = val;
    emit colorChanged();
    emit modified();
}

double ChromaKeyEffect::variance() const
{
    return m_data.variance;
}

void ChromaKeyEffect::setVariance(double val)
{
    if (qFuzzyCompare(m_data.variance, val))
        return;
    m_data.variance = val;
    emit varianceChanged();
    emit modified();
}

double ChromaKeyEffect::softness() const
{
    return m_data.softness;
}

void ChromaKeyEffect::setSoftness(double val)
{
    if (qFuzzyCompare(m_data.softness, val))
        return;
    m_data.softness = val;
    emit softnessChanged();
    emit modified();
}

bool ChromaKeyEffect::spillSuppress() const
{
    return m_data.spillSuppress;
}

void ChromaKeyEffect::setSpillSuppress(bool val)
{
    if (m_data.spillSuppress == val)
        return;
    m_data.spillSuppress = val;
    emit spillSuppressChanged();
    emit modified();
}

void ChromaKeyEffect::loadData(const ChromaKeyEffectData& data)
{
    setEnabled(data.enabled);
    setColor(data.color);
    setVariance(data.variance);
    setSoftness(data.softness);
    setSpillSuppress(data.spillSuppress);
}

ChromaKeyEffectData ChromaKeyEffect::toData() const
{
    return m_data;
}
