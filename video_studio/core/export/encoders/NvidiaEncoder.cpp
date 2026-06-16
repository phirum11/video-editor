#include "core/export/encoders/NvidiaEncoder.h"

QString NvidiaEncoder::encoderName() const {
    return QStringLiteral("NVIDIA NVENC");
}

QString NvidiaEncoder::h264Encoder() const {
    return QStringLiteral("h264_nvenc");
}

QString NvidiaEncoder::hevcEncoder() const {
    return QStringLiteral("hevc_nvenc");
}

QStringList NvidiaEncoder::extraArgs() const {
    return QStringList() << QStringLiteral("-preset") << QStringLiteral("p4") 
                         << QStringLiteral("-spatial-aq") << QStringLiteral("1");
}
