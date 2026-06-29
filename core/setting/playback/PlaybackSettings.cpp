#include "core/setting/playback/PlaybackSettings.h"

PlaybackSettings::PlaybackSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("Playback");
    m_playbackResolution = settings.value("playbackResolution", "Full").toString();
    m_videoRenderer = settings.value("videoRenderer", "Mercury GPU Acceleration").toString();
    m_dropFramesToMaintainRealtime = settings.value("dropFramesToMaintainRealtime", true).toBool();
    m_preRollBeforePlayback = settings.value("preRollBeforePlayback", false).toBool();
    settings.endGroup();
}

QString PlaybackSettings::playbackResolution() const { return m_playbackResolution; }
void PlaybackSettings::setPlaybackResolution(const QString& res) {
    if (m_playbackResolution != res) {
        m_playbackResolution = res;
        QSettings settings;
        settings.beginGroup("Playback");
        settings.setValue("playbackResolution", res);
        settings.endGroup();
        emit playbackResolutionChanged();
    }
}

QString PlaybackSettings::videoRenderer() const { return m_videoRenderer; }
void PlaybackSettings::setVideoRenderer(const QString& renderer) {
    if (m_videoRenderer != renderer) {
        m_videoRenderer = renderer;
        QSettings settings;
        settings.beginGroup("Playback");
        settings.setValue("videoRenderer", renderer);
        settings.endGroup();
        emit videoRendererChanged();
    }
}

bool PlaybackSettings::dropFramesToMaintainRealtime() const { return m_dropFramesToMaintainRealtime; }
void PlaybackSettings::setDropFramesToMaintainRealtime(bool drop) {
    if (m_dropFramesToMaintainRealtime != drop) {
        m_dropFramesToMaintainRealtime = drop;
        QSettings settings;
        settings.beginGroup("Playback");
        settings.setValue("dropFramesToMaintainRealtime", drop);
        settings.endGroup();
        emit dropFramesToMaintainRealtimeChanged();
    }
}

bool PlaybackSettings::preRollBeforePlayback() const { return m_preRollBeforePlayback; }
void PlaybackSettings::setPreRollBeforePlayback(bool preRoll) {
    if (m_preRollBeforePlayback != preRoll) {
        m_preRollBeforePlayback = preRoll;
        QSettings settings;
        settings.beginGroup("Playback");
        settings.setValue("preRollBeforePlayback", preRoll);
        settings.endGroup();
        emit preRollBeforePlaybackChanged();
    }
}
