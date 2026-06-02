#ifndef INTELENCODER_H
#define INTELENCODER_H

#include "HardwareEncoder.h"

class IntelEncoder : public HardwareEncoder {
public:
    QString encoderName() const override;
    QString h264Encoder() const override;
    QString hevcEncoder() const override;
    QStringList extraArgs() const override;
};

#endif // INTELENCODER_H
