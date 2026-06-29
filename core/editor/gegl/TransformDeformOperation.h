#pragma once

#include <QImage>
#include <QVariantMap>
#include <QString>

class TransformDeformOperation
{
public:
    static bool apply(const QString& name, QImage& image, const QVariantMap& params);

private:
    static void applyCageTransform(QImage& image, const QVariantMap& params);
    static void applyCageCoefCalc(QImage& image, const QVariantMap& params);
    static void applyProfileTransform(QImage& image, const QVariantMap& params);
    static void applyOffset(QImage& image, int offsetX, int offsetY);
    static void applyComposeCrop(QImage& image, const QRect& cropRect);
};
