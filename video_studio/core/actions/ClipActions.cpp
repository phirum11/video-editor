#include "ClipActions.h"
#include "ActionManager.h"
#include <QDebug>

ClipActions::ClipActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void ClipActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("clip.enable", "Enable/Disable");
    am.registerAction("clip.link", "Link/Unlink", "Ctrl+L");
    am.registerAction("clip.speed", "Speed/Duration...", "Ctrl+R");
    am.registerAction("clip.audio_channels", "Audio Channels...");
    am.registerAction("clip.rename", "Rename Clip...");
    am.registerAction("clip.properties", "Properties");
}
