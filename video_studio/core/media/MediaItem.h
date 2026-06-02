#pragma once

#include <QImage>
#include <QSize>
#include <QString>
#include <QtGlobal>

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class MediaItem final {
public:
    explicit MediaItem(QString filePath);
    explicit MediaItem(const std::string& filePath);
    ~MediaItem() = default;

    MediaItem(const MediaItem&) = default;
    MediaItem& operator=(const MediaItem&) = default;
    MediaItem(MediaItem&&) noexcept = default;
    MediaItem& operator=(MediaItem&&) noexcept = default;

    const QString& filePath() const noexcept;
    QString fileName() const;

    qint64 durationMicroseconds() const noexcept;
    double durationSeconds() const noexcept;
    QSize resolution() const noexcept;
    int width() const noexcept;
    int height() const noexcept;
    double frameRate() const noexcept;
    bool hasAudio() const noexcept;
    bool hasVideo() const noexcept;
    int videoStreamIndex() const noexcept;
    int audioStreamIndex() const noexcept;

    QImage extractFrame(double timestampSeconds, QSize requestedSize = {}) const;

private:
    void loadMetadata();

    QString m_filePath;
    qint64 m_durationUs = 0;
    QSize m_resolution;
    double m_frameRate = 0.0;
    bool m_hasAudio = false;
    bool m_hasVideo = false;
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
};
