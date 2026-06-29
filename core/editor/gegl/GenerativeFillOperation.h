#pragma once

#include <QImage>
#include <QVariantMap>
#include <QString>

class GenerativeFillOperation
{
public:
    static bool apply(const QString& name, QImage& image, const QVariantMap& params);

private:
    static void applyFloodFill(QImage& image, const QPoint& startPos, const QColor& fillColor, int tolerance);
    static void applyGradientFill(QImage& image, const QPoint& start, const QPoint& end, const QColor& c1, const QColor& c2);
    static void applyFillSource(QImage& image, const QColor& color);
    static void applySetAlpha(QImage& image, qreal alpha);
    static void applyBorder(QImage& image, int borderSize, const QColor& borderColor);
    static void applyGrow(QImage& image, int amount);
    static void applyShrink(QImage& image, int amount);
};
