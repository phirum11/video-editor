#include "core/export/encoders/IntelEncoder.h"

QString IntelEncoder::encoderName() const {
    return QStringLiteral("Intel Quick Sync");
}

QString IntelEncoder::h264Encoder() const {
    return QStringLiteral("h264_qsv");
}

QString IntelEncoder::hevcEncoder() const {
    return QStringLiteral("hevc_qsv");
}

QStringList IntelEncoder::extraArgs() const {
    return QStringList() << QStringLiteral("-preset") << QStringLiteral("fast");
}
