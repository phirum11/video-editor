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

private:
    static QImage applyBoxBlur(const QImage& source, const QRect& rect, int radius);
    static void boxBlurPass(const QImage& input, QImage& output, int radius, bool horizontal);
};
