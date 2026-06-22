#ifndef TIMELINECONTROLLER_H
#define TIMELINECONTROLLER_H

#include "core/timeline/models/TimelineClipModel.h"
#include "core/vocal/VocalIsolator.h"
#include "core/vocal/AIVoiceGenerator.h"

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
    Q_PROPERTY(QVariantList selectedClipIndices READ selectedClipIndices WRITE setSelectedClipIndices NOTIFY selectedClipIndicesChanged)
    Q_PROPERTY(int clipCount READ clipCount NOTIFY timelineChanged)
    Q_PROPERTY(double timelineEndSeconds READ endTimeSeconds NOTIFY timelineChanged)
    Q_PROPERTY(bool isIsolating READ isIsolating NOTIFY isIsolatingChanged)
    Q_PROPERTY(int activeIsolationProgress READ activeIsolationProgress NOTIFY activeIsolationProgressChanged)
    Q_PROPERTY(bool isGeneratingAIVoice READ isGeneratingAIVoice NOTIFY isGeneratingAIVoiceChanged)
    Q_PROPERTY(int activeAIVoiceProgress READ activeAIVoiceProgress NOTIFY activeAIVoiceProgressChanged)
    
    Q_PROPERTY(int videoTrackCount READ videoTrackCount NOTIFY trackCountsChanged)
    Q_PROPERTY(int audioTrackCount READ audioTrackCount NOTIFY trackCountsChanged)

public:
    explicit TimelineController(QObject *parent = nullptr);

    int playheadPosition() const;
    void setPlayheadPosition(int frameIndex);
    QAbstractListModel* clipModel() const;
    QVariantList selectedClipIndices() const;
    void setSelectedClipIndices(const QVariantList& indices);
    int clipCount() const;
    bool isIsolating() const { return m_activeIsolationCount > 0; }
    int activeIsolationProgress() const { return m_activeIsolationProgress; }
    bool isGeneratingAIVoice() const { return m_activeAIVoiceCount > 0; }
    int activeAIVoiceProgress() const { return m_activeAIVoiceProgress; }

    int videoTrackCount() const { return m_videoTrackCount; }
    int audioTrackCount() const { return m_audioTrackCount; }
    
    Q_INVOKABLE void cleanupEmptyTracks();

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
     * @brief Deletes all currently selected clips.
     */
    Q_INVOKABLE bool deleteSelectedClips();

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
     * @brief Moves multiple selected clips by a delta in time and tracks.
     */
    Q_INVOKABLE bool moveSelectedClips(double deltaSeconds, int deltaTrackIndex, bool linked);
    

    Q_INVOKABLE bool isTrackEmpty(bool isVideo, int trackIndex) const;
    Q_INVOKABLE void groupSelectedClips();
    Q_INVOKABLE void ungroupSelectedClips();
    /**
     * @brief Splits a clip into two separate clips at the specified split time.
     */
    Q_INVOKABLE int splitClip(int row, double splitSeconds, bool linked);

    /**
     * @brief Applies AI vocal isolation to a clip to separate voice from background noise.
     */
    Q_INVOKABLE void setVocalIsolation(int clipIndex, int isolationType);

    /**
     * @brief Generates AI voice for the given SRT clip and adds audio clips to the timeline.
     */
    Q_INVOKABLE void generateAIVoiceFromSrt(const QString& language);

    /**
     * @brief Mutes or unmutes the audio of a clip.
     */
    Q_INVOKABLE void setClipMuted(int clipIndex, bool muted);

    /**
     * @brief Track State Management
     */
    Q_INVOKABLE void setTrackLocked(bool isVideo, int trackIndex, bool locked);
    Q_INVOKABLE bool isTrackLocked(bool isVideo, int trackIndex) const;
    
    Q_INVOKABLE void setTrackHidden(bool isVideo, int trackIndex, bool hidden);
    Q_INVOKABLE bool isTrackHidden(bool isVideo, int trackIndex) const;
    
    Q_INVOKABLE void setTrackMuted(bool isVideo, int trackIndex, bool muted);
    Q_INVOKABLE bool isTrackMuted(bool isVideo, int trackIndex) const;

    /**
     * @brief Clears all clips from the timeline.
     */
    Q_INVOKABLE void clearClips();
    
    /**
     * @brief Automatically synchronizes visual clips to voice clips based on timestamps.
     */
    Q_INVOKABLE void autoEditSync();
    
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
    Q_INVOKABLE QList<double> getSnapPoints() const;
signals:
    void playheadPositionChanged();
    void selectedClipIndicesChanged();
    void timelineChanged();
    void isIsolatingChanged();
    void activeIsolationProgressChanged();
    void isGeneratingAIVoiceChanged();
    void activeAIVoiceProgressChanged();
    void trackCountsChanged();
    void trackStateChanged(bool isVideo, int trackIndex);

private:
    void calculateTrackCounts();
    int m_playheadPosition = 0;
    TimelineClipModel* m_clipModel = nullptr;
    QUndoStack* m_undoStack = nullptr;
    QVariantList m_selectedClipIndices;
    int m_nextLinkGroupId = 1;
    VocalIsolator* m_vocalIsolator = nullptr;
    AIVoiceGenerator* m_aiVoiceGenerator = nullptr;
    int m_activeIsolationCount = 0;
    int m_activeIsolationProgress = 0;
    int m_activeAIVoiceCount = 0;
    int m_activeAIVoiceProgress = -1;
    
    int m_videoTrackCount = 3;
    int m_audioTrackCount = 3;

    QMap<int, bool> m_videoTrackLocked;
    QMap<int, bool> m_videoTrackHidden;
    QMap<int, bool> m_videoTrackMuted;
    
    QMap<int, bool> m_audioTrackLocked;
    QMap<int, bool> m_audioTrackHidden;
    QMap<int, bool> m_audioTrackMuted;
};

#endif // TIMELINECONTROLLER_H
