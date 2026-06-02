#include "core/export/VideoExporter.h"
#include "core/subtitle/SrtParser.h"
#include "ui/timeline_view/TimelineController.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QTimer>
#include <QVariantMap>

#include <algorithm>
#include <iostream>

namespace {

int fail(const QString& message)
{
    std::cerr << message.toStdString() << '\n';
    return 1;
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 3) {
        return fail(QStringLiteral("Usage: ExportProbe <source-media> <output-file> [srt-file]"));
    }

    const QString sourcePath = QDir::toNativeSeparators(QString::fromLocal8Bit(argv[1]));
    const QString outputPath = QDir::toNativeSeparators(QString::fromLocal8Bit(argv[2]));
    const QString srtPath = argc >= 4
        ? QDir::toNativeSeparators(QString::fromLocal8Bit(argv[3]))
        : QString();
    if (!QFileInfo::exists(sourcePath)) {
        return fail(QStringLiteral("Missing source media: %1").arg(sourcePath));
    }
    if (!srtPath.isEmpty() && !QFileInfo::exists(srtPath)) {
        return fail(QStringLiteral("Missing subtitle file: %1").arg(srtPath));
    }

    QFile::remove(outputPath);
    QDir().mkpath(QFileInfo(outputPath).absolutePath());

    constexpr double probeDurationSeconds = 4.0;

    TimelineController timeline;
    const int row = timeline.addMediaAsset(QStringLiteral("ExportProbe"),
                                           sourcePath,
                                           probeDurationSeconds,
                                           true,
                                           true,
                                           0.0,
                                           2);
    if (row < 0 || timeline.clipCount() <= 0) {
        return fail(QStringLiteral("Could not add media to the timeline."));
    }

    if (auto* clipModel = qobject_cast<TimelineClipModel*>(timeline.clipModel())) {
        ClipEffects effects;
        effects.blur.radius = 36.0;
        effects.blur.isRegionEnabled = true;
        effects.blur.regionX = 0.20;
        effects.blur.regionY = 0.20;
        effects.blur.regionWidth = 0.45;
        effects.blur.regionHeight = 0.35;
        clipModel->updateClipEffects(row, effects);
    }

    if (!srtPath.isEmpty()) {
        const QVector<SubtitleEntry> subtitles = SrtParser::parse(srtPath);
        if (subtitles.isEmpty()) {
            return fail(QStringLiteral("Could not parse subtitle file: %1").arg(srtPath));
        }

        int subtitleCount = 0;
        for (const SubtitleEntry& subtitle : subtitles) {
            if (subtitle.startSeconds >= probeDurationSeconds) {
                continue;
            }

            const double endSeconds = std::min(subtitle.endSeconds, probeDurationSeconds);
            const double duration = endSeconds - subtitle.startSeconds;
            if (duration <= 0.01) {
                continue;
            }

            const int subtitleRow = timeline.addSubtitleClip(
                subtitle.text,
                srtPath,
                subtitle.startSeconds,
                duration,
                1
            );
            if (subtitleRow >= 0) {
                ++subtitleCount;
            }
        }
        if (subtitleCount <= 0) {
            return fail(QStringLiteral("Subtitle file had no valid cues: %1").arg(srtPath));
        }
    }

    QVariantMap settings;
    settings.insert(QStringLiteral("exportVideo"), true);
    settings.insert(QStringLiteral("exportAudio"), true);
    settings.insert(QStringLiteral("resolution"), QStringLiteral("360P"));
    settings.insert(QStringLiteral("bitrate"), QStringLiteral("Recommended"));
    settings.insert(QStringLiteral("codec"), QStringLiteral("H.264"));
    settings.insert(QStringLiteral("format"), QStringLiteral("MP4"));
    settings.insert(QStringLiteral("frameRate"), QStringLiteral("24 fps"));
    settings.insert(QStringLiteral("audioFormat"), QStringLiteral("AAC"));

    VideoExporter exporter;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    bool finished = false;
    bool success = false;
    double maxProgress = 0.0;
    QString message;

    QObject::connect(&exporter, &VideoExporter::exportProgress, [&](double progress) {
        maxProgress = std::max(maxProgress, progress);
    });
    QObject::connect(&exporter, &VideoExporter::exportFinished, [&](bool ok, const QString& msg) {
        finished = true;
        success = ok;
        message = msg;
        loop.quit();
    });
    QObject::connect(&timeout, &QTimer::timeout, [&]() {
        exporter.cancelExport();
        message = QStringLiteral("Timed out waiting for export.");
        loop.quit();
    });

    timeout.start(120000);
    exporter.startExportWithSettings(&timeline, outputPath, settings);
    loop.exec();

    if (!finished || !success) {
        return fail(message.isEmpty() ? QStringLiteral("Export did not finish.") : message);
    }

    const QFileInfo exportedFile(outputPath);
    if (!exportedFile.exists() || exportedFile.size() <= 1024) {
        return fail(QStringLiteral("Exported file is missing or too small: %1").arg(outputPath));
    }

    std::cout << "output=" << exportedFile.absoluteFilePath().toStdString() << '\n'
              << "bytes=" << exportedFile.size() << '\n'
              << "progress=" << maxProgress << '\n';
    return 0;
}
