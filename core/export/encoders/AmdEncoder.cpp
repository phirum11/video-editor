#include "core/export/encoders/AmdEncoder.h"

QString AmdEncoder::encoderName() const {
    return QStringLiteral("AMD AMF");
}

QString AmdEncoder::h264Encoder() const {
    return QStringLiteral("h264_amf");
}

QString AmdEncoder::hevcEncoder() const {
    return QStringLiteral("hevc_amf");
}

QStringList AmdEncoder::extraArgs() const {
    return QStringList() << QStringLiteral("-quality") << QStringLiteral("balanced");
}
