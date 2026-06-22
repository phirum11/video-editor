#include "TimelineController.h"

#include <QDebug>

#include <algorithm>
#include <cmath>
#include <QVariantMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QUuid>
#include "core/timeline/commands/TimelineCommands.h"
#include "core/actions/managers/ActionManager.h"
#include "core/vocal/VocalIsolator.h"
#include "core/vocal/AIVoiceGenerator.h"
#include "core/timeline/controllers/TimelineAutoEditor.h"

TimelineController::TimelineController(QObject *parent)
    : QObject(parent),
      m_clipModel(new TimelineClipModel(this)),
      m_undoStack(new QUndoStack(this)),
      m_vocalIsolator(new VocalIsolator(this)),
      m_aiVoiceGenerator(new AIVoiceGenerator(this))
{
    connect(m_clipModel, &QAbstractItemModel::rowsInserted, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::rowsRemoved, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::dataChanged, this, &TimelineController::timelineChanged);
    connect(m_clipModel, &QAbstractItemModel::modelReset, this, &TimelineController::timelineChanged);

    connect(this, &TimelineController::timelineChanged, this, &TimelineController::calculateTrackCounts);

    connect(&ActionManager::instance(), &ActionManager::actionTriggered, this, [this](const QString& id) {
        if (id == "edit.undo") {
            m_undoStack->undo();
        } else if (id == "edit.redo") {
            m_undoStack->redo();
        }
    });

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
        if (m_selectedClipIndices.contains(clipIndex)) {
            m_activeIsolationProgress = progress;
            emit activeIsolationProgressChanged();
        }
    });

    connect(m_aiVoiceGenerator, &AIVoiceGenerator::progressChanged, this, [this](int clipIndex, int progress) {
        m_activeAIVoiceProgress = progress;
        emit activeAIVoiceProgressChanged();
    });

    connect(m_aiVoiceGenerator, &AIVoiceGenerator::segmentGenerated, this, [this](int clipIndex, const QString& jsonStr) {
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString path = obj["path"].toString();
            double startSeconds = obj["start"].toDouble();
            double duration = obj["duration"].toDouble(0.0);
            
            // Generate a placeholder name based on the UI. We don't have an exact index,
            // but we can just call it "Generated Voice".
            addMediaAsset(
                "Generated Voice",
                path,
                duration,
                false,
                true,
                startSeconds,
                3
            );
        }
    });

    connect(m_aiVoiceGenerator, &AIVoiceGenerator::generationFinished, this, [this](int clipIndex, const QString& metadataJsonPath, bool success) {
        m_activeAIVoiceCount--;
        emit isGeneratingAIVoiceChanged();
        m_activeAIVoiceProgress = 0;
        emit activeAIVoiceProgressChanged();

        if (!success || metadataJsonPath.isEmpty()) {
            qWarning() << "AI Voice generation failed for clip" << clipIndex;
            return;
        }

        m_undoStack->beginMacro("Auto-mute original audio");

            
            // Auto-mute original video clips so the AI voice replaces the original sound
            for (int i = 0; i < m_clipModel->rowCount(); ++i) {
                TimelineClip clip = m_clipModel->clipAt(i);
                if (clip.hasVideo && clip.hasAudio && !clip.isMuted) {
                    m_clipModel->setClipMuted(i, true);
                }
            }
            
            m_undoStack->endMacro();
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

QVariantList TimelineController::selectedClipIndices() const
{
    return m_selectedClipIndices;
}

void TimelineController::setSelectedClipIndices(const QVariantList& indices)
{
    QSet<int> uniqueIndices;
    QSet<QString> processedGroups;

    for (const QVariant& v : indices) {
        int row = v.toInt();
        if (row < 0 || row >= m_clipModel->rowCount()) continue;
        
        uniqueIndices.insert(row);
        
        QString groupId = m_clipModel->groupAt(row);
        if (!groupId.isEmpty() && !processedGroups.contains(groupId)) {
            processedGroups.insert(groupId);
            for (int i = 0; i < m_clipModel->rowCount(); ++i) {
                if (m_clipModel->groupAt(i) == groupId) {
                    uniqueIndices.insert(i);
                }
            }
        }
    }
    
    QVariantList finalIndices;
    for (int idx : uniqueIndices) {
        finalIndices.append(idx);
    }
    
    if (m_selectedClipIndices == finalIndices) {
        return;
    }

    m_selectedClipIndices = finalIndices;
    emit selectedClipIndicesChanged();
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
    int targetTrack = trackIndex >= 0 ? trackIndex : (hasVideo ? 2 : 3);
    targetTrack = std::max(0, targetTrack);
    
    if (hasVideo && isTrackLocked(true, targetTrack)) return -1;
    if (!hasVideo && isTrackLocked(false, targetTrack >= 100 ? targetTrack - 100 : targetTrack)) return -1;

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
    clip.trackIndex = targetTrack;
    clip.hasVideo = hasVideo;
    clip.sourceInPoint = 0.0;
    clip.sourceDuration = clip.durationSeconds;

    m_undoStack->push(new AddClipCommand(m_clipModel, clip));
    // After pushing, the clip is added at the end
    const int row = m_clipModel->rowCount() - 1;
    setSelectedClipIndices({row});
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

    int vTrack = std::max(0, videoTrackIndex >= 0 ? videoTrackIndex : 2);
    int aTrack = videoTrackIndex >= 100 ? videoTrackIndex : 100; // Will be mapped to audio 0

    if (hasVideo && isTrackLocked(true, vTrack)) return -1;
    if (hasAudio && !hasVideo && isTrackLocked(false, aTrack - 100)) return -1;
    // Note: If it has both, we'd check both, but we'll let it slide if one is locked or just prevent.
    if (hasVideo && hasAudio && (isTrackLocked(true, vTrack) || isTrackLocked(false, aTrack - 100))) return -1;

    m_undoStack->beginMacro("Add Media Asset");

    int videoRow = -1;
    if (hasVideo) {
        TimelineClip videoClip;
        videoClip.clipName = clipName;
        videoClip.filePath = filePath;
        videoClip.linkGroupId = linkGroup;
        videoClip.startSeconds = safeStart;
        videoClip.durationSeconds = safeDuration;
        videoClip.trackIndex = vTrack;
        videoClip.hasVideo = true;
        videoClip.hasAudio = hasAudio;
        videoClip.sourceInPoint = 0.0;
        videoClip.sourceDuration = safeDuration;

        m_undoStack->push(new AddClipCommand(m_clipModel, videoClip));
        videoRow = m_clipModel->rowCount() - 1;
    }

    int audioRow = -1;
    if (hasAudio && !hasVideo) {
        TimelineClip clip;
        clip.clipName = clipName;
        clip.filePath = filePath;
        clip.linkGroupId = linkGroup;
        clip.startSeconds = safeStart;
        clip.durationSeconds = safeDuration;
        clip.trackIndex = aTrack;
        clip.hasVideo = false;
        clip.hasAudio = true;
        clip.sourceInPoint = 0.0;
        clip.sourceDuration = safeDuration;

        m_undoStack->push(new AddClipCommand(m_clipModel, clip));
        audioRow = m_clipModel->rowCount() - 1;
    }

    m_undoStack->endMacro();

    QVariantList selected;
    if (videoRow >= 0) selected.append(videoRow);
    if (audioRow >= 0) selected.append(audioRow);
    
    setSelectedClipIndices(selected);
    
    return videoRow >= 0 ? videoRow : audioRow;
}

int TimelineController::addSubtitleClip(const QString& text,
                                         const QString& srtFilePath,
                                         double startSeconds,
                                         double durationSeconds,
                                         int trackIndex)
{
    int vTrack = std::clamp(trackIndex, 0, 5);
    if (isTrackLocked(true, vTrack)) return -1;

    TimelineClip clip;
    clip.clipName = text;
    clip.filePath = srtFilePath;
    clip.linkGroupId = QStringLiteral("sub_%1").arg(m_nextLinkGroupId++);
    clip.startSeconds = std::max(0.0, startSeconds);
    clip.durationSeconds = std::max(0.1, durationSeconds);
    clip.trackIndex = vTrack;
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
    
    TimelineClip clip = m_clipModel->clipAt(row);
    bool isAudioTrack = clip.trackIndex >= 100;
    int logicalIndex = isAudioTrack ? clip.trackIndex - 100 : clip.trackIndex;
    if (isTrackLocked(!isAudioTrack, logicalIndex)) return false;

    m_undoStack->push(new RemoveClipCommand(m_clipModel, row, RemoveClipCommand::SingleRow));

    if (m_clipModel->rowCount() == 0) {
        setSelectedClipIndices(QVariantList());
    } else {
        setSelectedClipIndices({std::min(row, m_clipModel->rowCount() - 1)});
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

    // Check if ANY clip in this link group is on a locked track
    for (int i = 0; i < m_clipModel->rowCount(); ++i) {
        if (m_clipModel->linkGroupAt(i) == linkGroup) {
            TimelineClip c = m_clipModel->clipAt(i);
            bool cIsAudio = c.trackIndex >= 100;
            int logicalIndex = cIsAudio ? c.trackIndex - 100 : c.trackIndex;
            if (isTrackLocked(!cIsAudio, logicalIndex)) return false;
        }
    }

    m_undoStack->push(new RemoveClipCommand(m_clipModel, row, RemoveClipCommand::LinkedGroup, linkGroup));
    setSelectedClipIndices(QVariantList());
    return true;
}

bool TimelineController::deleteSelectedClips()
{
    if (m_selectedClipIndices.isEmpty()) return false;

    m_undoStack->beginMacro("Delete Clips");
    
    QList<int> sortedIndices;
    for (const QVariant& v : m_selectedClipIndices) {
        sortedIndices.append(v.toInt());
    }
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());
    
    for (int idx : sortedIndices) {
        deleteClip(idx);
    }
    
    m_undoStack->endMacro();
    setSelectedClipIndices(QVariantList());
    return true;
}

int TimelineController::removeClipsByFilePath(const QString& filePath)
{
    int countBefore = m_clipModel->rowCount();
    m_undoStack->push(new RemoveClipCommand(m_clipModel, -1, RemoveClipCommand::FilePath, filePath));
    int removedCount = countBefore - m_clipModel->rowCount();
    if (removedCount > 0) {
        setSelectedClipIndices(QVariantList());
    }
    return removedCount;
}

bool TimelineController::moveClip(int row, double startSeconds, int trackIndex, bool linked)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return false;
    
    TimelineClip clip = m_clipModel->clipAt(row);
    bool isAudioTrack = clip.trackIndex >= 100;
    int srcLogicalIndex = isAudioTrack ? clip.trackIndex - 100 : clip.trackIndex;
    int destLogicalIndex = isAudioTrack ? trackIndex - 100 : trackIndex;
    
    if (isTrackLocked(!isAudioTrack, srcLogicalIndex)) return false;
    if (isTrackLocked(!isAudioTrack, destLogicalIndex)) return false;

    m_undoStack->push(new MoveClipCommand(m_clipModel, row, startSeconds, trackIndex, linked));
    
    // Keep it selected if not already in selection
    if (!m_selectedClipIndices.contains(row)) {
        setSelectedClipIndices({row});
    }
    return true;
}

bool TimelineController::moveSelectedClips(double deltaSeconds, int deltaTrackIndex, bool linked)
{
    if (m_selectedClipIndices.isEmpty()) return false;

    m_undoStack->beginMacro("Move Clips");
    
    QSet<QString> processedLinkGroups;
    for (const QVariant& v : m_selectedClipIndices) {
        int idx = v.toInt();
        if (idx < 0 || idx >= m_clipModel->rowCount()) continue;
        
        if (linked) {
            QString linkGroup = m_clipModel->linkGroupAt(idx);
            if (!linkGroup.isEmpty()) {
                if (processedLinkGroups.contains(linkGroup)) continue;
                processedLinkGroups.insert(linkGroup);
            }
        }
        
        double currentStart = m_clipModel->clipAt(idx).startSeconds;
        int currentTrack = m_clipModel->clipAt(idx).trackIndex;
        
        moveClip(idx, currentStart + deltaSeconds, currentTrack + deltaTrackIndex, linked);
    }
    m_undoStack->endMacro();
    
    return true;
}

int TimelineController::splitClip(int row, double splitSeconds, bool linked)
{
    if (row < 0 || row >= m_clipModel->rowCount()) return -1;
    const QString linkGroup = m_clipModel->linkGroupAt(row);
    
    TimelineClip srcClip = m_clipModel->clipAt(row);
    bool isAudioTrack = srcClip.trackIndex >= 100;
    int srcLogicalIndex = isAudioTrack ? srcClip.trackIndex - 100 : srcClip.trackIndex;
    if (isTrackLocked(!isAudioTrack, srcLogicalIndex)) return -1;

    m_undoStack->beginMacro("Split Clip");
    
    const QString rightLinkGroup = QStringLiteral("clip_%1").arg(m_nextLinkGroupId++);
    int newSelectedRow = -1;

    if (linked && !linkGroup.isEmpty()) {
        for (int i = m_clipModel->rowCount() - 1; i >= 0; --i) {
            if (m_clipModel->linkGroupAt(i) != linkGroup) {
                continue;
            }
            TimelineClip c = m_clipModel->clipAt(i);
            bool cIsAudio = c.trackIndex >= 100;
            int cLogical = cIsAudio ? c.trackIndex - 100 : c.trackIndex;
            if (isTrackLocked(!cIsAudio, cLogical)) continue;

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
        setSelectedClipIndices({newSelectedRow});
    }
    return newSelectedRow;
}

void TimelineController::clearClips()
{
    m_undoStack->push(new RemoveClipCommand(m_clipModel, -1, RemoveClipCommand::All));
    setSelectedClipIndices(QVariantList());
}

void TimelineController::autoEditSync()
{
    TimelineAutoEditor::syncVisualsToAudio(m_clipModel, m_undoStack);
}

void TimelineController::generateAIVoiceFromSrt(const QString& language)
{
    // Prevent multiple simultaneous generations
    if (m_activeAIVoiceCount > 0) {
        qDebug() << "AI Voice generation already in progress, ignoring request.";
        return;
    }

    int index = -1;

    // First try the selected clip
    if (!m_selectedClipIndices.isEmpty()) {
        int firstSelected = m_selectedClipIndices.first().toInt();
        if (firstSelected >= 0 && firstSelected < m_clipModel->rowCount()) {
            QString path = m_clipModel->clipMapAt(firstSelected)["filePath"].toString();
            if (path.endsWith(".srt", Qt::CaseInsensitive)) {
                index = firstSelected;
            }
        }
    }

    // If not found, scan all clips for the first SRT file
    if (index == -1) {
        for (int i = 0; i < m_clipModel->rowCount(); ++i) {
            QString path = m_clipModel->clipMapAt(i)["filePath"].toString();
            if (path.endsWith(".srt", Qt::CaseInsensitive)) {
                index = i;
                break;
            }
        }
    }

    if (index == -1) {
        qWarning() << "No SRT clip found in the timeline for AI Voice generation.";
        return;
    }

    QVariantMap clipData = m_clipModel->clipMapAt(index);
    QString filePath = clipData["filePath"].toString();

    m_activeAIVoiceCount++;
    emit isGeneratingAIVoiceChanged();

    m_aiVoiceGenerator->requestGeneration(index, filePath, language);
}

void TimelineController::setClipMuted(int clipIndex, bool muted)
{
    if (clipIndex < 0 || clipIndex >= m_clipModel->rowCount()) return;
    m_clipModel->setClipMuted(clipIndex, muted);
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
    TimelineClip clip = m_clipModel->clipAt(row);
    bool isAudioTrack = clip.trackIndex >= 100;
    int logicalIndex = isAudioTrack ? clip.trackIndex - 100 : clip.trackIndex;
    if (isTrackLocked(!isAudioTrack, logicalIndex)) return;

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

QList<double> TimelineController::getSnapPoints() const
{
    QList<double> points;
    points.append(0.0);
    points.append(static_cast<double>(m_playheadPosition) / 1000.0);
    
    for (int i = 0; i < m_clipModel->rowCount(); ++i) {
        double start = clipStartSeconds(i);
        double end = clipEndSeconds(i);
        if (start >= 0.0) points.append(start);
        if (end >= 0.0) points.append(end);
    }
    return points;
}

double TimelineController::endTimeSeconds() const
{
    return m_clipModel->endTimeSeconds();
}
void TimelineController::ungroupSelectedClips()
{
    if (m_selectedClipIndices.isEmpty()) return;

    QVector<int> rows;
    for (const QVariant& v : m_selectedClipIndices) {
        rows.append(v.toInt());
    }

    m_undoStack->push(new GroupClipsCommand(m_clipModel, rows, QString()));
}

void TimelineController::groupSelectedClips()
{
    if (m_selectedClipIndices.size() < 2) return;

    QVector<int> rows;
    for (const QVariant& v : m_selectedClipIndices) {
        rows.append(v.toInt());
    }

    const QString newGroupId = QStringLiteral("group_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_undoStack->push(new GroupClipsCommand(m_clipModel, rows, newGroupId));
}

bool TimelineController::isTrackEmpty(bool isVideo, int trackIndex) const
{
    if (!m_clipModel) return true;
    for (int i = 0; i < m_clipModel->rowCount(); ++i) {
        TimelineClip clip = m_clipModel->clipAt(i);
        if (clip.hasVideo == isVideo && clip.trackIndex == (isVideo ? trackIndex : trackIndex + 100)) {
            return false;
        }
    }
    return true;
}

void TimelineController::calculateTrackCounts()
{
    int maxVideo = -1;
    int maxAudio = -1;
    
    for (int i = 0; i < m_clipModel->rowCount(); ++i) {
        TimelineClip clip = m_clipModel->clipAt(i);
        if (clip.hasVideo) {
            maxVideo = std::max(maxVideo, clip.trackIndex);
        } else {
            maxAudio = std::max(maxAudio, clip.trackIndex - 100);
        }
    }
    
    int newVideoCount = std::max(1, maxVideo + 1);
    int newAudioCount = std::max(0, maxAudio + 1);
    
    bool changed = false;
    if (newVideoCount != m_videoTrackCount) {
        m_videoTrackCount = newVideoCount;
        changed = true;
    }
    if (newAudioCount != m_audioTrackCount) {
        m_audioTrackCount = newAudioCount;
        changed = true;
    }
    
    if (changed) {
        emit trackCountsChanged();
    }
}

void TimelineController::cleanupEmptyTracks()
{
    calculateTrackCounts();
}

void TimelineController::setTrackLocked(bool isVideo, int trackIndex, bool locked)
{
    if (isVideo) {
        if (m_videoTrackLocked.value(trackIndex, false) == locked) return;
        m_videoTrackLocked[trackIndex] = locked;
    } else {
        if (m_audioTrackLocked.value(trackIndex, false) == locked) return;
        m_audioTrackLocked[trackIndex] = locked;
    }
    emit trackStateChanged(isVideo, trackIndex);
}

bool TimelineController::isTrackLocked(bool isVideo, int trackIndex) const
{
    return isVideo ? m_videoTrackLocked.value(trackIndex, false) : m_audioTrackLocked.value(trackIndex, false);
}

void TimelineController::setTrackHidden(bool isVideo, int trackIndex, bool hidden)
{
    if (isVideo) {
        if (m_videoTrackHidden.value(trackIndex, false) == hidden) return;
        m_videoTrackHidden[trackIndex] = hidden;
    } else {
        if (m_audioTrackHidden.value(trackIndex, false) == hidden) return;
        m_audioTrackHidden[trackIndex] = hidden;
    }
    emit trackStateChanged(isVideo, trackIndex);
    emit timelineChanged(); // Force redraw/re-evaluate
}

bool TimelineController::isTrackHidden(bool isVideo, int trackIndex) const
{
    return isVideo ? m_videoTrackHidden.value(trackIndex, false) : m_audioTrackHidden.value(trackIndex, false);
}

void TimelineController::setTrackMuted(bool isVideo, int trackIndex, bool muted)
{
    if (isVideo) {
        if (m_videoTrackMuted.value(trackIndex, false) == muted) return;
        m_videoTrackMuted[trackIndex] = muted;
    } else {
        if (m_audioTrackMuted.value(trackIndex, false) == muted) return;
        m_audioTrackMuted[trackIndex] = muted;
    }
    emit trackStateChanged(isVideo, trackIndex);
    emit timelineChanged(); // Force redraw/re-evaluate
}

bool TimelineController::isTrackMuted(bool isVideo, int trackIndex) const
{
    return isVideo ? m_videoTrackMuted.value(trackIndex, false) : m_audioTrackMuted.value(trackIndex, false);
}

