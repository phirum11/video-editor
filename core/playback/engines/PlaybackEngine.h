#pragma once

#include "core/playback/engines/AudioEngine.h"
#include "core/effects/models/EffectData.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QImage>
#include <QQuickPaintedItem>
#include <QSize>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

extern "C" {
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct SwrContext;
struct SwsContext;
}

class PlaybackEngine : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString clipName READ clipName NOTIFY clipChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY clipChanged)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double sequenceDuration READ sequenceDuration WRITE setSequenceDuration NOTIFY sequenceDurationChanged)
    Q_PROPERTY(double position READ position WRITE seek NOTIFY positionChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY clipChanged)
    Q_PROPERTY(bool hasAudio READ hasAudio NOTIFY clipChanged)
    Q_PROPERTY(double frameRate READ frameRate NOTIFY clipChanged)
    Q_PROPERTY(double audioLevelLeft READ audioLevelLeft NOTIFY audioLevelsChanged)
    Q_PROPERTY(double audioLevelRight READ audioLevelRight NOTIFY audioLevelsChanged)
    Q_PROPERTY(int sourceVideoWidth READ sourceVideoWidth NOTIFY clipChanged)
    Q_PROPERTY(int sourceVideoHeight READ sourceVideoHeight NOTIFY clipChanged)

public:
    explicit PlaybackEngine(QQuickItem* parent = nullptr);
    ~PlaybackEngine() override;

    QString clipName() const;
    QString filePath() const;
    double duration() const noexcept;
    double sequenceDuration() const noexcept;
    double position() const noexcept;
    bool isPlaying() const noexcept;
    bool hasVideo() const noexcept;
    bool hasAudio() const noexcept;
    double frameRate() const noexcept;
    double audioLevelLeft() const noexcept;
    double audioLevelRight() const noexcept;
    int sourceVideoWidth() const noexcept;
    int sourceVideoHeight() const noexcept;

    void paint(QPainter* painter) override;

    Q_INVOKABLE bool loadClip(const QString& name,
                              const QString& path,
                              double knownDuration,
                              bool expectedVideo,
                              const QString& originalPath = QString());
    Q_INVOKABLE void clear();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void seek(double seconds);
    Q_INVOKABLE void stepFrame(int frames);
    Q_INVOKABLE void setClipStartOffset(double offset);
    Q_INVOKABLE void setSequenceDuration(double duration);
    Q_INVOKABLE void setSourceInPoint(double inPoint);

    void setClipEffects(const ClipEffects& effects);

signals:
    void clipChanged();
    void durationChanged();
    void sequenceDurationChanged();
    void positionChanged();
    void playingChanged();
    void audioLevelsChanged();
    void playbackError(const QString& message);

private slots:
    void tick();

private:
    void closeDecoders();
    bool openVideoDecoder(const QString& path);
    bool openAudioDecoder(const QString& path);

    bool seekVideo(double seconds, bool publishFrame);
    bool seekAudio(double seconds);
    bool decodeNextVideoFrame(QImage* image, double* timestampSeconds, double minConvertedTimestampSeconds = -1.0);
    bool decodeNextAudioChunk(QByteArray* pcm);
    QSize playbackFrameSize(int sourceWidth, int sourceHeight) const;
    void feedAudio();
    bool writePendingAudio();
    void setPosition(double seconds, bool forceNotify = false);
    void setFrame(QImage image, double timestampSeconds);
    void setPlaying(bool playing);
    void setAudioLevels(double left, double right);
    void updateAudioLevels(const QByteArray& pcm);
    void reportError(const QString& message);
    bool isInClipRange(double timelineSeconds) const;
    double toClipTime(double timelineSeconds) const;
    void clearFrameToBlack();

    QString m_clipName;
    QString m_filePath;
    double m_duration = 0.0;
    double m_position = 0.0;
    double m_playStartPosition = 0.0;
    double m_lastVideoFrameSeconds = -1.0;
    double m_frameRate = 30.0;
    double m_audioSkipUntilSeconds = 0.0;
    double m_audioLevelLeft = 0.0;
    double m_audioLevelRight = 0.0;
    double m_clipStartOffset = 0.0;
    double m_clipFileDuration = 0.0;
    double m_sequenceDuration = 0.0;
    double m_sourceInPoint = 0.0;
    bool m_playing = false;
    bool m_hasVideo = false;
    bool m_hasAudio = false;
    bool m_videoEof = false;
    bool m_audioEof = false;
    bool m_wasInClipRange = false;

    QImage m_frame;
    QImage m_processedFrame;
    ClipEffects m_currentEffects;
    QTimer m_timer;
    QElapsedTimer m_clock;
    QElapsedTimer m_positionNotifyClock;
    qint64 m_lastPositionNotifyMs = -1;
    AudioEngine m_audioEngine;
    QByteArray m_pendingAudio;

    AVFormatContext* m_videoFormat = nullptr;
    AVCodecContext* m_videoCodec = nullptr;
    AVFrame* m_videoFrame = nullptr;
    AVPacket* m_videoPacket = nullptr;
    SwsContext* m_sws = nullptr;
    int m_videoStreamIndex = -1;

    AVFormatContext* m_audioFormat = nullptr;
    AVCodecContext* m_audioCodec = nullptr;
    AVFrame* m_audioFrame = nullptr;
    AVPacket* m_audioPacket = nullptr;
    SwrContext* m_swr = nullptr;
    int m_audioStreamIndex = -1;
};
