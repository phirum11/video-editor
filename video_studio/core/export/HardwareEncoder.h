#ifndef HARDWAREENCODER_H
#define HARDWAREENCODER_H

#include <QString>
#include <QStringList>

class HardwareEncoder {
public:
    virtual ~HardwareEncoder() = default;
    
    // The name of the encoder (e.g. "Intel Quick Sync")
    virtual QString encoderName() const = 0;
    
    // FFmpeg encoder name for H.264
    virtual QString h264Encoder() const = 0;
    
    // FFmpeg encoder name for HEVC/H.265
    virtual QString hevcEncoder() const = 0;
    
    // Any extra arguments required for this hardware encoder
    virtual QStringList extraArgs() const = 0;
};

#endif // HARDWAREENCODER_H
