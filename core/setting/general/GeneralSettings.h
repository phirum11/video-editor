#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class GeneralSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString defaultLocation READ defaultLocation WRITE setDefaultLocation NOTIFY defaultLocationChanged)
    Q_PROPERTY(QString autoSaveInterval READ autoSaveInterval WRITE setAutoSaveInterval NOTIFY autoSaveIntervalChanged)
    Q_PROPERTY(QString undoHistoryLimit READ undoHistoryLimit WRITE setUndoHistoryLimit NOTIFY undoHistoryLimitChanged)
    Q_PROPERTY(bool showWelcomeScreen READ showWelcomeScreen WRITE setShowWelcomeScreen NOTIFY showWelcomeScreenChanged)
    Q_PROPERTY(bool loadLastProjectOnStart READ loadLastProjectOnStart WRITE setLoadLastProjectOnStart NOTIFY loadLastProjectOnStartChanged)
    Q_PROPERTY(bool checkForUpdatesAutomatically READ checkForUpdatesAutomatically WRITE setCheckForUpdatesAutomatically NOTIFY checkForUpdatesAutomaticallyChanged)

public:
    explicit GeneralSettings(QObject* parent = nullptr);

    QString defaultLocation() const;
    void setDefaultLocation(const QString& location);

    QString autoSaveInterval() const;
    void setAutoSaveInterval(const QString& interval);

    QString undoHistoryLimit() const;
    void setUndoHistoryLimit(const QString& limit);

    bool showWelcomeScreen() const;
    void setShowWelcomeScreen(bool show);

    bool loadLastProjectOnStart() const;
    void setLoadLastProjectOnStart(bool load);

    bool checkForUpdatesAutomatically() const;
    void setCheckForUpdatesAutomatically(bool check);

signals:
    void defaultLocationChanged();
    void autoSaveIntervalChanged();
    void undoHistoryLimitChanged();
    void showWelcomeScreenChanged();
    void loadLastProjectOnStartChanged();
    void checkForUpdatesAutomaticallyChanged();

private:
    QString m_defaultLocation;
    QString m_autoSaveInterval;
    QString m_undoHistoryLimit;
    bool m_showWelcomeScreen;
    bool m_loadLastProjectOnStart;
    bool m_checkForUpdatesAutomatically;
};
