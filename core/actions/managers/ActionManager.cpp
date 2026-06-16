#include "core/actions/managers/ActionManager.h"
#include <QDebug>

ActionManager& ActionManager::instance() {
    static ActionManager instance;
    return instance;
}

ActionManager::ActionManager(QObject* parent) : QObject(parent) {}

void ActionManager::registerAction(const QString& id, const QString& text, const QString& shortcut, const QString& icon) {
    QVariantMap action;
    action["id"] = id;
    action["text"] = text;
    action["shortcut"] = shortcut;
    action["icon"] = icon;
    m_actions[id] = action;
}

void ActionManager::executeAction(const QString& id) {
    if (m_actions.contains(id)) {
        qDebug() << "Executing Action:" << id;
        emit actionTriggered(id);
    } else {
        qWarning() << "Action not found:" << id;
    }
}

QVariantMap ActionManager::getAction(const QString& id) const {
    return m_actions.value(id);
}
