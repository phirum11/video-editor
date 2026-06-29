#include "MediaImageProvider.h"
#include "core/media/models/MediaItem.h"
#include <QMutexLocker>
#include <QSemaphore>
#include <QUrl>
#include <QDebug>
#include <QPainter>
#include <QColor>
#include <QRandomGenerator>

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
        QSize targetSize(320, 180);
        if (requestedSize.width() > 0 && requestedSize.height() > 0) {
            targetSize = requestedSize;
        } else if (requestedSize.width() > 0) {
            targetSize = QSize(requestedSize.width(), 1);
        } else if (requestedSize.height() > 0) {
            targetSize = QSize(1, requestedSize.height());
        }

        const QString cacheKey = QStringLiteral("%1|%2|%3x%4")
            .arg(filePath,
                 QString::number(frameSeconds, 'f', 3),
                 QString::number(targetSize.width()),
                 QString::number(targetSize.height()));
                 
        qDebug() << "MediaImageProvider cacheKey:" << cacheKey;

        {
            QMutexLocker locker(&m_cacheMutex);
            if (const QImage* cachedFrame = m_cache.object(cacheKey)) {
                if (size) {
                    *size = cachedFrame->size();
                }
                return *cachedFrame;
            }
        }

        static QSemaphore s_ffmpegSemaphore(4);
        QImage frame;

        QString lowerPath = filePath.toLower();
        bool isImageExt = lowerPath.endsWith(".png") || lowerPath.endsWith(".jpg") || 
                          lowerPath.endsWith(".jpeg") || lowerPath.endsWith(".webp") || 
                          lowerPath.endsWith(".bmp") || lowerPath.endsWith(".gif");
                          
        if (isImageExt) {
            QImage rawImage(filePath);
            if (!rawImage.isNull()) {
                if (requestedSize.width() > 0 && requestedSize.height() > 0) {
                    frame = rawImage.scaled(requestedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                } else if (requestedSize.width() > 0) {
                    frame = rawImage.scaledToWidth(requestedSize.width(), Qt::SmoothTransformation);
                } else if (requestedSize.height() > 0) {
                    frame = rawImage.scaledToHeight(requestedSize.height(), Qt::SmoothTransformation);
                } else {
                    frame = rawImage;
                }
            }
        }
        
        if (frame.isNull()) {
            s_ffmpegSemaphore.acquire();
            MediaItem item(filePath);
            if (item.hasVideo()) {
                frame = item.extractFrame(frameSeconds, targetSize);
            } else if (item.hasAudio()) {
                QSize actualSize = targetSize;
                if (actualSize.width() <= 1 || actualSize.height() <= 1) {
                    actualSize = QSize(320, 180);
                }
                frame = QImage(actualSize, QImage::Format_ARGB32);
                frame.fill(QColor("#242424")); 
                QPainter painter(&frame);
                painter.setRenderHint(QPainter::Antialiasing, false);

                uint qhash = qHash(filePath);
                QRandomGenerator generator(qhash);

                int barWidth = 2;
                int spacing = 1;
                for (int x = 0; x < actualSize.width(); x += barWidth + spacing) {
                    float pct = (generator.bounded(25) + 75) / 100.0f; 
                    int h = (actualSize.height() - 4) * pct; 
                    int y = actualSize.height() - h;

                    int topHeight = qMax(2, h / 12);
                    painter.fillRect(x, y + topHeight, barWidth, h - topHeight, QColor("#008fd3"));
                    painter.fillRect(x, y, barWidth, topHeight, QColor("#f94719"));
                }
            }
            s_ffmpegSemaphore.release();
        }
        
        if (size) {
            *size = frame.size();
        }

        if (!frame.isNull()) {
            const qsizetype frameCostKb = std::max<qsizetype>(1, frame.sizeInBytes() / 1024);
            QMutexLocker locker(&m_cacheMutex);
            m_cache.insert(cacheKey, new QImage(frame), static_cast<int>(std::min<qsizetype>(frameCostKb, 64 * 1024)));
        }

        return frame;
    } catch (const std::exception& e) {
        qDebug() << "MediaImageProvider ERROR:" << e.what() << "for ID:" << id;
        return QImage();
    } catch (...) {
        qDebug() << "MediaImageProvider UNKNOWN ERROR for ID:" << id;
        return QImage();
    }
}
