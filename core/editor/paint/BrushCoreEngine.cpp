#include "BrushCoreEngine.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

bool BrushCoreEngine::applyBrushStroke(const QString& brushType, QImage& buffer, const QPointF& start, const QPointF& end, const QVariantMap& options)
{
    if (buffer.isNull()) return false;

    QColor color = options.value("color", QColor(Qt::black)).value<QColor>();
    qreal size = options.value("size", 10.0).toDouble();

    if (brushType == "Paintbrush") {
        applyPaintbrush(buffer, start, end, color, size);
        return true;
    } else if (brushType == "Airbrush") {
        applyAirbrush(buffer, start, end, color, size, options.value("rate", 0.5).toDouble());
        return true;
    } else if (brushType == "Pencil") {
        applyPencil(buffer, start, end, color, size);
        return true;
    } else if (brushType == "Eraser") {
        applyEraser(buffer, start, end, size);
        return true;
    } else if (brushType == "Ink") {
        applyInk(buffer, start, end, color, size, options.value("tilt", 0.0).toDouble());
        return true;
    } else if (brushType == "InkBlob") {
        applyInkBlob(buffer, end, color, size);
        return true;
    } else if (brushType == "MyPaint") {
        applyMyPaintBrush(buffer, start, end, options);
        return true;
    } else if (brushType == "Clone") {
        applyClone(buffer, options.value("src", QPointF(0,0)).toPointF(), end, size);
        return true;
    } else if (brushType == "PerspectiveClone") {
        applyPerspectiveClone(buffer, options.value("src", QPointF(0,0)).toPointF(), end, size, QTransform());
        return true;
    }

    return false;
}

void BrushCoreEngine::applyPaintbrush(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size)
{
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawLine(start, end);
    painter.end();
}

void BrushCoreEngine::applyAirbrush(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size, qreal rate)
{
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor airColor = color;
    airColor.setAlphaF(color.alphaF() * rate * 0.2);
    QPen pen(airColor, size * 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawLine(start, end);
    painter.end();
}

void BrushCoreEngine::applyPencil(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size)
{
    QPainter painter(&buffer);
    // Anti-aliasing disabled intentionally for aliased pixelated pencil stroke
    QPen pen(color, size, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    painter.setPen(pen);
    painter.drawLine(start, end);
    painter.end();
}

void BrushCoreEngine::applyEraser(QImage& buffer, const QPointF& start, const QPointF& end, qreal size)
{
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen pen(Qt::transparent, size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawLine(start, end);
    painter.end();
}

void BrushCoreEngine::applyInk(QImage& buffer, const QPointF& start, const QPointF& end, const QColor& color, qreal size, qreal tilt)
{
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, size, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    painter.setPen(pen);
    painter.drawLine(start, end);
    painter.end();
}

void BrushCoreEngine::applyInkBlob(QImage& buffer, const QPointF& pos, const QColor& color, qreal size)
{
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(pos, size / 2.0, size / 2.0);
    painter.end();
}

void BrushCoreEngine::applyMyPaintBrush(QImage& buffer, const QPointF& start, const QPointF& end, const QVariantMap& options)
{
    applyPaintbrush(buffer, start, end, options.value("color", QColor(Qt::black)).value<QColor>(), options.value("size", 10.0).toDouble());
}

void BrushCoreEngine::applyClone(QImage& buffer, const QPointF& src, const QPointF& dest, qreal size)
{
    Q_UNUSED(buffer); Q_UNUSED(src); Q_UNUSED(dest); Q_UNUSED(size);
    // Placeholder for pixel buffer source sampling and cloning
}

void BrushCoreEngine::applyPerspectiveClone(QImage& buffer, const QPointF& src, const QPointF& dest, qreal size, const QTransform& transform)
{
    Q_UNUSED(buffer); Q_UNUSED(src); Q_UNUSED(dest); Q_UNUSED(size); Q_UNUSED(transform);
    // Placeholder for homography matrix transformed pixel sampling
}
