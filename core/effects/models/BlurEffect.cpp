#include "BlurEffect.h"

#include <algorithm>
#include <cmath>

namespace {

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) < 0.000001;
}

double clampUnit(double value)
{
    if (!std::isfinite(value)) {
        return 0.0;
    }
    return std::clamp(value, 0.0, 1.0);
}

BlurEffectData normalizedData(BlurEffectData data)
{
    if (!std::isfinite(data.radius)) {
        data.radius = 0.0;
    }
    data.radius = std::clamp(data.radius, 0.0, 100.0);

    data.regionWidth = std::clamp(std::isfinite(data.regionWidth) ? data.regionWidth : 0.5, 0.001, 1.0);
    data.regionHeight = std::clamp(std::isfinite(data.regionHeight) ? data.regionHeight : 0.5, 0.001, 1.0);
    data.regionX = clampUnit(data.regionX);
    data.regionY = clampUnit(data.regionY);

    if (data.regionX + data.regionWidth > 1.0) {
        data.regionX = 1.0 - data.regionWidth;
    }
    if (data.regionY + data.regionHeight > 1.0) {
        data.regionY = 1.0 - data.regionHeight;
    }

    return data;
}

} // namespace

BlurEffect::BlurEffect(QObject *parent) : QObject(parent)
{
}

double BlurEffect::radius() const { return m_data.radius; }
void BlurEffect::setRadius(double val) { 
    const double clamped = std::clamp(std::isfinite(val) ? val : 0.0, 0.0, 100.0);
    if (!nearlyEqual(m_data.radius, clamped)) { 
        m_data.radius = clamped; 
        emit radiusChanged(); 
        emit modified(); 
    } 
}

QString BlurEffect::blurType() const { return m_data.blurType; }
void BlurEffect::setBlurType(const QString& type) { 
    if (m_data.blurType != type) { 
        m_data.blurType = type; 
        emit blurTypeChanged(); 
        emit modified(); 
    } 
}

bool BlurEffect::isRegionEnabled() const { return m_data.isRegionEnabled; }
void BlurEffect::setIsRegionEnabled(bool enabled) {
    if (m_data.isRegionEnabled != enabled) {
        m_data.isRegionEnabled = enabled;
        emit isRegionEnabledChanged();
        emit modified();
    }
}

double BlurEffect::regionX() const { return m_data.regionX; }
void BlurEffect::setRegionX(double val) {
    const double clamped = std::clamp(clampUnit(val), 0.0, 1.0 - m_data.regionWidth);
    if (!nearlyEqual(m_data.regionX, clamped)) {
        m_data.regionX = clamped;
        emit regionXChanged();
        emit modified();
    }
}

double BlurEffect::regionY() const { return m_data.regionY; }
void BlurEffect::setRegionY(double val) {
    const double clamped = std::clamp(clampUnit(val), 0.0, 1.0 - m_data.regionHeight);
    if (!nearlyEqual(m_data.regionY, clamped)) {
        m_data.regionY = clamped;
        emit regionYChanged();
        emit modified();
    }
}

double BlurEffect::regionWidth() const { return m_data.regionWidth; }
void BlurEffect::setRegionWidth(double val) {
    const double maxWidth = std::max(0.001, 1.0 - m_data.regionX);
    const double clamped = std::clamp(std::isfinite(val) ? val : m_data.regionWidth, 0.001, maxWidth);
    if (!nearlyEqual(m_data.regionWidth, clamped)) {
        m_data.regionWidth = clamped;
        emit regionWidthChanged();
        emit modified();
    }
}

double BlurEffect::regionHeight() const { return m_data.regionHeight; }
void BlurEffect::setRegionHeight(double val) {
    const double maxHeight = std::max(0.001, 1.0 - m_data.regionY);
    const double clamped = std::clamp(std::isfinite(val) ? val : m_data.regionHeight, 0.001, maxHeight);
    if (!nearlyEqual(m_data.regionHeight, clamped)) {
        m_data.regionHeight = clamped;
        emit regionHeightChanged();
        emit modified();
    }
}

void BlurEffect::updateRegion(double x, double y, double width, double height) {
    BlurEffectData next = m_data;
    next.regionX = x;
    next.regionY = y;
    next.regionWidth = width;
    next.regionHeight = height;
    next = normalizedData(next);

    bool changed = false;
    if (!nearlyEqual(m_data.regionX, next.regionX)) { m_data.regionX = next.regionX; emit regionXChanged(); changed = true; }
    if (!nearlyEqual(m_data.regionY, next.regionY)) { m_data.regionY = next.regionY; emit regionYChanged(); changed = true; }
    if (!nearlyEqual(m_data.regionWidth, next.regionWidth)) { m_data.regionWidth = next.regionWidth; emit regionWidthChanged(); changed = true; }
    if (!nearlyEqual(m_data.regionHeight, next.regionHeight)) { m_data.regionHeight = next.regionHeight; emit regionHeightChanged(); changed = true; }
    if (changed) {
        emit modified();
    }
}

void BlurEffect::loadData(const BlurEffectData& data) {
    const BlurEffectData next = normalizedData(data);
    if (!nearlyEqual(m_data.radius, next.radius)) { m_data.radius = next.radius; emit radiusChanged(); }
    if (m_data.blurType != next.blurType) { m_data.blurType = next.blurType; emit blurTypeChanged(); }
    if (m_data.isRegionEnabled != next.isRegionEnabled) { m_data.isRegionEnabled = next.isRegionEnabled; emit isRegionEnabledChanged(); }
    if (!nearlyEqual(m_data.regionX, next.regionX)) { m_data.regionX = next.regionX; emit regionXChanged(); }
    if (!nearlyEqual(m_data.regionY, next.regionY)) { m_data.regionY = next.regionY; emit regionYChanged(); }
    if (!nearlyEqual(m_data.regionWidth, next.regionWidth)) { m_data.regionWidth = next.regionWidth; emit regionWidthChanged(); }
    if (!nearlyEqual(m_data.regionHeight, next.regionHeight)) { m_data.regionHeight = next.regionHeight; emit regionHeightChanged(); }
}

BlurEffectData BlurEffect::toData() const {
    return m_data;
}
