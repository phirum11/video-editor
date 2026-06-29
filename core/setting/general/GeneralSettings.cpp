#include "core/setting/general/GeneralSettings.h"

GeneralSettings::GeneralSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("General");
    m_defaultLocation = settings.value("defaultLocation", "C:\\Users\\Public\\Videos\\Projects").toString();
    m_autoSaveInterval = settings.value("autoSaveInterval", "5 minutes").toString();
    m_undoHistoryLimit = settings.value("undoHistoryLimit", "50 steps").toString();
    m_showWelcomeScreen = settings.value("showWelcomeScreen", true).toBool();
    m_loadLastProjectOnStart = settings.value("loadLastProjectOnStart", false).toBool();
    m_checkForUpdatesAutomatically = settings.value("checkForUpdatesAutomatically", true).toBool();
    settings.endGroup();
}

QString GeneralSettings::defaultLocation() const { return m_defaultLocation; }
void GeneralSettings::setDefaultLocation(const QString& location) {
    if (m_defaultLocation != location) {
        m_defaultLocation = location;
        QSettings settings;
        settings.beginGroup("General");
        settings.setValue("defaultLocation", location);
        settings.endGroup();
        emit defaultLocationChanged();
    }
}

QString GeneralSettings::autoSaveInterval() const { return m_autoSaveInterval; }
void GeneralSettings::setAutoSaveInterval(const QString& interval) {
    if (m_autoSaveInterval != interval) {
        m_autoSaveInterval = interval;
        QSettings settings;
        settings.beginGroup("General");
        settings.setValue("autoSaveInterval", interval);
        settings.endGroup();
        emit autoSaveIntervalChanged();
    }
}

QString GeneralSettings::undoHistoryLimit() const { return m_undoHistoryLimit; }
void GeneralSettings::setUndoHistoryLimit(const QString& limit) {
    if (m_undoHistoryLimit != limit) {
        m_undoHistoryLimit = limit;
        QSettings settings;
        settings.beginGroup("General");
        settings.setValue("undoHistoryLimit", limit);
        settings.endGroup();
        emit undoHistoryLimitChanged();
    }
}

bool GeneralSettings::showWelcomeScreen() const { return m_showWelcomeScreen; }
void GeneralSettings::setShowWelcomeScreen(bool show) {
    if (m_showWelcomeScreen != show) {
        m_showWelcomeScreen = show;
        QSettings settings;
        settings.beginGroup("General");
        settings.setValue("showWelcomeScreen", show);
        settings.endGroup();
        emit showWelcomeScreenChanged();
    }
}

bool GeneralSettings::loadLastProjectOnStart() const { return m_loadLastProjectOnStart; }
void GeneralSettings::setLoadLastProjectOnStart(bool load) {
    if (m_loadLastProjectOnStart != load) {
        m_loadLastProjectOnStart = load;
        QSettings settings;
        settings.beginGroup("General");
        settings.setValue("loadLastProjectOnStart", load);
        settings.endGroup();
        emit loadLastProjectOnStartChanged();
    }
}

bool GeneralSettings::checkForUpdatesAutomatically() const { return m_checkForUpdatesAutomatically; }
void GeneralSettings::setCheckForUpdatesAutomatically(bool check) {
    if (m_checkForUpdatesAutomatically != check) {
        m_checkForUpdatesAutomatically = check;
        QSettings settings;
        settings.beginGroup("General");
        settings.setValue("checkForUpdatesAutomatically", check);
        settings.endGroup();
        emit checkForUpdatesAutomaticallyChanged();
    }
}
