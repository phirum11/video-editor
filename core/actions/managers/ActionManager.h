#pragma once
#include <QObject>
#include <QVariantMap>
#include <QMap>

class ActionManager : public QObject {
    Q_OBJECT
public:
    static ActionManager& instance();

    Q_INVOKABLE void registerAction(const QString& id, const QString& text, const QString& shortcut = "", const QString& icon = "");
    Q_INVOKABLE void executeAction(const QString& id);

    QVariantMap getAction(const QString& id) const;

signals:
    void actionTriggered(const QString& id);

private:
    ActionManager(QObject* parent = nullptr);
    ~ActionManager() = default;
    ActionManager(const ActionManager&) = delete;
    ActionManager& operator=(const ActionManager&) = delete;

    QMap<QString, QVariantMap> m_actions;
};
