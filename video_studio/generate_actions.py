import os

files = {
    "core/actions/ActionManager.h": """#pragma once
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
""",
    "core/actions/ActionManager.cpp": """#include "ActionManager.h"
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
""",
    "core/actions/MenuManager.h": """#pragma once
#include <QObject>
#include <QVariantList>

class MenuManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList menuStructure READ menuStructure NOTIFY menuStructureChanged)

public:
    static MenuManager& instance();

    QVariantList menuStructure() const;
    void buildDefaultMenu();

signals:
    void menuStructureChanged();

private:
    MenuManager(QObject* parent = nullptr);
    ~MenuManager() = default;
    MenuManager(const MenuManager&) = delete;
    MenuManager& operator=(const MenuManager&) = delete;

    QVariantList m_menuStructure;
    
    QVariantMap createMenu(const QString& title, const QVariantList& items);
    QVariantMap createMenuItem(const QString& actionId);
    QVariantMap createSubMenu(const QString& title, const QVariantList& items);
    QVariantMap createSeparator();
};
""",
    "core/actions/MenuManager.cpp": """#include "MenuManager.h"
#include "ActionManager.h"

MenuManager& MenuManager::instance() {
    static MenuManager instance;
    return instance;
}

MenuManager::MenuManager(QObject* parent) : QObject(parent) {
}

QVariantList MenuManager::menuStructure() const {
    return m_menuStructure;
}

QVariantMap MenuManager::createMenu(const QString& title, const QVariantList& items) {
    QVariantMap menu;
    menu["type"] = "menu";
    menu["title"] = title;
    menu["items"] = items;
    return menu;
}

QVariantMap MenuManager::createMenuItem(const QString& actionId) {
    QVariantMap item;
    item["type"] = "action";
    item["actionId"] = actionId;
    
    QVariantMap actionData = ActionManager::instance().getAction(actionId);
    item["text"] = actionData["text"];
    item["shortcut"] = actionData["shortcut"];
    return item;
}

QVariantMap MenuManager::createSubMenu(const QString& title, const QVariantList& items) {
    QVariantMap menu;
    menu["type"] = "submenu";
    menu["title"] = title;
    menu["items"] = items;
    return menu;
}

QVariantMap MenuManager::createSeparator() {
    QVariantMap item;
    item["type"] = "separator";
    return item;
}

void MenuManager::buildDefaultMenu() {
    QVariantList fileItems = {
        createMenuItem("file.new"),
        createMenuItem("file.open"),
        createSubMenu("Open Recent", {
            createMenuItem("file.recent1"),
            createMenuItem("file.recent2")
        }),
        createSeparator(),
        createMenuItem("file.save"),
        createMenuItem("file.save_as"),
        createSeparator(),
        createMenuItem("file.import"),
        createMenuItem("file.project_settings"),
        createSeparator(),
        createMenuItem("file.exit")
    };

    QVariantList editItems = {
        createMenuItem("edit.undo"),
        createMenuItem("edit.redo"),
        createSeparator(),
        createMenuItem("edit.cut"),
        createMenuItem("edit.copy"),
        createMenuItem("edit.paste"),
        createMenuItem("edit.delete"),
        createMenuItem("edit.ripple_delete"),
        createSeparator(),
        createMenuItem("edit.select_all"),
        createMenuItem("edit.deselect_all"),
        createSeparator(),
        createMenuItem("edit.preferences")
    };

    QVariantList clipItems = {
        createMenuItem("clip.enable"),
        createMenuItem("clip.link"),
        createMenuItem("clip.speed"),
        createMenuItem("clip.audio_channels"),
        createMenuItem("clip.rename"),
        createMenuItem("clip.properties")
    };

    QVariantList seqItems = {
        createMenuItem("sequence.new"),
        createMenuItem("sequence.settings"),
        createSeparator(),
        createMenuItem("sequence.render_in_out"),
        createMenuItem("sequence.add_tracks"),
        createMenuItem("sequence.delete_empty_tracks"),
        createMenuItem("sequence.apply_default_transitions")
    };

    QVariantList viewItems = {
        createSubMenu("Workspaces", {
            createMenuItem("view.workspace.editing"),
            createMenuItem("view.workspace.color"),
            createMenuItem("view.workspace.audio"),
            createMenuItem("view.workspace.default")
        }),
        createSubMenu("Playback Resolution", {
            createMenuItem("view.playback.full"),
            createMenuItem("view.playback.half"),
            createMenuItem("view.playback.quarter"),
            createMenuItem("view.playback.eighth")
        }),
        createSubMenu("Show/Hide Panels", {
            createMenuItem("view.panel.project"),
            createMenuItem("view.panel.effects"),
            createMenuItem("view.panel.timeline"),
            createMenuItem("view.panel.source")
        }),
        createSeparator(),
        createMenuItem("view.zoom_in"),
        createMenuItem("view.zoom_out"),
        createMenuItem("view.fit_timeline")
    };

    QVariantList toolItems = {
        createMenuItem("tool.selection"),
        createMenuItem("tool.razor"),
        createMenuItem("tool.ripple"),
        createMenuItem("tool.pen")
    };

    QVariantList helpItems = {
        createMenuItem("help.docs"),
        createMenuItem("help.shortcuts"),
        createSeparator(),
        createMenuItem("help.updates"),
        createMenuItem("help.about")
    };

    m_menuStructure = {
        createMenu("File", fileItems),
        createMenu("Edit", editItems),
        createMenu("Clip", clipItems),
        createMenu("Sequence", seqItems),
        createMenu("View", viewItems),
        createMenu("Tools", toolItems),
        createMenu("Help", helpItems)
    };

    emit menuStructureChanged();
}
"""
}

# Add Action Category Files
categories = ["File", "Edit", "Clip", "Sequence", "View", "Tool", "Help"]

for cat in categories:
    h_code = f"""#pragma once
#include <QObject>

class {cat}Actions : public QObject {{
    Q_OBJECT
public:
    explicit {cat}Actions(QObject* parent = nullptr);
    void registerActions();
}};
"""
    cpp_code = f"""#include "{cat}Actions.h"
#include "ActionManager.h"
#include <QDebug>

{cat}Actions::{cat}Actions(QObject* parent) : QObject(parent) {{
    registerActions();
}}

void {cat}Actions::registerActions() {{
    auto& am = ActionManager::instance();
"""
    
    if cat == "File":
        cpp_code += """
    am.registerAction("file.new", "New Project", "Ctrl+N");
    am.registerAction("file.open", "Open Project...", "Ctrl+O");
    am.registerAction("file.recent1", "Project 1.vsp");
    am.registerAction("file.recent2", "Project 2.vsp");
    am.registerAction("file.save", "Save", "Ctrl+S");
    am.registerAction("file.save_as", "Save As...", "Ctrl+Shift+S");
    am.registerAction("file.import", "Import Media...", "Ctrl+I");
    am.registerAction("file.project_settings", "Project Settings...");
    am.registerAction("file.exit", "Exit", "Ctrl+Q");
"""
    elif cat == "Edit":
        cpp_code += """
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
"""
    elif cat == "Clip":
        cpp_code += """
    am.registerAction("clip.enable", "Enable/Disable");
    am.registerAction("clip.link", "Link/Unlink", "Ctrl+L");
    am.registerAction("clip.speed", "Speed/Duration...", "Ctrl+R");
    am.registerAction("clip.audio_channels", "Audio Channels...");
    am.registerAction("clip.rename", "Rename Clip...");
    am.registerAction("clip.properties", "Properties");
"""
    elif cat == "Sequence":
        cpp_code += """
    am.registerAction("sequence.new", "New Sequence...");
    am.registerAction("sequence.settings", "Sequence Settings...");
    am.registerAction("sequence.render_in_out", "Render In to Out", "Enter");
    am.registerAction("sequence.add_tracks", "Add Tracks");
    am.registerAction("sequence.delete_empty_tracks", "Delete Empty Tracks");
    am.registerAction("sequence.apply_default_transitions", "Apply Default Transitions", "Ctrl+D");
"""
    elif cat == "View":
        cpp_code += """
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
"""
    elif cat == "Tool":
        cpp_code += """
    am.registerAction("tool.selection", "Selection Tool", "V");
    am.registerAction("tool.razor", "Razor Tool", "C");
    am.registerAction("tool.ripple", "Ripple Edit Tool", "B");
    am.registerAction("tool.pen", "Pen Tool", "P");
"""
    elif cat == "Help":
        cpp_code += """
    am.registerAction("help.docs", "Documentation / User Manual");
    am.registerAction("help.shortcuts", "Keyboard Shortcuts Guide");
    am.registerAction("help.updates", "Check for Updates...");
    am.registerAction("help.about", "About Video Studio Pro");
"""

    cpp_code += "}\n"
    
    files[f"core/actions/{cat}Actions.h"] = h_code
    files[f"core/actions/{cat}Actions.cpp"] = cpp_code

# Write files
os.makedirs("core/actions", exist_ok=True)
for path, content in files.items():
    with open(path, "w") as f:
        f.write(content)

print("Files generated successfully!")
