#ifndef AMDENCODER_H
#define AMDENCODER_H

#include "HardwareEncoder.h"

class AmdEncoder : public HardwareEncoder {
public:
    QString encoderName() const override;
    QString h264Encoder() const override;
    QString hevcEncoder() const override;
    QStringList extraArgs() const override;
};

#endif // AMDENCODER_H
