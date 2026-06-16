#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariantMap>
#include <QVector>
#include "core/effects/models/EffectData.h"

struct TimelineClip
{
    QString clipName;
    QString filePath;
    QString linkGroupId;
    QString originalFilePath;
    int vocalIsolationType = 0;
    int isolationProgress = -1;
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    double sourceInPoint = 0.0;
    double sourceDuration = 0.0;
    int trackIndex = 2;
    bool hasVideo = true;
    bool hasAudio = false;
    ClipEffects effects;
};

class TimelineClipModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ClipRole {
        ClipNameRole = Qt::UserRole + 1,
        FilePathRole,
        StartSecondsRole,
        DurationSecondsRole,
        TrackIndexRole,
        HasVideoRole,
        HasAudioRole,
        LinkGroupRole,
        SourceInPointRole,
        SourceDurationRole,
        OriginalFilePathRole,
        VocalIsolationTypeRole,
        IsolationProgressRole
    };

    explicit TimelineClipModel(QObject* parent = nullptr);

    int addClip(const TimelineClip& clip);
    void insertClipAt(int row, const TimelineClip& clip);
    bool removeClip(int row);
    int removeLinkedClips(int row);
    int removeClipsByFilePath(const QString& filePath);
    bool moveClip(int row, double startSeconds, int trackIndex, bool linked);
    bool trimClip(int row, double newStartSeconds, double newDurationSeconds, double newInPoint, bool linked);
    int splitClip(int row, double splitSeconds, const QString& rightLinkGroupId = {});
    void clear();
    void updateClipAudioSource(int row, const QString& newPath, const QString& originalPath, int isolationType);
    void updateClipIsolationProgress(int row, int progress);

    ClipEffects clipEffectsAt(int row) const;
    void updateClipEffects(int row, const ClipEffects& effects);

    double endTimeSeconds() const;
    QVariantMap clipMapAt(int row) const;
    TimelineClip clipAt(int row) const;
    QString linkGroupAt(int row) const;
    QVector<int> getLinkedRows(const QString& linkGroupId) const;
    bool updateClip(int row, const TimelineClip& clip);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    bool isValidRow(int row) const;
    void updateRow(int row);

    QVector<TimelineClip> m_clips;
};
