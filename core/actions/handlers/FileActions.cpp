#include "core/actions/handlers/FileActions.h"
#include "core/actions/managers/ActionManager.h"
#include <QDebug>

FileActions::FileActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void FileActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("file.new", "New Project", "Ctrl+N");
    am.registerAction("file.open", "Open Project...", "Ctrl+O");
    am.registerAction("file.recent1", "Project 1.vsp");
    am.registerAction("file.recent2", "Project 2.vsp");
    am.registerAction("file.save", "Save", "Ctrl+S");
    am.registerAction("file.save_as", "Save As...", "Ctrl+Shift+S");
    am.registerAction("file.import", "Import Media...", "Ctrl+I");
    am.registerAction("file.project_settings", "Project Settings...");
    am.registerAction("file.exit", "Exit", "Ctrl+Q");
}
