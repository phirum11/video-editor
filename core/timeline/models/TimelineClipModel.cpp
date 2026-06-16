#include "core/timeline/models/TimelineClipModel.h"

#include <algorithm>
#include <cmath>

TimelineClipModel::TimelineClipModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TimelineClipModel::addClip(const TimelineClip& clip)
{
    const int row = m_clips.size();
    beginInsertRows(QModelIndex(), row, row);
    m_clips.push_back(clip);
    endInsertRows();
    return row;
}

void TimelineClipModel::insertClipAt(int row, const TimelineClip& clip)
{
    int clampedRow = std::clamp(row, 0, (int)m_clips.size());
    beginInsertRows(QModelIndex(), clampedRow, clampedRow);
    m_clips.insert(clampedRow, clip);
    endInsertRows();
}

bool TimelineClipModel::removeClip(int row)
{
    if (!isValidRow(row)) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_clips.removeAt(row);
    endRemoveRows();
    return true;
}

int TimelineClipModel::removeLinkedClips(int row)
{
    const QString linkGroup = linkGroupAt(row);
    if (linkGroup.isEmpty()) {
        return removeClip(row) ? 1 : 0;
    }

    int removedCount = 0;
    for (int i = m_clips.size() - 1; i >= 0; --i) {
        if (m_clips[i].linkGroupId != linkGroup) {
            continue;
        }

        beginRemoveRows(QModelIndex(), i, i);
        m_clips.removeAt(i);
        endRemoveRows();
        ++removedCount;
    }
    return removedCount;
}

int TimelineClipModel::removeClipsByFilePath(const QString& filePath)
{
    int removedCount = 0;
    for (int row = m_clips.size() - 1; row >= 0; --row) {
        if (m_clips[row].filePath != filePath) {
            continue;
        }

        beginRemoveRows(QModelIndex(), row, row);
        m_clips.removeAt(row);
        endRemoveRows();
        ++removedCount;
    }

    return removedCount;
}

bool TimelineClipModel::moveClip(int row, double startSeconds, int trackIndex, bool linked)
{
    if (!isValidRow(row)) {
        return false;
    }

    const TimelineClip original = m_clips[row];
    const double nextStart = std::max(0.0, startSeconds);
    const double startDelta = nextStart - original.startSeconds;
    const QString linkGroup = original.linkGroupId;

    if (linked && !linkGroup.isEmpty()) {
        for (int i = 0; i < m_clips.size(); ++i) {
            if (m_clips[i].linkGroupId != linkGroup) {
                continue;
            }

            m_clips[i].startSeconds = std::max(0.0, m_clips[i].startSeconds + startDelta);
            if (i == row && trackIndex >= 0) {
                m_clips[i].trackIndex = std::clamp(trackIndex, 0, 5);
            }
            updateRow(i);
        }
        return true;
    }

    m_clips[row].startSeconds = nextStart;
    if (trackIndex >= 0) {
        m_clips[row].trackIndex = std::clamp(trackIndex, 0, 5);
    }
    updateRow(row);
    return true;
}

bool TimelineClipModel::trimClip(int row, double newStartSeconds, double newDurationSeconds, double newInPoint, bool linked)
{
    if (!isValidRow(row)) {
        return false;
    }

    const TimelineClip original = m_clips[row];
    const QString linkGroup = original.linkGroupId;
    const double startDelta = newStartSeconds - original.startSeconds;
    const double durationDelta = newDurationSeconds - original.durationSeconds;
    const double inPointDelta = newInPoint - original.sourceInPoint;

    if (linked && !linkGroup.isEmpty()) {
        for (int i = 0; i < m_clips.size(); ++i) {
            if (m_clips[i].linkGroupId != linkGroup) {
                continue;
            }

            m_clips[i].startSeconds = std::max(0.0, m_clips[i].startSeconds + startDelta);
            m_clips[i].durationSeconds = std::max(0.1, m_clips[i].durationSeconds + durationDelta);
            m_clips[i].sourceInPoint = std::max(0.0, m_clips[i].sourceInPoint + inPointDelta);
            updateRow(i);
        }
        return true;
    }

    m_clips[row].startSeconds = std::max(0.0, newStartSeconds);
    m_clips[row].durationSeconds = std::max(0.1, newDurationSeconds);
    m_clips[row].sourceInPoint = std::max(0.0, newInPoint);
    updateRow(row);
    return true;
}

int TimelineClipModel::splitClip(int row, double splitSeconds, const QString& rightLinkGroupId)
{
    if (!isValidRow(row)) {
        return -1;
    }

    TimelineClip& clip = m_clips[row];
    const double clipEnd = clip.startSeconds + clip.durationSeconds;
    if (splitSeconds <= clip.startSeconds + 0.05 || splitSeconds >= clipEnd - 0.05) {
        return -1;
    }

    const double leftDuration = splitSeconds - clip.startSeconds;

    TimelineClip rightClip = clip;
    if (!rightLinkGroupId.isEmpty()) {
        rightClip.linkGroupId = rightLinkGroupId;
    }
    rightClip.startSeconds = splitSeconds;
    rightClip.durationSeconds = clipEnd - splitSeconds;
    rightClip.sourceInPoint = clip.sourceInPoint + leftDuration;
    clip.durationSeconds = leftDuration;
    updateRow(row);

    return addClip(rightClip);
}

void TimelineClipModel::clear()
{
    if (m_clips.isEmpty()) {
        return;
    }

    beginResetModel();
    m_clips.clear();
    endResetModel();
}

ClipEffects TimelineClipModel::clipEffectsAt(int row) const
{
    if (isValidRow(row)) {
        return m_clips[row].effects;
    }
    return ClipEffects();
}

void TimelineClipModel::updateClipEffects(int row, const ClipEffects& effects)
{
    if (isValidRow(row)) {
        m_clips[row].effects = effects;
        // Depending on whether we want this to trigger a view update in QML via data(), 
        // we might emit dataChanged here. For now, since effects aren't in the model roles,
        // we might just update it silently.
    }
}

double TimelineClipModel::endTimeSeconds() const
{
    double endTime = 0.0;
    for (const TimelineClip& clip : m_clips) {
        endTime = std::max(endTime, clip.startSeconds + clip.durationSeconds);
    }
    return endTime;
}

void TimelineClipModel::updateClipAudioSource(int row, const QString& newPath, const QString& originalPath, int isolationType)
{
    if (!isValidRow(row)) return;
    
    m_clips[row].filePath = newPath;
    m_clips[row].originalFilePath = originalPath;
    m_clips[row].vocalIsolationType = isolationType;
    
    updateRow(row);
}

void TimelineClipModel::updateClipIsolationProgress(int row, int progress)
{
    if (!isValidRow(row)) return;
    
    m_clips[row].isolationProgress = progress;
    
    updateRow(row);
}

QVariantMap TimelineClipModel::clipMapAt(int row) const
{
    if (!isValidRow(row)) {
        return {};
    }

    const TimelineClip& clip = m_clips[row];

    QVariantMap map;
    map.insert(QStringLiteral("clipName"), clip.clipName);
    map.insert(QStringLiteral("filePath"), clip.filePath);
    map.insert(QStringLiteral("startSeconds"), clip.startSeconds);
    map.insert(QStringLiteral("durationSeconds"), clip.durationSeconds);
    map.insert(QStringLiteral("trackIndex"), clip.trackIndex);
    map.insert(QStringLiteral("hasVideo"), clip.hasVideo);
    map.insert(QStringLiteral("hasAudio"), clip.hasAudio);
    map.insert(QStringLiteral("linkGroupId"), clip.linkGroupId);
    map.insert(QStringLiteral("sourceInPoint"), clip.sourceInPoint);
    map.insert(QStringLiteral("sourceDuration"), clip.sourceDuration);
    map.insert(QStringLiteral("originalFilePath"), clip.originalFilePath);
    map.insert(QStringLiteral("vocalIsolationType"), clip.vocalIsolationType);
    map.insert(QStringLiteral("isolationProgress"), clip.isolationProgress);

    return map;
}

TimelineClip TimelineClipModel::clipAt(int row) const
{
    return isValidRow(row) ? m_clips[row] : TimelineClip{};
}

QString TimelineClipModel::linkGroupAt(int row) const
{
    return isValidRow(row) ? m_clips[row].linkGroupId : QString{};
}

QVector<int> TimelineClipModel::getLinkedRows(const QString& linkGroupId) const
{
    QVector<int> rows;
    if (linkGroupId.isEmpty()) return rows;
    for (int i = 0; i < m_clips.size(); ++i) {
        if (m_clips[i].linkGroupId == linkGroupId) {
            rows.append(i);
        }
    }
    return rows;
}

bool TimelineClipModel::updateClip(int row, const TimelineClip& clip)
{
    if (!isValidRow(row)) return false;
    m_clips[row] = clip;
    updateRow(row);
    return true;
}

int TimelineClipModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_clips.size();
}

QVariant TimelineClipModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return {};
    }

    const TimelineClip& clip = m_clips[index.row()];
    switch (role) {
    case ClipNameRole:
        return clip.clipName;
    case FilePathRole:
        return clip.filePath;
    case StartSecondsRole:
        return clip.startSeconds;
    case DurationSecondsRole:
        return clip.durationSeconds;
    case TrackIndexRole:
        return clip.trackIndex;
    case HasVideoRole:
        return clip.hasVideo;
    case HasAudioRole: return clip.hasAudio;
    case LinkGroupRole: return clip.linkGroupId;
    case SourceInPointRole: return clip.sourceInPoint;
    case SourceDurationRole: return clip.sourceDuration;
    case OriginalFilePathRole: return clip.originalFilePath;
    case VocalIsolationTypeRole: return clip.vocalIsolationType;
    case IsolationProgressRole: return clip.isolationProgress;
    }

    return {};
}

QHash<int, QByteArray> TimelineClipModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ClipNameRole] = "clipName";
    roles[FilePathRole] = "filePath";
    roles[StartSecondsRole] = "startSeconds";
    roles[DurationSecondsRole] = "durationSeconds";
    roles[TrackIndexRole] = "trackIndex";
    roles[HasVideoRole] = "hasVideo";
    roles[HasAudioRole] = "hasAudio";
    roles[LinkGroupRole] = "linkGroupId";
    roles[SourceInPointRole] = "sourceInPoint";
    roles[SourceDurationRole] = "sourceDuration";
    roles[OriginalFilePathRole] = "originalFilePath";
    roles[VocalIsolationTypeRole] = "vocalIsolationType";
    roles[IsolationProgressRole] = "isolationProgress";
    return roles;
}

bool TimelineClipModel::isValidRow(int row) const
{
    return row >= 0 && row < m_clips.size();
}

void TimelineClipModel::updateRow(int row)
{
    if (!isValidRow(row)) {
        return;
    }

    const QModelIndex modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex);
}
