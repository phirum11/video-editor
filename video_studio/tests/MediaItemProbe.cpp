#include "core/media/models/MediaItem.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QStringList>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList arguments = app.arguments();
    if (arguments.size() < 2) {
        std::cerr << "Usage: MediaItemProbe <media-file> [loops] [thumbnail-width] [thumbnail-height]\n";
        return 2;
    }

    const QString filePath = arguments.at(1);
    const int loops = arguments.size() >= 3
        ? std::max(1, arguments.at(2).toInt())
        : 5;
    const QSize requestedSize(arguments.size() >= 5 ? std::max(1, arguments.at(3).toInt()) : 0,
                              arguments.size() >= 5 ? std::max(1, arguments.at(4).toInt()) : 0);

    if (!QFileInfo::exists(filePath)) {
        std::cerr << "Media file does not exist: " << filePath.toStdString() << "\n";
        return 2;
    }

    try {
        MediaItem media(filePath);
        std::cout << "file=" << media.fileName().toStdString() << "\n"
                  << "duration=" << std::fixed << std::setprecision(3) << media.durationSeconds() << "s\n"
                  << "resolution=" << media.width() << "x" << media.height() << "\n"
                  << "fps=" << std::setprecision(3) << media.frameRate() << "\n"
                  << "hasVideo=" << media.hasVideo() << "\n"
                  << "hasAudio=" << media.hasAudio() << "\n"
                  << "loops=" << loops << "\n"
                  << "requestedSize=" << requestedSize.width() << "x" << requestedSize.height() << "\n";

        if (!media.hasVideo()) {
            std::cerr << "No video stream to probe.\n";
            return 3;
        }

        const double duration = std::max(0.0, media.durationSeconds());
        const std::vector<double> timestamps = {
            0.0,
            duration > 0.0 ? duration * 0.10 : 0.0,
            duration > 0.0 ? duration * 0.35 : 0.0,
            duration > 0.0 ? duration * 0.65 : 0.0,
            duration > 0.0 ? std::max(0.0, duration - 1.0) : 0.0
        };

        qint64 totalMs = 0;
        qint64 worstMs = 0;
        int frameCount = 0;

        for (int loop = 1; loop <= loops; ++loop) {
            for (double timestamp : timestamps) {
                QElapsedTimer timer;
                timer.start();
                const QImage image = media.extractFrame(timestamp, requestedSize);
                const qint64 elapsedMs = timer.elapsed();

                if (image.isNull()) {
                    std::cerr << "Failed to extract frame at " << timestamp
                              << "s during loop " << loop << "\n";
                    return 4;
                }

                totalMs += elapsedMs;
                worstMs = std::max(worstMs, elapsedMs);
                ++frameCount;

                std::cout << "loop=" << loop
                          << " timestamp=" << std::setprecision(3) << timestamp
                          << "s frame=" << image.width() << "x" << image.height()
                          << " extractMs=" << elapsedMs << "\n";
            }
        }

        const double averageMs = frameCount > 0
            ? static_cast<double>(totalMs) / frameCount
            : 0.0;

        std::cout << "frames=" << frameCount << "\n"
                  << "averageExtractMs=" << std::fixed << std::setprecision(2) << averageMs << "\n"
                  << "worstExtractMs=" << worstMs << "\n";
    } catch (const std::exception& error) {
        std::cerr << "Probe failed: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
