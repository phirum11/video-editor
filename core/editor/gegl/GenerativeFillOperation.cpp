#include "GenerativeFillOperation.h"
#include <QPainter>
#include <QLinearGradient>
#include <QDebug>

bool GenerativeFillOperation::apply(const QString& name, QImage& image, const QVariantMap& params)
{
    if (image.isNull()) return false;

    if (name == "gimp:flood-fill") {
        applyFloodFill(image, params.value("pos", QPoint(0,0)).toPoint(), params.value("color", QColor(Qt::black)).value<QColor>(), params.value("tolerance", 15).toInt());
        return true;
    } else if (name == "gimp:gradient-fill") {
        applyGradientFill(image, params.value("start", QPoint(0,0)).toPoint(), params.value("end", QPoint(image.width(), image.height())).toPoint(),
                          params.value("color1", QColor(Qt::black)).value<QColor>(), params.value("color2", QColor(Qt::white)).value<QColor>());
        return true;
    } else if (name == "gimp:fill-source") {
        applyFillSource(image, params.value("color", QColor(Qt::black)).value<QColor>());
        return true;
    } else if (name == "gimp:set-alpha") {
        applySetAlpha(image, params.value("alpha", 1.0).toDouble());
        return true;
    } else if (name == "gimp:border") {
        applyBorder(image, params.value("size", 5).toInt(), params.value("color", QColor(Qt::black)).value<QColor>());
        return true;
    } else if (name == "gimp:grow") {
        applyGrow(image, params.value("amount", 2).toInt());
        return true;
    } else if (name == "gimp:shrink") {
        applyShrink(image, params.value("amount", 2).toInt());
        return true;
    }

    return false;
}

void GenerativeFillOperation::applyFloodFill(QImage& image, const QPoint& startPos, const QColor& fillColor, int tolerance)
{
    Q_UNUSED(tolerance);
    // Simplified flood fill representation
    if (startPos.x() >= 0 && startPos.x() < image.width() && startPos.y() >= 0 && startPos.y() < image.height()) {
        QPainter painter(&image);
        painter.fillRect(image.rect(), fillColor);
        painter.end();
    }
}

void GenerativeFillOperation::applyGradientFill(QImage& image, const QPoint& start, const QPoint& end, const QColor& c1, const QColor& c2)
{
    QPainter painter(&image);
    QLinearGradient grad(start, end);
    grad.setColorAt(0.0, c1);
    grad.setColorAt(1.0, c2);
    painter.fillRect(image.rect(), grad);
    painter.end();
}

void GenerativeFillOperation::applyFillSource(QImage& image, const QColor& color)
{
    image.fill(color);
}

void GenerativeFillOperation::applySetAlpha(QImage& image, qreal alpha)
{
    int width = image.width();
    int height = image.height();
    int alphaVal = qBound(0, static_cast<int>(alpha * 255), 255);

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            line[x] = qRgba(c.red(), c.green(), c.blue(), alphaVal);
        }
    }
}

void GenerativeFillOperation::applyBorder(QImage& image, int borderSize, const QColor& borderColor)
{
    QPainter painter(&image);
    QPen pen(borderColor, borderSize * 2);
    painter.setPen(pen);
    painter.drawRect(image.rect());
    painter.end();
}

void GenerativeFillOperation::applyGrow(QImage& image, int amount)
{
    Q_UNUSED(image); Q_UNUSED(amount);
    // Placeholder for morphological dilation
}

void GenerativeFillOperation::applyShrink(QImage& image, int amount)
{
    Q_UNUSED(image); Q_UNUSED(amount);
    // Placeholder for morphological erosion
}
