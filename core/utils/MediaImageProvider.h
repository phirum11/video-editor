#pragma once

#include <QCache>
#include <QMutex>
#include <QQuickImageProvider>

class MediaImageProvider : public QQuickImageProvider {
public:
    MediaImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QCache<QString, QImage> m_cache;
    QMutex m_cacheMutex;
};
