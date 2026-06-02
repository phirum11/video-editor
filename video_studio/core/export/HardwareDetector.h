#ifndef HARDWAREDETECTOR_H
#define HARDWAREDETECTOR_H

#include <QString>
#include <memory>
#include "HardwareEncoder.h"

class HardwareDetector {
public:
    // Detects the best available hardware encoder on this system by probing ffmpeg.
    // Falls back to returning nullptr if no hardware encoder is found or supported.
    static std::shared_ptr<HardwareEncoder> detectBestEncoder(const QString& ffmpegPath);
};

#endif // HARDWAREDETECTOR_H
