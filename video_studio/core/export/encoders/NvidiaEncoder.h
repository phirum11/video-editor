#ifndef NVIDIAENCODER_H
#define NVIDIAENCODER_H

#include "core/export/encoders/HardwareEncoder.h"

class NvidiaEncoder : public HardwareEncoder {
public:
    QString encoderName() const override;
    QString h264Encoder() const override;
    QString hevcEncoder() const override;
    QStringList extraArgs() const override;
};

#endif // NVIDIAENCODER_H
