#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <QString>
#include <QCache>
#include <QMutex>

class AudioWaveformProvider : public QQuickImageProvider
{
public:
    AudioWaveformProvider();
    ~AudioWaveformProvider() override = default;

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QImage generateWaveform(const QString& filePath, const QSize& targetSize);
    
    QMutex m_mutex;
    QCache<QString, QImage> m_cache;
};
