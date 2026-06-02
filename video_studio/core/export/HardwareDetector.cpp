#include "HardwareDetector.h"
#include "IntelEncoder.h"
#include "NvidiaEncoder.h"
#include "AmdEncoder.h"
#include <QProcess>
#include <QDebug>

std::shared_ptr<HardwareEncoder> HardwareDetector::detectBestEncoder(const QString& ffmpegPath) {
    if (ffmpegPath.isEmpty()) {
        return nullptr;
    }

    QProcess process;
    process.start(ffmpegPath, QStringList() << QStringLiteral("-encoders"));
    if (!process.waitForFinished(5000)) {
        return nullptr;
    }

    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    
    // We prioritize Intel QSV as requested (since user has Intel GPU), then NVENC, then AMF.
    // However, an actual robust system might check which GPU is currently active.
    // Since ffmpeg lists supported encoders, we just check if they are built-in.
    // Note: Just because ffmpeg has the encoder compiled in, doesn't mean the local hardware supports it.
    // A true detection would do a test encode (ffmpeg -f lavfi -i nullsrc ...).
    // For this implementation, checking ffmpeg output is usually sufficient since static builds 
    // include them all, but we will assume if the user has Iris Xe, QSV is best.
    
    // A more rigorous test:
    auto testEncoder = [&](const QString& encoderName) -> bool {
        QProcess testProc;
        // Run a very fast 1-frame test to see if hardware encoding succeeds
        testProc.start(ffmpegPath, QStringList() 
            << QStringLiteral("-f") << QStringLiteral("lavfi") 
            << QStringLiteral("-i") << QStringLiteral("color=c=black:s=128x128:r=1:d=1")
            << QStringLiteral("-c:v") << encoderName
            << QStringLiteral("-f") << QStringLiteral("null") << QStringLiteral("-"));
        testProc.waitForFinished(3000);
        return testProc.exitCode() == 0;
    };

    if (output.contains(QStringLiteral("h264_qsv"))) {
        if (testEncoder(QStringLiteral("h264_qsv"))) {
            qDebug() << "Detected Intel Quick Sync Video hardware acceleration.";
            return std::make_shared<IntelEncoder>();
        }
    }

    if (output.contains(QStringLiteral("h264_nvenc"))) {
        if (testEncoder(QStringLiteral("h264_nvenc"))) {
            qDebug() << "Detected NVIDIA NVENC hardware acceleration.";
            return std::make_shared<NvidiaEncoder>();
        }
    }

    if (output.contains(QStringLiteral("h264_amf"))) {
        if (testEncoder(QStringLiteral("h264_amf"))) {
            qDebug() << "Detected AMD AMF hardware acceleration.";
            return std::make_shared<AmdEncoder>();
        }
    }

    qDebug() << "No suitable hardware acceleration detected. Falling back to software encoding.";
    return nullptr;
}
