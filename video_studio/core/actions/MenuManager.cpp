#include "MenuManager.h"
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
        createSubMenu("Layout", {
            createMenuItem("view.layout.default"),
            createMenuItem("view.layout.vertical")
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
