#pragma once

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QString>
#include <QVector>

struct GifFrame {
    QImage image;
    int delayMs = 100; // Default 100ms (10fps) if GIF has no delay
};

class GifFrameDecoder
{
public:
    GifFrameDecoder() = default;

    bool load(const QString& filePath);
    bool isValid() const { return !m_frames.isEmpty(); }

    int frameCount() const { return m_frames.size(); }
    double totalDurationMs() const { return m_totalDurationMs; }
    double totalDurationSeconds() const { return m_totalDurationMs / 1000.0; }

    /// Returns the correct frame for a given elapsed time (in seconds),
    /// automatically looping when elapsedSeconds exceeds the GIF duration.
    QImage frameAt(double elapsedSeconds) const;

    /// Returns the frame index for a given elapsed time.
    int frameIndexAt(double elapsedSeconds) const;

    /// Thread-safe static cache: returns a cached decoder (creates and loads if not found).
    /// Returns nullptr if the file cannot be loaded.
    static GifFrameDecoder* get(const QString& filePath);

    /// Removes a specific entry from the cache.
    static void evict(const QString& filePath);

    /// Clears the entire cache.
    static void clearCache();

private:
    QVector<GifFrame> m_frames;
    double m_totalDurationMs = 0.0;
    QString m_filePath;

    static QMutex s_cacheMutex;
    static QHash<QString, GifFrameDecoder*> s_cache;
};
