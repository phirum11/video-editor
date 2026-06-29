#include "GifFrameDecoder.h"

#include <QDebug>
#include <QFileInfo>
#include <QImageReader>
#include <QMutexLocker>
#include <QUrl>

#include <algorithm>
#include <cmath>

QMutex GifFrameDecoder::s_cacheMutex;
QHash<QString, GifFrameDecoder*> GifFrameDecoder::s_cache;

bool GifFrameDecoder::load(const QString& filePath)
{
    m_frames.clear();
    m_totalDurationMs = 0.0;
    m_filePath = filePath;

    QImageReader reader(filePath);
    if (!reader.canRead()) {
        qWarning() << "GifFrameDecoder: cannot read" << filePath
                   << reader.errorString();
        return false;
    }

    // Force ARGB32_Premultiplied so alpha compositing is trivial later
    const QSize gifSize = reader.size();

    while (reader.canRead()) {
        GifFrame frame;
        frame.image = reader.read();
        if (frame.image.isNull()) {
            break;
        }

        // Ensure a consistent premultiplied format for fast painting
        if (frame.image.format() != QImage::Format_ARGB32_Premultiplied) {
            frame.image = frame.image.convertToFormat(
                QImage::Format_ARGB32_Premultiplied);
        }

        // nextImageDelay() gives the delay *before* moving to the next frame
        frame.delayMs = reader.nextImageDelay();
        if (frame.delayMs <= 0) {
            frame.delayMs = 100; // Fallback: 10 fps
        }

        m_totalDurationMs += frame.delayMs;
        m_frames.append(std::move(frame));
    }

    if (m_frames.isEmpty()) {
        qWarning() << "GifFrameDecoder: no frames decoded from" << filePath;
        return false;
    }

    return true;
}

QImage GifFrameDecoder::frameAt(double elapsedSeconds) const
{
    const int idx = frameIndexAt(elapsedSeconds);
    if (idx < 0 || idx >= m_frames.size()) {
        return {};
    }
    return m_frames[idx].image;
}

int GifFrameDecoder::frameIndexAt(double elapsedSeconds) const
{
    if (m_frames.isEmpty() || m_totalDurationMs <= 0.0) {
        return -1;
    }

    // Convert to ms and wrap (loop)
    double elapsedMs = std::max(0.0, elapsedSeconds * 1000.0);
    elapsedMs = std::fmod(elapsedMs, m_totalDurationMs);

    double accumulated = 0.0;
    for (int i = 0; i < m_frames.size(); ++i) {
        accumulated += m_frames[i].delayMs;
        if (elapsedMs < accumulated) {
            return i;
        }
    }

    // Should not reach here, but return last frame as fallback
    return m_frames.size() - 1;
}

GifFrameDecoder* GifFrameDecoder::get(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return nullptr;
    }

    QString actualPath = filePath;
    if (actualPath.startsWith(QStringLiteral("file://"))) {
        actualPath = QUrl(actualPath).toLocalFile();
    }

    // Normalize path for consistent cache keys
    const QString key = QFileInfo(actualPath).absoluteFilePath();

    QMutexLocker locker(&s_cacheMutex);

    auto it = s_cache.find(key);
    if (it != s_cache.end()) {
        return it.value()->isValid() ? it.value() : nullptr;
    }

    // Not cached — decode now (still under lock; acceptable because
    // GIF decode is fast for these small overlay GIFs, typically < 1 MB)
    auto* decoder = new GifFrameDecoder();
    if (!decoder->load(actualPath)) {
        delete decoder;
        // Insert a nullptr sentinel so we don't re-attempt a bad file
        s_cache.insert(key, nullptr);
        return nullptr;
    }

    s_cache.insert(key, decoder);
    return decoder;
}

void GifFrameDecoder::evict(const QString& filePath)
{
    QString actualPath = filePath;
    if (actualPath.startsWith(QStringLiteral("file://"))) {
        actualPath = QUrl(actualPath).toLocalFile();
    }
    const QString key = QFileInfo(actualPath).absoluteFilePath();

    QMutexLocker locker(&s_cacheMutex);
    auto it = s_cache.find(key);
    if (it != s_cache.end()) {
        delete it.value();
        s_cache.erase(it);
    }
}

void GifFrameDecoder::clearCache()
{
    QMutexLocker locker(&s_cacheMutex);
    qDeleteAll(s_cache);
    s_cache.clear();
}
