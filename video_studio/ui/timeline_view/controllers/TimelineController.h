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

    /**
     * @brief Adds a basic clip to the timeline.
     * @return The row index of the newly added clip.
     */
    Q_INVOKABLE int addClip(const QString& clipName,
                            const QString& filePath,
                            double durationSeconds,
                            bool hasVideo,
                            double startSeconds,
                            int trackIndex);

    /**
     * @brief Adds a rich media asset (video with linked audio) to the timeline.
     * Automatically splits the asset across video and audio tracks.
     */
    Q_INVOKABLE int addMediaAsset(const QString& clipName,
                                  const QString& filePath,
                                  double durationSeconds,
                                  bool hasVideo,
                                  bool hasAudio,
                                  double startSeconds,
                                  int videoTrackIndex);

    /**
     * @brief Adds a subtitle block to the dedicated subtitle track.
     */
    Q_INVOKABLE int addSubtitleClip(const QString& text,
                                    const QString& srtFilePath,
                                    double startSeconds,
                                    double durationSeconds,
                                    int trackIndex);

    /**
     * @brief Deletes a single clip at the specified row.
     */
    Q_INVOKABLE bool deleteClip(int row);

    /**
     * @brief Deletes a clip and any other clips linked to it (e.g., its extracted audio).
     */
    Q_INVOKABLE bool deleteLinkedClip(int row);

    /**
     * @brief Deletes the currently selected clip.
     */
    Q_INVOKABLE bool deleteSelectedClip();

    /**
     * @brief Removes all clips that reference a specific media file path.
     */
    Q_INVOKABLE int removeClipsByFilePath(const QString& filePath);

    /**
     * @brief Moves a clip to a new start time and track. 
     * If linked is true, moves all linked audio/video clips synchronously.
     */
    Q_INVOKABLE bool moveClip(int row, double startSeconds, int trackIndex, bool linked);

    /**
     * @brief Splits a clip into two separate clips at the specified split time.
     */
    Q_INVOKABLE int splitClip(int row, double splitSeconds, bool linked);

    /**
     * @brief Applies AI vocal isolation to a clip to separate voice from background noise.
     */
    Q_INVOKABLE void setVocalIsolation(int clipIndex, int isolationType);

    /**
     * @brief Clears all clips from the timeline.
     */
    Q_INVOKABLE void clearClips();
    
    /**
     * @brief Registers an undoable trim command for a clip's boundaries.
     */
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
