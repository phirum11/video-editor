#pragma once

#include <QObject>
#include <QPointF>
#include <QVector>
#include <QImage>

struct StrokeSample {
    QPointF position;
    qreal pressure;
    qreal tiltX;
    qreal tiltY;
    qreal velocity;
};

class PaintDynamicsEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal velocityScaling READ velocityScaling WRITE setVelocityScaling NOTIFY velocityScalingChanged)
    Q_PROPERTY(qreal pressureScaling READ pressureScaling WRITE setPressureScaling NOTIFY pressureScalingChanged)

public:
    explicit PaintDynamicsEngine(QObject *parent = nullptr);
    ~PaintDynamicsEngine() override;

    qreal velocityScaling() const { return m_velocityScaling; }
    void setVelocityScaling(qreal scale);

    qreal pressureScaling() const { return m_pressureScaling; }
    void setPressureScaling(qreal scale);

    Q_INVOKABLE QVector<StrokeSample> interpolateStroke(const StrokeSample& start, const StrokeSample& end, qreal stepPixelSize);

signals:
    void velocityScalingChanged();
    void pressureScalingChanged();

private:
    qreal m_velocityScaling = 1.0;
    qreal m_pressureScaling = 1.0;
};
