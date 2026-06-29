#include "core/setting/cache/CacheSettings.h"

CacheSettings::CacheSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("Cache");
    m_mediaCachePath = settings.value("mediaCachePath", "C:\\Users\\AppData\\Local\\VideoStudio\\Cache").toString();
    m_currentCacheSize = settings.value("currentCacheSize", "0 MB").toString();
    m_autoDeleteOldCacheFiles = settings.value("autoDeleteOldCacheFiles", true).toBool();
    m_maximumCacheSize = settings.value("maximumCacheSize", "10 GB").toString();
    settings.endGroup();
}

QString CacheSettings::mediaCachePath() const { return m_mediaCachePath; }
void CacheSettings::setMediaCachePath(const QString& path) {
    if (m_mediaCachePath != path) {
        m_mediaCachePath = path;
        QSettings settings;
        settings.beginGroup("Cache");
        settings.setValue("mediaCachePath", path);
        settings.endGroup();
        emit mediaCachePathChanged();
    }
}

QString CacheSettings::currentCacheSize() const { return m_currentCacheSize; }
void CacheSettings::setCurrentCacheSize(const QString& size) {
    if (m_currentCacheSize != size) {
        m_currentCacheSize = size;
        QSettings settings;
        settings.beginGroup("Cache");
        settings.setValue("currentCacheSize", size);
        settings.endGroup();
        emit currentCacheSizeChanged();
    }
}

bool CacheSettings::autoDeleteOldCacheFiles() const { return m_autoDeleteOldCacheFiles; }
void CacheSettings::setAutoDeleteOldCacheFiles(bool autoDelete) {
    if (m_autoDeleteOldCacheFiles != autoDelete) {
        m_autoDeleteOldCacheFiles = autoDelete;
        QSettings settings;
        settings.beginGroup("Cache");
        settings.setValue("autoDeleteOldCacheFiles", autoDelete);
        settings.endGroup();
        emit autoDeleteOldCacheFilesChanged();
    }
}

QString CacheSettings::maximumCacheSize() const { return m_maximumCacheSize; }
void CacheSettings::setMaximumCacheSize(const QString& size) {
    if (m_maximumCacheSize != size) {
        m_maximumCacheSize = size;
        QSettings settings;
        settings.beginGroup("Cache");
        settings.setValue("maximumCacheSize", size);
        settings.endGroup();
        emit maximumCacheSizeChanged();
    }
}
