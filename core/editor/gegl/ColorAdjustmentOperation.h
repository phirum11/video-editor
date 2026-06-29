#pragma once

#include <QImage>
#include <QVariantMap>
#include <QString>

class ColorAdjustmentOperation
{
public:
    static bool apply(const QString& name, QImage& image, const QVariantMap& params);

private:
    static void applyBrightnessContrast(QImage& image, qreal brightness, qreal contrast);
    static void applyCurves(QImage& image, const QVariantMap& params);
    static void applyLevels(QImage& image, const QVariantMap& params);
    static void applyColorBalance(QImage& image, const QVariantMap& params);
    static void applyHueSaturation(QImage& image, qreal hue, qreal saturation, qreal lightness);
    static void applyColorize(QImage& image, const QColor& color);
    static void applyDesaturate(QImage& image);
    static void applyEqualize(QImage& image);
    static void applyPosterize(QImage& image, int levels);
    static void applyInvert(QImage& image);
    static void applyThreshold(QImage& image, int threshold);
    static void applyThresholdAlpha(QImage& image, int threshold);
};
