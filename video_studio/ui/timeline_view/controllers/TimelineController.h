#ifndef TIMELINECONTROLLER_H
#define TIMELINECONTROLLER_H

#include "core/timeline/models/TimelineClipModel.h"
#include "core/vocal/VocalIsolator.h"

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QUndoStack>
#include <QtQml/qqmlregistration.h>

class TimelineController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(int playheadPosition READ playheadPosition WRITE setPlayheadPosition NOTIFY playheadPositionChanged)
    Q_PROPERTY(QAbstractListModel* clipModel READ clipModel CONSTANT)
    Q_PROPERTY(int selectedClipIndex READ selectedClipIndex WRITE setSelectedClipIndex NOTIFY selectedClipIndexChanged)
    Q_PROPERTY(int clipCount READ clipCount NOTIFY timelineChanged)
    Q_PROPERTY(double timelineEndSeconds READ endTimeSeconds NOTIFY timelineChanged)
    Q_PROPERTY(bool isIsolating READ isIsolating NOTIFY isIsolatingChanged)
    Q_PROPERTY(int activeIsolationProgress READ activeIsolationProgress NOTIFY activeIsolationProgressChanged)

public:
    explicit TimelineController(QObject *parent = nullptr);

    int playheadPosition() const;
    void setPlayheadPosition(int frameIndex);
    QAbstractListModel* clipModel() const;
    int selectedClipIndex() const;
    void setSelectedClipIndex(int index);
    int clipCount() const;
    bool isIsolating() const { return m_activeIsolationCount > 0; }
    int activeIsolationProgress() const { return m_activeIsolationProgress; }

    Q_INVOKABLE int addClip(const QString& clipName,
                            const QString& filePath,
                            double durationSeconds,
                            bool hasVideo,
                            double startSeconds,
                            int trackIndex);
    Q_INVOKABLE int addMediaAsset(const QString& clipName,
                                  const QString& filePath,
                                  double durationSeconds,
                                  bool hasVideo,
                                  bool hasAudio,
                                  double startSeconds,
                                  int videoTrackIndex);
    Q_INVOKABLE int addSubtitleClip(const QString& text,
                                    const QString& srtFilePath,
                                    double startSeconds,
                                    double durationSeconds,
                                    int trackIndex);
    Q_INVOKABLE bool deleteClip(int row);
    Q_INVOKABLE bool deleteLinkedClip(int row);
    Q_INVOKABLE bool deleteSelectedClip();
    Q_INVOKABLE int removeClipsByFilePath(const QString& filePath);
    Q_INVOKABLE bool moveClip(int row, double startSeconds, int trackIndex, bool linked);
    Q_INVOKABLE int splitClip(int row, double splitSeconds, bool linked);
    Q_INVOKABLE void setVocalIsolation(int clipIndex, int isolationType);
    Q_INVOKABLE void clearClips();
    
    Q_INVOKABLE void pushTrimCommand(int row, double newStart, double newDuration, double newInPoint, bool linked);

    Q_INVOKABLE QVariantMap clipAt(int row) const;
    Q_INVOKABLE QString clipFilePath(int row) const;
    Q_INVOKABLE double clipStartSeconds(int row) const;
    Q_INVOKABLE double clipDurationSeconds(int row) const;
    Q_INVOKABLE double clipEndSeconds(int row) const;
    Q_INVOKABLE bool clipContains(int row, double seconds) const;
    Q_INVOKABLE double endTimeSeconds() const;
signals:
    void playheadPositionChanged();
    void selectedClipIndexChanged();
    void timelineChanged();
    void isIsolatingChanged();
    void activeIsolationProgressChanged();
private:
    int m_playheadPosition = 0;
    TimelineClipModel* m_clipModel = nullptr;
    QUndoStack* m_undoStack = nullptr;
    int m_selectedClipIndex = -1;
    int m_nextLinkGroupId = 1;
    VocalIsolator* m_vocalIsolator = nullptr;
    int m_activeIsolationCount = 0;
    int m_activeIsolationProgress = 0;
};

#endif // TIMELINECONTROLLER_H
