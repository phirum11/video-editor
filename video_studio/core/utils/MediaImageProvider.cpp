#include "MediaImageProvider.h"
#include "core/media/MediaItem.h"
#include <QMutexLocker>
#include <QUrl>

#include <algorithm>

MediaImageProvider::MediaImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image),
      m_cache(64 * 1024)
{
}

QImage MediaImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    try {
        const QString decodedId = QUrl::fromPercentEncoding(id.toUtf8());
        const QStringList parts = decodedId.split('|');
        const QString filePath = parts.isEmpty() ? decodedId : parts.first();

        bool parsedTimestamp = false;
        const double timestampSeconds = parts.size() > 1
            ? parts.at(1).toDouble(&parsedTimestamp)
            : 0.0;
        const double frameSeconds = parsedTimestamp ? std::max(0.0, timestampSeconds) : 0.0;
        const QSize targetSize = requestedSize.isValid() && !requestedSize.isEmpty()
            ? requestedSize
            : QSize(320, 180);
        const QString cacheKey = QStringLiteral("%1|%2|%3x%4")
            .arg(filePath,
                 QString::number(frameSeconds, 'f', 3),
                 QString::number(targetSize.width()),
                 QString::number(targetSize.height()));

        {
            QMutexLocker locker(&m_cacheMutex);
            if (const QImage* cachedFrame = m_cache.object(cacheKey)) {
                if (size) {
                    *size = cachedFrame->size();
                }
                return *cachedFrame;
            }
        }

        MediaItem item(filePath);
        QImage frame = item.extractFrame(frameSeconds, targetSize);
        
        if (size) {
            *size = frame.size();
        }

        if (!frame.isNull()) {
            const qsizetype frameCostKb = std::max<qsizetype>(1, frame.sizeInBytes() / 1024);
            QMutexLocker locker(&m_cacheMutex);
            m_cache.insert(cacheKey, new QImage(frame), static_cast<int>(std::min<qsizetype>(frameCostKb, 64 * 1024)));
        }

        return frame;
    } catch (...) {
        return QImage();
    }
}
