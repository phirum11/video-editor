#pragma once

#include <QImage>
#include <QPointF>
#include <QColor>
#include <QString>
#include <QVariantMap>

class BrushCoreEngine
{
public:
    static bool applyBrushStroke(const QString& brushType, QImage& buffer, const QPointF& start, const QPointF& end, const QVariantMap& options);

private:
    static void applyPaintbrush(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size);
    static void applyAirbrush(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size, qreal rate);
    static void applyPencil(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size);
    static void applyEraser(QImage& buffer, const QPointF& start, const QPointF& end, qreal size);
    static void applyInk(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size, qreal tilt);
    static void applyInkBlob(QImage& buffer, const QPointF& pos, const QColor& color, qreal size);
    static void applyMyPaintBrush(QImage& buffer, const QPointF& start, const QPointF& end, const QVariantMap& options);
    static void applyClone(QImage& buffer, const QPointF& src, const QPointF& dest, qreal size);
    static void applyPerspectiveClone(QImage& buffer, const QPointF& src, const QPointF& dest, qreal size, const QTransform& transform);
};
