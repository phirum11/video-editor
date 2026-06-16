#include "core/actions/handlers/ToolActions.h"
#include "core/actions/managers/ActionManager.h"
#include <QDebug>

ToolActions::ToolActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void ToolActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("tool.selection", "Selection Tool", "V");
    am.registerAction("tool.razor", "Razor Tool", "C");
    am.registerAction("tool.ripple", "Ripple Edit Tool", "B");
    am.registerAction("tool.pen", "Pen Tool", "P");
}
