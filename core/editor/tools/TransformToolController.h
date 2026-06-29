#pragma once

#include <QObject>
#include <QTransform>
#include <QPointF>
#include <QString>
#include <QVariantMap>

class TransformToolController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QTransform currentTransform READ currentTransform NOTIFY transformChanged)

public:
    explicit TransformToolController(QObject *parent = nullptr);
    ~TransformToolController() override;

    QTransform currentTransform() const { return m_transform; }

    Q_INVOKABLE void applyMove(qreal dx, qreal dy);
    Q_INVOKABLE void applyRotate(qreal angleDegrees, const QPointF& center);
    Q_INVOKABLE void applyScale(qreal sx, qreal sy, const QPointF& center);
    Q_INVOKABLE void applyShear(qreal shx, qreal shy);
    Q_INVOKABLE void applyPerspective(const QPolygonF& sourceQuad, const QPolygonF& targetQuad);
    Q_INVOKABLE void applyFlip(bool horizontal, bool vertical, const QPointF& center);

    Q_INVOKABLE void resetTransform();

signals:
    void transformChanged();

private:
    QTransform m_transform;
};
