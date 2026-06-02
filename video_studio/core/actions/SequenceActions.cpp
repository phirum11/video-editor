#include "SequenceActions.h"
#include "ActionManager.h"
#include <QDebug>

SequenceActions::SequenceActions(QObject* parent) : QObject(parent) {
    registerActions();
}

void SequenceActions::registerActions() {
    auto& am = ActionManager::instance();

    am.registerAction("sequence.new", "New Sequence...");
    am.registerAction("sequence.settings", "Sequence Settings...");
    am.registerAction("sequence.render_in_out", "Render In to Out", "Enter");
    am.registerAction("sequence.add_tracks", "Add Tracks");
    am.registerAction("sequence.delete_empty_tracks", "Delete Empty Tracks");
    am.registerAction("sequence.apply_default_transitions", "Apply Default Transitions", "Ctrl+D");
}
