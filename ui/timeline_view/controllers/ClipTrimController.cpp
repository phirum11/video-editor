#include "ClipTrimController.h"

#include <algorithm>

ClipTrimController::ClipTrimController(QObject *parent)
    : QObject(parent)
{
}

bool ClipTrimController::isValid(TimelineController* timeline, int row) const
{
    if (!timeline || !timeline->clipModel()) {
        return false;
    }
    return row >= 0 && row < timeline->clipModel()->rowCount();
}

bool ClipTrimController::trimClipLeft(TimelineController* timeline, int row, double deltaSeconds)
{
    if (!isValid(timeline, row)) return false;

    auto* model = static_cast<TimelineClipModel*>(timeline->clipModel());
    const QVariantMap clip = timeline->clipAt(row);
    if (clip.isEmpty()) return false;

    const double currentStart = clip.value("startSeconds").toDouble();
    const double currentDuration = clip.value("durationSeconds").toDouble();
    const double currentInPoint = clip.value("sourceInPoint").toDouble();
    const double sourceDuration = clip.value("sourceDuration").toDouble();
    const QString linkGroup = clip.value("linkGroupId").toString();

    double newStart = currentStart + deltaSeconds;
    double newDuration = currentDuration - deltaSeconds;
    double newInPoint = currentInPoint + deltaSeconds;

    // Constraints
    if (newDuration < 0.1) {
        double diff = 0.1 - newDuration;
        newDuration = 0.1;
        newStart -= diff;
        newInPoint -= diff;
    }
    if (newInPoint < 0.0) {
        double diff = -newInPoint;
        newInPoint = 0.0;
        newStart += diff;
        newDuration -= diff;
    }
    if (newStart < 0.0) {
        double diff = -newStart;
        newStart = 0.0;
        newInPoint += diff;
        newDuration -= diff;
    }
    if (newInPoint + newDuration > sourceDuration && sourceDuration > 0) {
        // Technically this shouldn't happen if right edge isn't moving, but just in case
        newDuration = sourceDuration - newInPoint;
    }

    timeline->pushTrimCommand(row, newStart, newDuration, newInPoint, !linkGroup.isEmpty());
    return true;
}

bool ClipTrimController::trimClipRight(TimelineController* timeline, int row, double deltaSeconds)
{
    if (!isValid(timeline, row)) return false;

    auto* model = static_cast<TimelineClipModel*>(timeline->clipModel());
    const QVariantMap clip = timeline->clipAt(row);
    if (clip.isEmpty()) return false;

    const double currentStart = clip.value("startSeconds").toDouble();
    const double currentDuration = clip.value("durationSeconds").toDouble();
    const double currentInPoint = clip.value("sourceInPoint").toDouble();
    const double sourceDuration = clip.value("sourceDuration").toDouble();
    const QString linkGroup = clip.value("linkGroupId").toString();

    double newDuration = currentDuration + deltaSeconds;

    // Constraints
    if (newDuration < 0.1) {
        newDuration = 0.1;
    }
    if (sourceDuration > 0 && currentInPoint + newDuration > sourceDuration) {
        newDuration = sourceDuration - currentInPoint;
    }

    timeline->pushTrimCommand(row, currentStart, newDuration, currentInPoint, !linkGroup.isEmpty());
    return true;
}
