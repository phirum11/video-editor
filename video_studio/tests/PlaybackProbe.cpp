#include "core/playback/engines/PlaybackEngine.h"

#include <QGuiApplication>
#include <QTimer>

#include <iostream>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    const QStringList arguments = app.arguments();
    if (arguments.size() < 2) {
        std::cerr << "Usage: PlaybackProbe <media-file> [milliseconds]\n";
        return 2;
    }

    const QString filePath = arguments.at(1);
    const int milliseconds = arguments.size() >= 3
        ? std::max(500, arguments.at(2).toInt())
        : 3000;

    PlaybackEngine engine;
    if (!engine.loadClip(QStringLiteral("probe"), filePath, 0.0, true)) {
        std::cerr << "loadClip failed\n";
        return 3;
    }

    std::cout << "duration=" << engine.duration() << "\n"
              << "fps=" << engine.frameRate() << "\n"
              << "hasVideo=" << engine.hasVideo() << "\n"
              << "hasAudio=" << engine.hasAudio() << "\n";

    if (!engine.hasVideo()) {
        std::cerr << "No video stream opened\n";
        return 4;
    }

    if (!engine.hasAudio()) {
        std::cerr << "No audio output opened\n";
        return 5;
    }

    double maxLeft = 0.0;
    double maxRight = 0.0;
    QObject::connect(&engine, &PlaybackEngine::audioLevelsChanged, [&]() {
        maxLeft = std::max(maxLeft, engine.audioLevelLeft());
        maxRight = std::max(maxRight, engine.audioLevelRight());
    });

    engine.play();
    QTimer::singleShot(milliseconds, &app, &QCoreApplication::quit);
    app.exec();
    engine.pause();

    std::cout << "position=" << engine.position() << "\n"
              << "maxLeft=" << maxLeft << "\n"
              << "maxRight=" << maxRight << "\n"
              << "playing=" << engine.isPlaying() << "\n";

    if (engine.position() < static_cast<double>(milliseconds) / 1000.0 * 0.55) {
        std::cerr << "Playback did not advance enough\n";
        return 6;
    }

    if (maxLeft <= 0.001 && maxRight <= 0.001) {
        std::cerr << "Audio levels did not move\n";
        return 7;
    }

    return 0;
}
