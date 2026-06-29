#include "TransformToolController.h"
#include <QDebug>
#include <QPolygonF>

TransformToolController::TransformToolController(QObject *parent)
    : QObject(parent)
{
}

TransformToolController::~TransformToolController() = default;

void TransformToolController::applyMove(qreal dx, qreal dy)
{
    m_transform.translate(dx, dy);
    emit transformChanged();
}

void TransformToolController::applyRotate(qreal angleDegrees, const QPointF& center)
{
    m_transform.translate(center.x(), center.y());
    m_transform.rotate(angleDegrees);
    m_transform.translate(-center.x(), -center.y());
    emit transformChanged();
}

void TransformToolController::applyScale(qreal sx, qreal sy, const QPointF& center)
{
    m_transform.translate(center.x(), center.y());
    m_transform.scale(sx, sy);
    m_transform.translate(-center.x(), -center.y());
    emit transformChanged();
}

void TransformToolController::applyShear(qreal shx, qreal shy)
{
    m_transform.shear(shx, shy);
    emit transformChanged();
}

void TransformToolController::applyPerspective(const QPolygonF& sourceQuad, const QPolygonF& targetQuad)
{
    Q_UNUSED(sourceQuad); Q_UNUSED(targetQuad);
    // Placeholder for 4-point homography solving matrix calculation
    emit transformChanged();
}

void TransformToolController::applyFlip(bool horizontal, bool vertical, const QPointF& center)
{
    m_transform.translate(center.x(), center.y());
    m_transform.scale(horizontal ? -1.0 : 1.0, vertical ? -1.0 : 1.0);
    m_transform.translate(-center.x(), -center.y());
    emit transformChanged();
}

void TransformToolController::resetTransform()
{
    m_transform.reset();
    emit transformChanged();
}
