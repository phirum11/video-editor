#include "EditActions.h"
#include "ActionManager.h"
#include <QDebug>

EditActions::EditActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void EditActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("edit.undo", "Undo", "Ctrl+Z");
    am.registerAction("edit.redo", "Redo", "Ctrl+Shift+Z");
    am.registerAction("edit.cut", "Cut");
    am.registerAction("edit.copy", "Copy");
    am.registerAction("edit.paste", "Paste");
    am.registerAction("edit.delete", "Delete");
    am.registerAction("edit.ripple_delete", "Ripple Delete", "Shift+Del");
    am.registerAction("edit.select_all", "Select All", "Ctrl+A");
    am.registerAction("edit.deselect_all", "Deselect All", "Ctrl+Shift+A");
    am.registerAction("edit.preferences", "Preferences");
}
