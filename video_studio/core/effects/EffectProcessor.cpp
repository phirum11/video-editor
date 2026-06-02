#include "EffectProcessor.h"
#include <QPainter>
#include <algorithm>
#include <cmath>

QImage EffectProcessor::processImage(const QImage& source, const ClipEffects& effects)
{
    if (source.isNull()) return source;
    
    QImage result = source;

    // 1. Blur
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

    // 2. Transform (Scale, Rotation, Position, Opacity)
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
