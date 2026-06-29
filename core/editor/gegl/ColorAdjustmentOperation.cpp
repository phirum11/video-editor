#include "ColorAdjustmentOperation.h"
#include <QColor>
#include <QDebug>
#include <cmath>

bool ColorAdjustmentOperation::apply(const QString& name, QImage& image, const QVariantMap& params)
{
    if (image.isNull()) return false;
    if (image.format() != QImage::Format_ARGB32_Premultiplied && image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    if (name == "gimp:brightness-contrast") {
        applyBrightnessContrast(image, params.value("brightness", 0.0).toDouble(), params.value("contrast", 1.0).toDouble());
        return true;
    } else if (name == "gimp:curves") {
        applyCurves(image, params);
        return true;
    } else if (name == "gimp:levels") {
        applyLevels(image, params);
        return true;
    } else if (name == "gimp:color-balance") {
        applyColorBalance(image, params);
        return true;
    } else if (name == "gimp:hue-saturation") {
        applyHueSaturation(image, params.value("hue", 0.0).toDouble(), params.value("saturation", 0.0).toDouble(), params.value("lightness", 0.0).toDouble());
        return true;
    } else if (name == "gimp:colorize") {
        applyColorize(image, params.value("color", QColor(Qt::blue)).value<QColor>());
        return true;
    } else if (name == "gimp:desaturate") {
        applyDesaturate(image);
        return true;
    } else if (name == "gimp:equalize") {
        applyEqualize(image);
        return true;
    } else if (name == "gimp:posterize") {
        applyPosterize(image, params.value("levels", 4).toInt());
        return true;
    } else if (name == "gimp:invert") {
        applyInvert(image);
        return true;
    } else if (name == "gimp:threshold") {
        applyThreshold(image, params.value("threshold", 128).toInt());
        return true;
    } else if (name == "gimp:threshold-alpha") {
        applyThresholdAlpha(image, params.value("threshold", 128).toInt());
        return true;
    }

    return false;
}

void ColorAdjustmentOperation::applyBrightnessContrast(QImage& image, qreal brightness, qreal contrast)
{
    int width = image.width();
    int height = image.height();
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            int r = qBound(0, static_cast<int>((c.red() - 127) * contrast + 127 + brightness * 255), 255);
            int g = qBound(0, static_cast<int>((c.green() - 127) * contrast + 127 + brightness * 255), 255);
            int b = qBound(0, static_cast<int>((c.blue() - 127) * contrast + 127 + brightness * 255), 255);
            line[x] = qRgba(r, g, b, c.alpha());
        }
    }
}

void ColorAdjustmentOperation::applyCurves(QImage& image, const QVariantMap& params)
{
    Q_UNUSED(params);
    // Placeholder for non-linear curve LUT mapping
}

void ColorAdjustmentOperation::applyLevels(QImage& image, const QVariantMap& params)
{
    Q_UNUSED(params);
    // Placeholder for black/white point level adjustment
}

void ColorAdjustmentOperation::applyColorBalance(QImage& image, const QVariantMap& params)
{
    Q_UNUSED(params);
    // Placeholder for shadow/mid/highlight color balance
}

void ColorAdjustmentOperation::applyHueSaturation(QImage& image, qreal hue, qreal saturation, qreal lightness)
{
    Q_UNUSED(hue); Q_UNUSED(saturation); Q_UNUSED(lightness);
    // Placeholder for HSL manipulation
}

void ColorAdjustmentOperation::applyColorize(QImage& image, const QColor& color)
{
    int width = image.width();
    int height = image.height();
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            int gray = qGray(c.red(), c.green(), c.blue());
            int r = (gray * color.red()) / 255;
            int g = (gray * color.green()) / 255;
            int b = (gray * color.blue()) / 255;
            line[x] = qRgba(r, g, b, c.alpha());
        }
    }
}

void ColorAdjustmentOperation::applyDesaturate(QImage& image)
{
    int width = image.width();
    int height = image.height();
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            int gray = qGray(c.red(), c.green(), c.blue());
            line[x] = qRgba(gray, gray, gray, c.alpha());
        }
    }
}

void ColorAdjustmentOperation::applyEqualize(QImage& image)
{
    Q_UNUSED(image);
    // Placeholder for histogram equalization
}

void ColorAdjustmentOperation::applyPosterize(QImage& image, int levels)
{
    if (levels < 2) levels = 2;
    int width = image.width();
    int height = image.height();
    int step = 255 / (levels - 1);
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            int r = (c.red() / step) * step;
            int g = (c.green() / step) * step;
            int b = (c.blue() / step) * step;
            line[x] = qRgba(r, g, b, c.alpha());
        }
    }
}

void ColorAdjustmentOperation::applyInvert(QImage& image)
{
    image.invertPixels(QImage::InvertRgba);
}

void ColorAdjustmentOperation::applyThreshold(QImage& image, int threshold)
{
    int width = image.width();
    int height = image.height();
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            int gray = qGray(c.red(), c.green(), c.blue());
            int val = (gray >= threshold) ? 255 : 0;
            line[x] = qRgba(val, val, val, c.alpha());
        }
    }
}

void ColorAdjustmentOperation::applyThresholdAlpha(QImage& image, int threshold)
{
    int width = image.width();
    int height = image.height();
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor c(line[x]);
            int a = (c.alpha() >= threshold) ? 255 : 0;
            line[x] = qRgba(c.red(), c.green(), c.blue(), a);
        }
    }
}
