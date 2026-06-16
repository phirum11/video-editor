#include "TimelineController.h"

#include <QDebug>

#include <algorithm>
#include <cmath>

#include "core/timeline/commands/TimelineCommands.h"
#include "core/actions/managers/ActionManager.h"

TimelineController::TimelineController(QObject *parent)
    : QObject(parent),
      m_clipModel(new TimelineClipModel(this)),
      m_undoStack(new QUndoStack(this))
{
    connect(m_clipModel, &QAbstractItemModel::rowsInserted, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::rowsRemoved, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::dataChanged, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::modelReset, this, &TimelineController::timelineChanged);

    connect(&ActionManager::instance(), &ActionManager::actionTriggered, this, [this](const QString& id) {
        if (id == "edit.undo") {
            m_undoStack->undo();
        } else if (id == "edit.redo") {
            m_undoStack->redo();
        }
    });

    m_vocalIsolator = new VocalIsolator(this);
    connect(m_vocalIsolator, &VocalIsolator::isolationFinished, this, [this](int clipIndex, int isolationType, const QString& resultFilePath, bool success) {
        if (!success) {
            qWarning() << "Vocal isolation failed for clip" << clipIndex;
            m_clipModel->updateClipIsolationProgress(clipIndex, -1);
            return;
        }
        
        TimelineClip clip = m_clipModel->clipAt(clipIndex);
        QString origPath = clip.originalFilePath.isEmpty() ? clip.filePath : clip.originalFilePath;
        m_clipModel->updateClipAudioSource(clipIndex, resultFilePath, origPath, isolationType);
        m_clipModel->updateClipIsolationProgress(clipIndex, -1);
        
        m_activeIsolationCount = std::max(0, m_activeIsolationCount - 1);
        if (m_activeIsolationCount == 0) {
            m_activeIsolationProgress = 0;
            emit activeIsolationProgressChanged();
            emit isIsolatingChanged();
        }
    });

    connect(m_vocalIsolator, &VocalIsolator::progressChanged, this, [this](int clipIndex, int progress) {
        m_clipModel->updateClipIsolationProgress(clipIndex, progress);
        if (m_activeIsolationProgress != progress) {
            m_activeIsolationProgress = progress;
            emit activeIsolationProgressChanged();
        }
    });
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

    m_undoStack->push(new AddClipCommand(m_clipModel, clip));
    // After pushing, the clip is added at the end
    const int row = m_clipModel->rowCount() - 1;
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

        m_undoStack->push(new AddClipCommand(m_clipModel, videoClip));
        const int selectedRow = m_clipModel->rowCount() - 1;

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

    m_undoStack->push(new AddClipCommand(m_clipModel, clip));
    const int selectedRow = m_clipModel->rowCount() - 1;
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

    m_undoStack->push(new AddClipCommand(m_clipModel, clip));
    return m_clipModel->rowCount() - 1;
}

bool TimelineController::deleteClip(int row)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return false;
    m_undoStack->push(new RemoveClipCommand(m_clipModel, row, RemoveClipCommand::SingleRow));

    if (m_clipModel->rowCount() == 0) {
        setSelectedClipIndex(-1);
    } else {
        setSelectedClipIndex(std::min(row, m_clipModel->rowCount() - 1));
    }

    return true;
}

bool TimelineController::deleteLinkedClip(int row)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return false;
    QString linkGroup = m_clipModel->linkGroupAt(row);
    
    if (linkGroup.isEmpty()) {
        return deleteClip(row);
    }

    m_undoStack->push(new RemoveClipCommand(m_clipModel, row, RemoveClipCommand::LinkedGroup, linkGroup));
    setSelectedClipIndex(-1);
    return true;
}

bool TimelineController::deleteSelectedClip()
{
    return deleteClip(m_selectedClipIndex);
}

int TimelineController::removeClipsByFilePath(const QString& filePath)
{
    int countBefore = m_clipModel->rowCount();
    m_undoStack->push(new RemoveClipCommand(m_clipModel, -1, RemoveClipCommand::FilePath, filePath));
    int removedCount = countBefore - m_clipModel->rowCount();
    if (removedCount > 0) {
        setSelectedClipIndex(-1);
    }
    return removedCount;
}

bool TimelineController::moveClip(int row, double startSeconds, int trackIndex, bool linked)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return false;
    m_undoStack->push(new MoveClipCommand(m_clipModel, row, startSeconds, trackIndex, linked));
    setSelectedClipIndex(row);
    return true;
}

int TimelineController::splitClip(int row, double splitSeconds, bool linked)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return -1;
    const QString linkGroup = m_clipModel->linkGroupAt(row);
    
    m_undoStack->beginMacro("Split Clip");
    
    const QString rightLinkGroup = QStringLiteral("clip_%1").arg(m_nextLinkGroupId++);
    int newSelectedRow = -1;

    if (linked && !linkGroup.isEmpty()) {
        for (int i = m_clipModel->rowCount() - 1; i >= 0; --i) {
            if (m_clipModel->linkGroupAt(i) != linkGroup) {
                continue;
            }
            // Check if split point is valid
            double start = m_clipModel->clipAt(i).startSeconds;
            double end = start + m_clipModel->clipAt(i).durationSeconds;
            if (splitSeconds > start + 0.05 && splitSeconds < end - 0.05) {
                m_undoStack->push(new SplitClipCommand(m_clipModel, i, splitSeconds, rightLinkGroup));
                newSelectedRow = m_clipModel->rowCount() - 1;
            }
        }
    } else {
        double start = m_clipModel->clipAt(row).startSeconds;
        double end = start + m_clipModel->clipAt(row).durationSeconds;
        if (splitSeconds > start + 0.05 && splitSeconds < end - 0.05) {
            m_undoStack->push(new SplitClipCommand(m_clipModel, row, splitSeconds, rightLinkGroup));
            newSelectedRow = m_clipModel->rowCount() - 1;
        }
    }

    m_undoStack->endMacro();

    if (newSelectedRow >= 0) {
        setSelectedClipIndex(newSelectedRow);
    }
    return newSelectedRow;
}

void TimelineController::clearClips()
{
    m_undoStack->push(new RemoveClipCommand(m_clipModel, -1, RemoveClipCommand::All));
    setSelectedClipIndex(-1);
}

void TimelineController::setVocalIsolation(int clipIndex, int isolationType)
{
    if (clipIndex < 0 || clipIndex >= m_clipModel->rowCount()) return;
    
    TimelineClip clip = m_clipModel->clipAt(clipIndex);
    
    if (isolationType == 0) {
        int oldProgress = clip.isolationProgress;
        m_vocalIsolator->cancelIsolation(clipIndex);
        m_clipModel->updateClipIsolationProgress(clipIndex, -1);
        
        if (oldProgress >= 0) {
            m_activeIsolationCount = std::max(0, m_activeIsolationCount - 1);
            if (m_activeIsolationCount == 0) {
                m_activeIsolationProgress = 0;
                emit activeIsolationProgressChanged();
                emit isIsolatingChanged();
            }
        }
        
        // Revert to original
        if (!clip.originalFilePath.isEmpty()) {
            m_clipModel->updateClipAudioSource(clipIndex, clip.originalFilePath, QString(), 0);
        }
        return;
    }

    if (!clip.hasAudio) {
        qWarning() << "Cannot isolate vocals from a clip without audio:" << clipIndex;
        m_clipModel->updateClipIsolationProgress(clipIndex, -1);
        return;
    }
    
    int oldProgress = clip.isolationProgress;
    m_clipModel->updateClipIsolationProgress(clipIndex, 0);
    
    QString sourceFile = clip.originalFilePath.isEmpty() ? clip.filePath : clip.originalFilePath;
    m_vocalIsolator->requestIsolation(clipIndex, sourceFile, isolationType);
    
    if (oldProgress < 0) {
        m_activeIsolationCount++;
        if (m_activeIsolationCount == 1) {
            m_activeIsolationProgress = 0;
            emit activeIsolationProgressChanged();
            emit isIsolatingChanged();
        }
    }
}

void TimelineController::pushTrimCommand(int row, double newStart, double newDuration, double newInPoint, bool linked)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return;
    m_undoStack->push(new TrimClipCommand(m_clipModel, row, newStart, newDuration, newInPoint, linked));
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
