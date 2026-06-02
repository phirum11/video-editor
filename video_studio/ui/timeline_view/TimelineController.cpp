#include "TimelineController.h"

#include <QDebug>

#include <algorithm>
#include <cmath>

TimelineController::TimelineController(QObject *parent)
    : QObject(parent),
      m_clipModel(new TimelineClipModel(this))
{
    connect(m_clipModel, &QAbstractItemModel::rowsInserted, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::rowsRemoved, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::dataChanged, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::modelReset, this, &TimelineController::timelineChanged);
}

int TimelineController::playheadPosition() const  
{
    return m_playheadPosition;
}

void TimelineController::setPlayheadPosition(int frameIndex)
{
    if (m_playheadPosition == frameIndex) {
        return;
    }

    m_playheadPosition = frameIndex;
    emit playheadPositionChanged();
}

QAbstractListModel* TimelineController::clipModel() const
{
    return m_clipModel;
}

int TimelineController::selectedClipIndex() const
{
    return m_selectedClipIndex;
}

void TimelineController::setSelectedClipIndex(int index)
{
    const int normalizedIndex = index >= 0 && index < m_clipModel->rowCount() ? index : -1;
    if (m_selectedClipIndex == normalizedIndex) {
        return;
    }

    m_selectedClipIndex = normalizedIndex;
    emit selectedClipIndexChanged();
}

int TimelineController::clipCount() const
{
    return m_clipModel->rowCount();
}

int TimelineController::addClip(const QString& clipName,
                                const QString& filePath,
                                double durationSeconds,
                                bool hasVideo,
                                double startSeconds,
                                int trackIndex)
{
    TimelineClip clip;
    clip.clipName = clipName;
    clip.filePath = filePath;
    clip.linkGroupId = QStringLiteral("clip_%1").arg(m_nextLinkGroupId++);
    clip.durationSeconds = std::isfinite(durationSeconds) && durationSeconds > 0.0
        ? durationSeconds
        : 5.0;
    clip.startSeconds = std::isfinite(startSeconds) && startSeconds >= 0.0
        ? startSeconds
        : m_clipModel->endTimeSeconds();
    clip.trackIndex = trackIndex >= 0 ? trackIndex : (hasVideo ? 2 : 3);
    clip.trackIndex = std::clamp(clip.trackIndex, 0, 5);
    clip.hasVideo = hasVideo;
    clip.sourceInPoint = 0.0;
    clip.sourceDuration = clip.durationSeconds;

    const int row = m_clipModel->addClip(clip);
    setSelectedClipIndex(row);
    return row;
}

int TimelineController::addMediaAsset(const QString& clipName,
                                      const QString& filePath,
                                      double durationSeconds,
                                      bool hasVideo,
                                      bool hasAudio,
                                      double startSeconds,
                                      int videoTrackIndex)
{
    const QString linkGroup = QStringLiteral("clip_%1").arg(m_nextLinkGroupId++);
    const double safeDuration = std::isfinite(durationSeconds) && durationSeconds > 0.0
        ? durationSeconds
        : 5.0;
    const double safeStart = std::isfinite(startSeconds) && startSeconds >= 0.0
        ? startSeconds
        : m_clipModel->endTimeSeconds();

    if (hasVideo) {
        TimelineClip videoClip;
        videoClip.clipName = clipName;
        videoClip.filePath = filePath;
        videoClip.linkGroupId = linkGroup;
        videoClip.startSeconds = safeStart;
        videoClip.durationSeconds = safeDuration;
        videoClip.trackIndex = std::clamp(videoTrackIndex >= 0 ? videoTrackIndex : 2, 0, 2);
        videoClip.hasVideo = true;
        videoClip.hasAudio = hasAudio;
        videoClip.sourceInPoint = 0.0;
        videoClip.sourceDuration = safeDuration;

        const int selectedRow = m_clipModel->addClip(videoClip);

        setSelectedClipIndex(selectedRow);
        return selectedRow;
    }

    if (!hasAudio) {
        return -1;
    }

    TimelineClip clip;
    clip.clipName = clipName;
    clip.filePath = filePath;
    clip.linkGroupId = linkGroup;
    clip.startSeconds = safeStart;
    clip.durationSeconds = safeDuration;
    clip.trackIndex = videoTrackIndex >= 3
        ? std::clamp(videoTrackIndex, 3, 5)
        : 3;
    clip.hasVideo = false;
    clip.hasAudio = true;
    clip.sourceInPoint = 0.0;
    clip.sourceDuration = safeDuration;

    const int selectedRow = m_clipModel->addClip(clip);
    setSelectedClipIndex(selectedRow);
    return selectedRow;
}

int TimelineController::addSubtitleClip(const QString& text,
                                         const QString& srtFilePath,
                                         double startSeconds,
                                         double durationSeconds,
                                         int trackIndex)
{
    TimelineClip clip;
    clip.clipName = text;
    clip.filePath = srtFilePath;
    clip.linkGroupId = QStringLiteral("sub_%1").arg(m_nextLinkGroupId++);
    clip.startSeconds = std::max(0.0, startSeconds);
    clip.durationSeconds = std::max(0.1, durationSeconds);
    clip.trackIndex = std::clamp(trackIndex, 0, 5);
    clip.hasVideo = false;
    clip.hasAudio = false;
    clip.sourceInPoint = 0.0;
    clip.sourceDuration = clip.durationSeconds;

    return m_clipModel->addClip(clip);
}

bool TimelineController::deleteClip(int row)
{
    const bool removed = m_clipModel->removeClip(row);
    if (!removed) {
        return false;
    }

    if (m_clipModel->rowCount() == 0) {
        setSelectedClipIndex(-1);
    } else {
        setSelectedClipIndex(std::min(row, m_clipModel->rowCount() - 1));
    }

    return true;
}

bool TimelineController::deleteLinkedClip(int row)
{
    const int removedCount = m_clipModel->removeLinkedClips(row);
    if (removedCount <= 0) {
        return false;
    }

    setSelectedClipIndex(-1);
    return true;
}

bool TimelineController::deleteSelectedClip()
{
    return deleteClip(m_selectedClipIndex);
}

int TimelineController::removeClipsByFilePath(const QString& filePath)
{
    const int removedCount = m_clipModel->removeClipsByFilePath(filePath);
    if (removedCount > 0) {
        setSelectedClipIndex(-1);
    }
    return removedCount;
}

bool TimelineController::moveClip(int row, double startSeconds, int trackIndex, bool linked)
{
    const bool moved = m_clipModel->moveClip(row, startSeconds, trackIndex, linked);
    if (moved) {
        setSelectedClipIndex(row);
    }
    return moved;
}

int TimelineController::splitClip(int row, double splitSeconds, bool linked)
{
    const QString linkGroup = m_clipModel->linkGroupAt(row);
    const QString rightLinkGroup = QStringLiteral("clip_%1").arg(m_nextLinkGroupId++);
    int newSelectedRow = -1;

    if (linked && !linkGroup.isEmpty()) {
        for (int i = m_clipModel->rowCount() - 1; i >= 0; --i) {
            if (m_clipModel->linkGroupAt(i) != linkGroup) {
                continue;
            }
            const int splitRow = m_clipModel->splitClip(i, splitSeconds, rightLinkGroup);
            if (splitRow >= 0) {
                newSelectedRow = splitRow;
            }
        }
    } else {
        newSelectedRow = m_clipModel->splitClip(row, splitSeconds, rightLinkGroup);
    }

    if (newSelectedRow >= 0) {
        setSelectedClipIndex(newSelectedRow);
    }
    return newSelectedRow;
}

void TimelineController::clearClips()
{
    m_clipModel->clear();
    setSelectedClipIndex(-1);
}

QVariantMap TimelineController::clipAt(int row) const
{
    return m_clipModel->clipMapAt(row);
}

QString TimelineController::clipFilePath(int row) const
{
    return m_clipModel->clipMapAt(row).value(QStringLiteral("filePath")).toString();
}

double TimelineController::clipStartSeconds(int row) const
{
    return m_clipModel->clipMapAt(row).value(QStringLiteral("startSeconds"), -1.0).toDouble();
}

double TimelineController::clipDurationSeconds(int row) const
{
    return m_clipModel->clipMapAt(row).value(QStringLiteral("durationSeconds"), 0.0).toDouble();
}

double TimelineController::clipEndSeconds(int row) const
{
    const double start = clipStartSeconds(row);
    const double duration = clipDurationSeconds(row);
    if (!std::isfinite(start) || start < 0.0 || !std::isfinite(duration) || duration <= 0.0) {
        return -1.0;
    }
    return start + duration;
}

bool TimelineController::clipContains(int row, double seconds) const
{
    const double start = clipStartSeconds(row);
    const double end = clipEndSeconds(row);
    return std::isfinite(seconds) && start >= 0.0 && end >= start
        && seconds >= start && seconds <= end;
}

double TimelineController::endTimeSeconds() const
{
    return m_clipModel->endTimeSeconds();
}
