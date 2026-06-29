#include "PaintDynamicsEngine.h"
#include <QtMath>
#include <QDebug>

PaintDynamicsEngine::PaintDynamicsEngine(QObject *parent)
    : QObject(parent)
{
}

PaintDynamicsEngine::~PaintDynamicsEngine() = default;

void PaintDynamicsEngine::setVelocityScaling(qreal scale)
{
    if (!qFuzzyCompare(m_velocityScaling, scale)) {
        m_velocityScaling = scale;
        emit velocityScalingChanged();
    }
}

void PaintDynamicsEngine::setPressureScaling(qreal scale)
{
    if (!qFuzzyCompare(m_pressureScaling, scale)) {
        m_pressureScaling = scale;
        emit pressureScalingChanged();
    }
}

QVector<StrokeSample> PaintDynamicsEngine::interpolateStroke(const StrokeSample& start, const StrokeSample& end, qreal stepPixelSize)
{
    QVector<StrokeSample> samples;
    qreal dx = end.position.x() - start.position.x();
    qreal dy = end.position.y() - start.position.y();
    qreal dist = qSqrt(dx * dx + dy * dy);

    if (dist <= stepPixelSize || stepPixelSize <= 0.0) {
        samples.append(end);
        return samples;
    }

    int steps = static_cast<int>(dist / stepPixelSize);
    for (int i = 1; i <= steps; ++i) {
        qreal t = static_cast<qreal>(i) / steps;
        StrokeSample s;
        s.position = QPointF(start.position.x() + dx * t, start.position.y() + dy * t);
        s.pressure = start.pressure + (end.pressure - start.pressure) * t;
        s.tiltX = start.tiltX + (end.tiltX - start.tiltX) * t;
        s.tiltY = start.tiltY + (end.tiltY - start.tiltY) * t;
        s.velocity = start.velocity + (end.velocity - start.velocity) * t;
        samples.append(s);
    }

    return samples;
}
