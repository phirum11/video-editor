#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class CacheSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mediaCachePath READ mediaCachePath WRITE setMediaCachePath NOTIFY mediaCachePathChanged)
    Q_PROPERTY(QString currentCacheSize READ currentCacheSize WRITE setCurrentCacheSize NOTIFY currentCacheSizeChanged)
    Q_PROPERTY(bool autoDeleteOldCacheFiles READ autoDeleteOldCacheFiles WRITE setAutoDeleteOldCacheFiles NOTIFY autoDeleteOldCacheFilesChanged)
    Q_PROPERTY(QString maximumCacheSize READ maximumCacheSize WRITE setMaximumCacheSize NOTIFY maximumCacheSizeChanged)

public:
    explicit CacheSettings(QObject* parent = nullptr);

    QString mediaCachePath() const;
    void setMediaCachePath(const QString& path);

    QString currentCacheSize() const;
    void setCurrentCacheSize(const QString& size);

    bool autoDeleteOldCacheFiles() const;
    void setAutoDeleteOldCacheFiles(bool autoDelete);

    QString maximumCacheSize() const;
    void setMaximumCacheSize(const QString& size);

signals:
    void mediaCachePathChanged();
    void currentCacheSizeChanged();
    void autoDeleteOldCacheFilesChanged();
    void maximumCacheSizeChanged();

private:
    QString m_mediaCachePath;
    QString m_currentCacheSize;
    bool m_autoDeleteOldCacheFiles;
    QString m_maximumCacheSize;
};
