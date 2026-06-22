#pragma once

#include <QString>

struct TransformEffectData {
    double scale = 100.0;
    double posX = 960.0;
    double posY = 540.0;
    double rotation = 0.0;
    double anchorX = 960.0;
    double anchorY = 540.0;
    double opacity = 100.0;
    QString blendMode = "Normal";
};

struct ColorEffectData {
    // Stubs for future properties
    double brightness = 0.0;
    double contrast = 100.0;
    double saturation = 100.0;
};

struct BlurEffectData {
    // Stubs for future properties
    double radius = 0.0;
    QString blurType = "Gaussian";
    
    // Region Properties
    bool isRegionEnabled = true;
    double regionX = 0.25;      // Normalized coordinates (0.0 to 1.0)
    double regionY = 0.25;
    double regionWidth = 0.5;
    double regionHeight = 0.5;
};

struct StylizeEffectData {
    // Stubs for future properties
    QString styleName = "None";
    double intensity = 100.0;
};

struct AudioEffectData {
    // Stubs for future properties
    double volume = 100.0;
    double pan = 0.0;
};

struct ChromaKeyEffectData {
    bool enabled = false;
    QString color = "#00FF00"; // Default green
    double variance = 0.25;
    double softness = 0.1;
    bool spillSuppress = true;
};

struct ClipEffects {
    TransformEffectData transform;
    ColorEffectData color;
    BlurEffectData blur;
    StylizeEffectData stylize;
    AudioEffectData audio;
    ChromaKeyEffectData chromaKey;
};
