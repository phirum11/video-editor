#include "core/effects/processors/EffectProcessor.h"
#include <QPainter>
#include <algorithm>
#include <cmath>

#include <QColor>
#include <QVector3D>

namespace {

int clampChannel(double value)
{
    return std::clamp(static_cast<int>(std::lround(value)), 0, 255);
}

QImage applyColorAdjust(const QImage& source, const ColorEffectData& color)
{
    if (std::abs(color.brightness) < 0.0001
        && std::abs(color.contrast - 100.0) < 0.0001
        && std::abs(color.saturation - 100.0) < 0.0001) {
        return source;
    }

    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    const double brightness = std::clamp(color.brightness, -100.0, 100.0) * 2.55;
    const double contrast = std::max(0.0, color.contrast) / 100.0;
    const double saturation = std::max(0.0, color.saturation) / 100.0;

    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            const QRgb pixel = line[x];
            const double a = qAlpha(pixel);
            double r = qRed(pixel);
            double g = qGreen(pixel);
            double b = qBlue(pixel);
            const double gray = r * 0.299 + g * 0.587 + b * 0.114;

            r = gray + (r - gray) * saturation;
            g = gray + (g - gray) * saturation;
            b = gray + (b - gray) * saturation;

            r = (r - 128.0) * contrast + 128.0 + brightness;
            g = (g - 128.0) * contrast + 128.0 + brightness;
            b = (b - 128.0) * contrast + 128.0 + brightness;

            line[x] = qRgba(clampChannel(r), clampChannel(g), clampChannel(b), clampChannel(a));
        }
    }

    return result.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

QImage applyMultiShotGrid(const QImage& source)
{
    if (source.isNull()) {
        return source;
    }

    QImage result(source.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::black);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int columns = 2;
    const int rows = 2;
    const int cellWidth = std::max(1, result.width() / columns);
    const int cellHeight = std::max(1, result.height() / rows);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const QRect target(column * cellWidth,
                               row * cellHeight,
                               column == columns - 1 ? result.width() - column * cellWidth : cellWidth,
                               row == rows - 1 ? result.height() - row * cellHeight : cellHeight);
            painter.save();
            painter.setClipRect(target);
            if ((row + column) % 2 == 1) {
                painter.translate(target.center());
                painter.scale(-1.0, 1.0);
                painter.translate(-target.center());
            }
            painter.drawImage(target, source);
            painter.restore();
        }
    }

    painter.setPen(QPen(QColor(255, 255, 255, 55), std::max(1, result.width() / 360)));
    painter.drawLine(cellWidth, 0, cellWidth, result.height());
    painter.drawLine(0, cellHeight, result.width(), cellHeight);
    return result;
}

QImage applyGlitchStyle(const QImage& source, double intensity)
{
    if (source.isNull()) {
        return source;
    }

    const int maxShift = std::max(1, static_cast<int>(std::lround(std::clamp(intensity, 0.0, 100.0) / 100.0 * 18.0)));
    QImage input = source.convertToFormat(QImage::Format_ARGB32);
    QImage result(input.size(), QImage::Format_ARGB32);

    for (int y = 0; y < input.height(); ++y) {
        const QRgb* src = reinterpret_cast<const QRgb*>(input.constScanLine(y));
        QRgb* dst = reinterpret_cast<QRgb*>(result.scanLine(y));
        const int redShift = static_cast<int>(std::lround(std::sin(y * 0.13) * maxShift));
        const int blueShift = static_cast<int>(std::lround(std::cos(y * 0.09) * maxShift));
        for (int x = 0; x < input.width(); ++x) {
            const QRgb center = src[x];
            const QRgb redPixel = src[std::clamp(x + redShift, 0, input.width() - 1)];
            const QRgb bluePixel = src[std::clamp(x + blueShift, 0, input.width() - 1)];
            dst[x] = qRgba(qRed(redPixel), qGreen(center), qBlue(bluePixel), qAlpha(center));
        }
    }

    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(QRect(0, 0, result.width(), std::max(1, result.height() / 80)), QColor(255, 255, 255, 36));
    painter.fillRect(QRect(0, result.height() / 2, result.width(), std::max(1, result.height() / 70)), QColor(0, 255, 255, 28));
    painter.end();

    return result.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

QImage applyCarbonStyle(const QImage& source, double intensity)
{
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    const double strength = std::clamp(intensity, 0.0, 100.0) / 100.0;

    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            const QRgb pixel = line[x];
            const double gray = qRed(pixel) * 0.299 + qGreen(pixel) * 0.587 + qBlue(pixel) * 0.114;
            const int carbon = clampChannel(gray * (0.8 + strength * 0.25));
            const int cool = clampChannel(carbon + 12 * strength);
            line[x] = qRgba(carbon, carbon, cool, qAlpha(pixel));
        }
    }

    return result.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

QImage applyGenericStylize(const QImage& source, const StylizeEffectData& stylize)
{
    if (stylize.styleName.compare(QStringLiteral("None"), Qt::CaseInsensitive) == 0
        || stylize.intensity <= 0.0) {
        return source;
    }

    if (stylize.styleName.compare(QStringLiteral("Multi-Shot Grid"), Qt::CaseInsensitive) == 0) {
        return applyMultiShotGrid(source);
    }
    if (stylize.styleName.contains(QStringLiteral("Glitch"), Qt::CaseInsensitive)) {
        return applyGlitchStyle(source, stylize.intensity);
    }
    if (stylize.styleName.contains(QStringLiteral("Carbon"), Qt::CaseInsensitive)) {
        return applyCarbonStyle(source, stylize.intensity);
    }

    QImage result = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SoftLight);
    const uint hash = qHash(stylize.styleName);
    const QColor tint(static_cast<int>(80 + hash % 120),
                      static_cast<int>(80 + (hash >> 8) % 120),
                      static_cast<int>(80 + (hash >> 16) % 120),
                      static_cast<int>(std::clamp(stylize.intensity, 0.0, 100.0) * 1.2));
    painter.fillRect(result.rect(), tint);
    painter.end();
    return result;
}

} // namespace

QImage EffectProcessor::processImage(const QImage& source, const ClipEffects& effects)
{
    if (source.isNull()) return source;
    
    QImage result = source;

    // 0. Chroma Key
    if (effects.chromaKey.enabled) {
        QImage chromaImage = result.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        QColor keyColor(effects.chromaKey.color);
        int kr = keyColor.red();
        int kg = keyColor.green();
        int kb = keyColor.blue();
        
        // Convert variance (0.0 - 1.0) to squared distance threshold (max dist squared is 3*255^2 = 195075)
        double maxDistSq = 195075.0;
        double threshold = effects.chromaKey.variance * effects.chromaKey.variance * maxDistSq;
        double softThreshold = (effects.chromaKey.variance + effects.chromaKey.softness) * (effects.chromaKey.variance + effects.chromaKey.softness) * maxDistSq;
        
        int width = chromaImage.width();
        int height = chromaImage.height();
        
        for (int y = 0; y < height; ++y) {
            QRgb* scanLine = reinterpret_cast<QRgb*>(chromaImage.scanLine(y));
            for (int x = 0; x < width; ++x) {
                QRgb pixel = scanLine[x];
                int r = qRed(pixel);
                int g = qGreen(pixel);
                int b = qBlue(pixel);
                int a = qAlpha(pixel);
                
                double distSq = (r - kr)*(r - kr) + (g - kg)*(g - kg) + (b - kb)*(b - kb);
                
                if (distSq < threshold) {
                    a = 0;
                } else if (distSq < softThreshold && softThreshold > threshold) {
                    double alphaFactor = (distSq - threshold) / (softThreshold - threshold);
                    a = static_cast<int>(a * alphaFactor);
                    
                    if (effects.chromaKey.spillSuppress) {
                        // Simple spill suppress: reduce key color component
                        if (kg > kr && kg > kb) { // Green screen
                            g = std::min(g, std::max(r, b));
                        } else if (kb > kr && kb > kg) { // Blue screen
                            b = std::min(b, std::max(r, g));
                        }
                    }
                }
                
                scanLine[x] = qRgba(r * a / 255, g * a / 255, b * a / 255, a);
            }
        }
        result = chromaImage;
    }

    // 1. Color and stylize presets
    result = applyColorAdjust(result, effects.color);
    result = applyGenericStylize(result, effects.stylize);

    // 2. Blur
    if (effects.blur.radius > 0) {
        int radius = std::clamp(static_cast<int>(std::lround(effects.blur.radius)), 1, 100);
        if (radius > 0) {
            QRect region;
            if (effects.blur.isRegionEnabled) {
                const double rx = std::clamp(std::isfinite(effects.blur.regionX) ? effects.blur.regionX : 0.0, 0.0, 1.0);
                const double ry = std::clamp(std::isfinite(effects.blur.regionY) ? effects.blur.regionY : 0.0, 0.0, 1.0);
                const double rw = std::clamp(std::isfinite(effects.blur.regionWidth) ? effects.blur.regionWidth : 0.0, 0.0, 1.0 - rx);
                const double rh = std::clamp(std::isfinite(effects.blur.regionHeight) ? effects.blur.regionHeight : 0.0, 0.0, 1.0 - ry);

                const int x1 = std::clamp(static_cast<int>(std::floor(rx * source.width())), 0, source.width());
                const int y1 = std::clamp(static_cast<int>(std::floor(ry * source.height())), 0, source.height());
                const int x2 = std::clamp(static_cast<int>(std::ceil((rx + rw) * source.width())), x1, source.width());
                const int y2 = std::clamp(static_cast<int>(std::ceil((ry + rh) * source.height())), y1, source.height());
                region = QRect(x1, y1, x2 - x1, y2 - y1).intersected(source.rect());
            } else {
                region = source.rect();
            }
            
            if (!region.isEmpty()) {
                result = applyBoxBlur(result, region, radius);
            }
        }
    }

    // 3. Transform (Scale, Rotation, Position, Opacity)
    // We only apply this if it deviates from defaults, to save CPU.
    if (effects.transform.scale != 100.0 || effects.transform.rotation != 0.0 || 
        effects.transform.posX != 960.0 || effects.transform.posY != 540.0 || 
        effects.transform.opacity != 100.0) 
    {
        QImage transformed(source.size(), QImage::Format_ARGB32_Premultiplied);
        transformed.fill(Qt::transparent);
        
        QPainter painter(&transformed);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.setRenderHint(QPainter::Antialiasing, true);
        
        // Opacity
        painter.setOpacity(effects.transform.opacity / 100.0);
        
        // Translate to position
        // Map 1920x1080 abstract coordinates to the actual frame size
        double scaleX = source.width() / 1920.0;
        double scaleY = source.height() / 1080.0;
        
        painter.translate(effects.transform.posX * scaleX, effects.transform.posY * scaleY);
        painter.rotate(effects.transform.rotation);
        
        // Scale
        double s = effects.transform.scale / 100.0;
        painter.scale(s, s);
        
        // Translate back from anchor
        painter.translate(-effects.transform.anchorX * scaleX, -effects.transform.anchorY * scaleY);
        
        painter.drawImage(0, 0, result);
        painter.end();
        result = transformed;
    }

    return result;
}

QImage EffectProcessor::applyBoxBlur(const QImage& source, const QRect& rect, int radius)
{
    const QRect targetRect = rect.intersected(source.rect());
    if (targetRect.isEmpty() || radius <= 0) {
        return source;
    }

    const QRect sampleRect = targetRect.adjusted(-radius, -radius, radius, radius).intersected(source.rect());
    if (sampleRect.isEmpty()) {
        return source;
    }

    int scaleFactor = 1;
    if (radius >= 16) scaleFactor = 4;
    else if (radius >= 6) scaleFactor = 2;

    int scaledRadius = std::max(1, radius / scaleFactor);

    QImage work = source.copy(sampleRect);
    if (scaleFactor > 1) {
        work = work.scaled(work.width() / scaleFactor, work.height() / scaleFactor, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    QImage buffer1 = work.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage buffer2(buffer1.size(), QImage::Format_ARGB32_Premultiplied);

    boxBlurPass(buffer1, buffer2, scaledRadius, true);
    boxBlurPass(buffer2, buffer1, scaledRadius, false);
    boxBlurPass(buffer1, buffer2, std::max(1, scaledRadius / 2), true);
    boxBlurPass(buffer2, buffer1, std::max(1, scaledRadius / 2), false);

    if (scaleFactor > 1) {
        buffer1 = buffer1.scaled(sampleRect.width(), sampleRect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QImage finalImage = source;
    QPainter p(&finalImage);
    const QPoint sourceOffset = targetRect.topLeft() - sampleRect.topLeft();
    p.drawImage(targetRect.topLeft(),
                buffer1,
                QRect(sourceOffset, targetRect.size()));
    return finalImage;
}

void EffectProcessor::boxBlurPass(const QImage& input, QImage& output, int radius, bool horizontal)
{
    if (input.isNull() || radius <= 0) {
        return;
    }

    const int width = input.width();
    const int height = input.height();

    if (horizontal) {
        for (int y = 0; y < height; ++y) {
            const QRgb* src = reinterpret_cast<const QRgb*>(input.constScanLine(y));
            QRgb* dst = reinterpret_cast<QRgb*>(output.scanLine(y));

            int a = 0;
            int r = 0;
            int g = 0;
            int b = 0;

            for (int i = -radius; i <= radius; ++i) {
                const QRgb pixel = src[std::clamp(i, 0, width - 1)];
                a += qAlpha(pixel);
                r += qRed(pixel);
                g += qGreen(pixel);
                b += qBlue(pixel);
            }

            const int count = radius * 2 + 1;
            for (int x = 0; x < width; ++x) {
                dst[x] = qRgba(r / count, g / count, b / count, a / count);

                const QRgb removePixel = src[std::clamp(x - radius, 0, width - 1)];
                const QRgb addPixel = src[std::clamp(x + radius + 1, 0, width - 1)];
                a += qAlpha(addPixel) - qAlpha(removePixel);
                r += qRed(addPixel) - qRed(removePixel);
                g += qGreen(addPixel) - qGreen(removePixel);
                b += qBlue(addPixel) - qBlue(removePixel);
            }
        }
    } else {
        const QRgb* srcData = reinterpret_cast<const QRgb*>(input.constBits());
        QRgb* dstData = reinterpret_cast<QRgb*>(output.bits());
        const qsizetype stride = input.bytesPerLine() / sizeof(QRgb);

        for (int x = 0; x < width; ++x) {
            int a = 0;
            int r = 0;
            int g = 0;
            int b = 0;

            for (int i = -radius; i <= radius; ++i) {
                const int clampedY = std::clamp(i, 0, height - 1);
                const QRgb pixel = srcData[clampedY * stride + x];
                a += qAlpha(pixel);
                r += qRed(pixel);
                g += qGreen(pixel);
                b += qBlue(pixel);
            }

            const int count = radius * 2 + 1;
            for (int y = 0; y < height; ++y) {
                dstData[y * stride + x] = qRgba(r / count, g / count, b / count, a / count);

                const int removeY = std::clamp(y - radius, 0, height - 1);
                const int addY = std::clamp(y + radius + 1, 0, height - 1);
                const QRgb removePixel = srcData[removeY * stride + x];
                const QRgb addPixel = srcData[addY * stride + x];

                a += qAlpha(addPixel) - qAlpha(removePixel);
                r += qRed(addPixel) - qRed(removePixel);
                g += qGreen(addPixel) - qGreen(removePixel);
                b += qBlue(addPixel) - qBlue(removePixel);
            }
        }
    }
}

void EffectProcessor::processAudio(QByteArray& pcmFloat, const ClipEffects& effects)
{
    if (effects.audio.volume == 100.0) return; // No change
    
    if (pcmFloat.isEmpty()) return;
    
    float volMultiplier = static_cast<float>(effects.audio.volume / 100.0);
    
    auto* samples = reinterpret_cast<float*>(pcmFloat.data());
    const qsizetype sampleCount = pcmFloat.size() / static_cast<qsizetype>(sizeof(float));
    for (qsizetype i = 0; i < sampleCount; ++i) {
        samples[i] = samples[i] * volMultiplier;
    }
}
