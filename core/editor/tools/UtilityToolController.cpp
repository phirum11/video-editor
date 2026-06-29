#include "UtilityToolController.h"
#include <QImage>
#include <QtMath>
#include <QDebug>

UtilityToolController::UtilityToolController(QObject *parent)
    : QObject(parent)
{
}

UtilityToolController::~UtilityToolController() = default;

QColor UtilityToolController::pickColorAt(const QPointF& pos, const QImage& buffer) const
{
    int x = static_cast<int>(pos.x());
    int y = static_cast<int>(pos.y());
    if (!buffer.isNull() && x >= 0 && x < buffer.width() && y >= 0 && y < buffer.height()) {
        return buffer.pixelColor(x, y);
    }
    return QColor(Qt::transparent);
}

qreal UtilityToolController::measureDistance(const QPointF& p1, const QPointF& p2) const
{
    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    return qSqrt(dx * dx + dy * dy);
}

qreal UtilityToolController::measureAngle(const QPointF& p1, const QPointF& p2) const
{
    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    return qRadiansToDegrees(qAtan2(dy, dx));
}

QVariantMap UtilityToolController::addGuide(const QString& orientation, qreal position)
{
    qDebug() << "UtilityToolController addGuide:" << orientation << position;
    emit guideAdded(orientation, position);
    QVariantMap res; res["orientation"] = orientation; res["position"] = position;
    return res;
}

QVariantMap UtilityToolController::addSamplePoint(const QPointF& pos)
{
    qDebug() << "UtilityToolController addSamplePoint:" << pos;
    emit samplePointAdded(pos);
    QVariantMap res; res["x"] = pos.x(); res["y"] = pos.y();
    return res;
}
