#include "core/actions/handlers/HelpActions.h"
#include "core/actions/managers/ActionManager.h"
#include <QDebug>

HelpActions::HelpActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void HelpActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("help.docs", "Documentation / User Manual");
    am.registerAction("help.shortcuts", "Keyboard Shortcuts Guide");
    am.registerAction("help.updates", "Check for Updates...");
    am.registerAction("help.about", "About Video Studio Pro");
}
