#include "ui/timeline_view/TimelineController.h"

#include <QCoreApplication>
#include <QVariantMap>

#include <cmath>
#include <iostream>

namespace {
bool nearlyEqual(double a, double b)
{
    return std::abs(a - b) < 0.001;
}

int fail(const char* message)
{
    std::cerr << message << '\n';
    return 1;
}
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    TimelineController timeline;
    const QString filePath = QStringLiteral("C:/tmp/EP001.mp4");

    const int videoRow = timeline.addMediaAsset(QStringLiteral("EP001.mp4"),
                                                filePath,
                                                120.0,
                                                true,
                                                true,
                                                10.0,
                                                2);
    if (videoRow < 0 || timeline.clipCount() != 2) {
        return fail("Expected linked audio and video clips to be added");
    }

    const QVariantMap videoClip = timeline.clipAt(videoRow);
    if (!videoClip.value(QStringLiteral("hasVideo")).toBool()
        || !nearlyEqual(videoClip.value(QStringLiteral("startSeconds")).toDouble(), 10.0)
        || videoClip.value(QStringLiteral("trackIndex")).toInt() != 2) {
        return fail("Video clip metadata was not stored correctly");
    }

    if (!timeline.moveClip(videoRow, 15.0, 1, true)) {
        return fail("Linked move failed");
    }

    for (int row = 0; row < timeline.clipCount(); ++row) {
        const QVariantMap clip = timeline.clipAt(row);
        if (!nearlyEqual(clip.value(QStringLiteral("startSeconds")).toDouble(), 15.0)) {
            return fail("Linked move did not keep audio/video in sync");
        }
    }

    const int splitRow = timeline.splitClip(videoRow, 60.0, true);
    if (splitRow < 0 || timeline.clipCount() != 4) {
        return fail("Linked split did not create matching right-side clips");
    }

    if (!nearlyEqual(timeline.endTimeSeconds(), 135.0)) {
        return fail("Timeline end time is incorrect after split");
    }

    if (!timeline.deleteLinkedClip(videoRow) || timeline.clipCount() != 2) {
        return fail("Linked delete did not remove the selected pair");
    }

    const int removedCount = timeline.removeClipsByFilePath(filePath);
    if (removedCount != 2 || timeline.clipCount() != 0) {
        return fail("Removing a media item did not clear its timeline clips");
    }

    const int audioRow = timeline.addMediaAsset(QStringLiteral("audio.wav"),
                                                QStringLiteral("C:/tmp/audio.wav"),
                                                8.0,
                                                false,
                                                true,
                                                2.0,
                                                5);
    const QVariantMap audioClip = timeline.clipAt(audioRow);
    if (audioRow < 0
        || audioClip.value(QStringLiteral("hasVideo")).toBool()
        || audioClip.value(QStringLiteral("trackIndex")).toInt() != 5) {
        return fail("Audio-only media did not honor the requested audio track");
    }

    std::cout << "clipCount=" << timeline.clipCount() << '\n'
              << "endTime=" << timeline.endTimeSeconds() << '\n';
    return 0;
}
