#pragma once

#include <QImage>
#include <QByteArray>
#include <QRect>
#include "core/effects/models/EffectData.h"

class EffectProcessor
{
public:
    static QImage processImage(const QImage& source, const ClipEffects& effects);
    static void processAudio(QByteArray& pcmFloat, const ClipEffects& effects);

    /// Alpha-blend a GIF overlay frame on top of the base image.
    /// The gifFrame is scaled to match the base dimensions.
    static QImage compositeGifOverlay(const QImage& base, const QImage& gifFrame);

private:
    static QImage applyBoxBlur(const QImage& source, const QRect& rect, int radius);
    static void boxBlurPass(const QImage& input, QImage& output, int radius, bool horizontal);
};
