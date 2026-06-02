#include "ViewActions.h"
#include "ActionManager.h"
#include <QDebug>

ViewActions::ViewActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void ViewActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("view.workspace.editing", "Editing");
    am.registerAction("view.workspace.color", "Color Grading");
    am.registerAction("view.workspace.audio", "Audio Processing");
    am.registerAction("view.workspace.default", "Default");
    am.registerAction("view.playback.full", "Full");
    am.registerAction("view.playback.half", "1/2");
    am.registerAction("view.playback.quarter", "1/4");
    am.registerAction("view.playback.eighth", "1/8");
    am.registerAction("view.panel.project", "Project Browser");
    am.registerAction("view.panel.effects", "Effects");
    am.registerAction("view.panel.timeline", "Timeline");
    am.registerAction("view.panel.source", "Source Monitor");
    am.registerAction("view.zoom_in", "Zoom In");
    am.registerAction("view.zoom_out", "Zoom Out");
    am.registerAction("view.fit_timeline", "Fit to Timeline");
    
    // Layout
    am.registerAction("view.layout.default", "Default Layout");
    am.registerAction("view.layout.vertical", "Vertical Layout");
}
