#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class PlaybackSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString playbackResolution READ playbackResolution WRITE setPlaybackResolution NOTIFY playbackResolutionChanged)
    Q_PROPERTY(QString videoRenderer READ videoRenderer WRITE setVideoRenderer NOTIFY videoRendererChanged)
    Q_PROPERTY(bool dropFramesToMaintainRealtime READ dropFramesToMaintainRealtime WRITE setDropFramesToMaintainRealtime NOTIFY dropFramesToMaintainRealtimeChanged)
    Q_PROPERTY(bool preRollBeforePlayback READ preRollBeforePlayback WRITE setPreRollBeforePlayback NOTIFY preRollBeforePlaybackChanged)

public:
    explicit PlaybackSettings(QObject* parent = nullptr);

    QString playbackResolution() const;
    void setPlaybackResolution(const QString& res);

    QString videoRenderer() const;
    void setVideoRenderer(const QString& renderer);

    bool dropFramesToMaintainRealtime() const;
    void setDropFramesToMaintainRealtime(bool drop);

    bool preRollBeforePlayback() const;
    void setPreRollBeforePlayback(bool preRoll);

signals:
    void playbackResolutionChanged();
    void videoRendererChanged();
    void dropFramesToMaintainRealtimeChanged();
    void preRollBeforePlaybackChanged();

private:
    QString m_playbackResolution;
    QString m_videoRenderer;
    bool m_dropFramesToMaintainRealtime;
    bool m_preRollBeforePlayback;
};
