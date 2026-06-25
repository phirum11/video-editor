pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI
import "../dialogs"

Rectangle {
    id: headerRoot
    property var windowTarget
    property var timelineController: null
    property var subtitleController: null
    signal openSettings()
    signal saveProjectRequested()
    signal openProjectRequested()
    property color textPrimary: Theme.text
    property color textMuted: Theme.textMuted
    property color accent: Theme.accent

    implicitHeight: 38
    color: Theme.background
    clip: true

    gradient: Gradient {
        GradientStop { position: 0.0; color: Theme.background }
        GradientStop { position: 0.55; color: Theme.background }
        GradientStop { position: 1.0; color: Theme.background }
    }

    function toggleMaximized() {
        if (!headerRoot.windowTarget)
            return

        if (headerRoot.windowTarget.visibility === Window.Maximized)
            headerRoot.windowTarget.showNormal()
        else
            headerRoot.windowTarget.showMaximized()
    }

    function tryExport() {
        if (!headerRoot.timelineController || 
            !headerRoot.timelineController.clipModel || 
            headerRoot.timelineController.clipModel.rowCount() === 0) {
            emptyTimelineDialog.open()
        } else {
            exportDialog.open()
        }
    }

    Item {
        anchors.fill: parent

        MouseArea {
            id: dragArea
            property real pressX: 0
            property real pressY: 0

            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onPressed: function(mouse) {
                pressX = mouse.x
                pressY = mouse.y
            }
            onPositionChanged: function(mouse) {
                if (!dragArea.pressed || !headerRoot.windowTarget || headerRoot.windowTarget.visibility === Window.Maximized)
                    return

                headerRoot.windowTarget.x += mouse.x - dragArea.pressX
                headerRoot.windowTarget.y += mouse.y - dragArea.pressY
            }
            onDoubleClicked: headerRoot.toggleMaximized()
        }

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // 1. App Icon & Title
            RowLayout {
                Layout.leftMargin: 8
                spacing: 8

                Rectangle {
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    radius: 3
                    color: Theme.surfaceRaised
                    border.color: Theme.divider
                    border.width: 1

                    Image {
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        source: "qrc:/VideoStudioUI/assets/film.svg"
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    text: qsTr("Video Studio Pro")
                    color: headerRoot.textPrimary
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.maximumWidth: 200
                }
            }

            Item { Layout.preferredWidth: 16 } // spacing

            // 2. Menu Bar
            RowLayout {
                spacing: 2

                AbstractButton {
                    id: menuBtn
                    text: "Menu"
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 28
                    Layout.alignment: Qt.AlignVCenter
                    hoverEnabled: true

                    HoverHandler { cursorShape: Qt.PointingHandCursor }

                    contentItem: RowLayout {
                        spacing: 1
                        anchors.centerIn: parent
                        Text {
                            text: "Menu"
                            color: menuBtn.hovered || mainMenu.opened ? headerRoot.textPrimary : headerRoot.textMuted
                            font.pixelSize: 14
                        }
                        Image {
                            Layout.leftMargin: -2
                            Layout.preferredWidth: 12
                            Layout.preferredHeight: 12
                            source: mainMenu.opened ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
                            opacity: menuBtn.hovered || mainMenu.opened ? 1.0 : 0.7
                        }
                    }
                    
                    background: Rectangle {
                        radius: 3
                        color: menuBtn.hovered || mainMenu.opened ? Theme.surfaceHover : "transparent"
                        border.color: menuBtn.hovered || mainMenu.opened ? Theme.divider : "transparent"
                        border.width: 1
                    }
                    
                    onClicked: mainMenu.open()

                    Menu {
                        id: mainMenu
                        y: menuBtn.height
                        background: Rectangle {
                            implicitWidth: 260
                            color: Theme.surfaceInset
                            border.color: Theme.divider
                            border.width: 1
                            radius: 4
                        }

                        delegate: AppMenuItem { }

                        Component {
                            id: actionComp
                            AppMenuItem {
                                id: compItem
                                property string customActionId
                                // qmllint disable unqualified
                                onTriggered: {
                                    if (compItem.customActionId === "file.save") {
                                        headerRoot.saveProjectRequested()
                                    } else if (compItem.customActionId === "file.open") {
                                        headerRoot.openProjectRequested()
                                    } else if (typeof ActionManager !== "undefined") {
                                        ActionManager.executeAction(compItem.customActionId)
                                    }
                                }
                                // qmllint enable unqualified
                            }
                        }

                        Component {
                            id: subMenuComp
                            AppMenu {}
                        }

                        Component.onCompleted: {
                            function buildMenu(menuData, parentMenu) {
                                for (let i = 0; i < menuData.items.length; ++i) {
                                    let itemData = menuData.items[i];
                                    if (itemData.type === "separator") {
                                        let sep = Qt.createQmlObject('import QtQuick; import QtQuick.Controls; import VideoStudioUI; MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: Theme.divider } }', parentMenu.contentItem);
                                        parentMenu.addItem(sep);
                                    } else if (itemData.type === "action") {
                                        let action = actionComp.createObject(parentMenu.contentItem, {text: itemData.text, shortcutText: itemData.shortcut, customActionId: itemData.actionId});
                                        parentMenu.addItem(action);
                                    } else if (itemData.type === "submenu") {
                                        let subMenu = subMenuComp.createObject(parentMenu, {title: itemData.title});
                                        buildMenu(itemData, subMenu);
                                        parentMenu.addMenu(subMenu);
                                    }
                                }
                            }

                            if (typeof MenuManager !== "undefined") {
                                let structure = MenuManager.menuStructure;
                                for (let i = 0; i < structure.length; ++i) {
                                    let menuData = structure[i];
                                    let topMenu = subMenuComp.createObject(mainMenu, {title: menuData.title});
                                    buildMenu(menuData, topMenu);
                                    mainMenu.addMenu(topMenu);
                                }
                            }
                        }
                    }
                }
            }

            // 3. Filler
            Item {
                Layout.fillWidth: true
            }

            // 3.2 Isolation Progress
            RowLayout {
                Layout.rightMargin: 16
                visible: headerRoot.timelineController && headerRoot.timelineController.isIsolating
                spacing: 8

                Text {
                    text: "Isolating Voice..."
                    color: headerRoot.textMuted
                    font.pixelSize: 12
                }

                Rectangle {
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 6
                    Layout.alignment: Qt.AlignVCenter
                    color: Theme.surfaceInset
                    radius: 3

                    Rectangle {
                        height: parent.height
                        width: parent.width * (headerRoot.timelineController ? headerRoot.timelineController.activeIsolationProgress / 100.0 : 0)
                        color: "#25e5cf"
                        radius: 3
                        
                        Behavior on width {
                            NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
                        }
                    }
                }

                Text {
                    text: (headerRoot.timelineController ? headerRoot.timelineController.activeIsolationProgress : 0) + "%"
                    color: headerRoot.textPrimary
                    font.pixelSize: 12
                    font.family: "monospace"
                    Layout.preferredWidth: 30
                }
            }

            // 3.3 AI Voice Progress
            RowLayout {
                Layout.rightMargin: 16
                visible: headerRoot.timelineController && headerRoot.timelineController.isGeneratingAIVoice
                spacing: 8

                Text {
                    text: "Generating Voice..."
                    color: headerRoot.textMuted
                    font.pixelSize: 12
                }

                Rectangle {
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 6
                    Layout.alignment: Qt.AlignVCenter
                    color: Theme.surfaceInset
                    radius: 3

                    Rectangle {
                        height: parent.height
                        width: parent.width * (headerRoot.timelineController ? headerRoot.timelineController.activeAIVoiceProgress / 100.0 : 0)
                        color: "#ff66cc" // a different color to distinguish
                        radius: 3 
                        Behavior on width {
                            NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
                        }
                    }
                }

                Text {
                    text: (headerRoot.timelineController ? headerRoot.timelineController.activeAIVoiceProgress : 0) + "%"
                    color: headerRoot.textPrimary
                    font.pixelSize: 12
                    font.family: "monospace"
                    Layout.preferredWidth: 30
                }
            }

            // 3.5 Export Button
            AbstractButton {
                id: headerExportBtn
                text: "Export"
                Layout.preferredWidth: 64
                Layout.preferredHeight: 26
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 12
                hoverEnabled: true

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                contentItem: Text {
                    text: headerExportBtn.text
                    color: "#000000"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                background: Rectangle {
                    color: headerExportBtn.hovered ? "#38ffe8" : "#25e5cf"
                    radius: 4
                }
                
                onClicked: headerRoot.tryExport()
            }

            // 4. Settings Button
            AbstractButton {
                id: settingsOpenBtn
                Layout.preferredWidth: 30
                Layout.preferredHeight: 28
                Layout.alignment: Qt.AlignVCenter
                hoverEnabled: true

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                ToolTip.visible: hovered
                ToolTip.text: "Settings"

                background: Rectangle {
                    radius: 4
                    color: settingsOpenBtn.hovered ? Theme.surfaceHover : "transparent"
                    border.color: settingsOpenBtn.hovered ? Theme.divider : "transparent"
                    border.width: 1
                }

                contentItem: Item {
                    Image {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        sourceSize: Qt.size(16, 16)
                        source: "qrc:/VideoStudioUI/assets/settings.svg"
                        opacity: settingsOpenBtn.hovered ? 1.0 : 0.7
                        fillMode: Image.PreserveAspectFit
                    }
                }

                onClicked: headerRoot.openSettings()
            }

            Item { Layout.preferredWidth: 8 } // spacing

            // 5. Window Controls
            RowLayout {
                spacing: 0
                Layout.alignment: Qt.AlignTop

                WindowControlButton {
                    mode: "minimize"
                    onClicked: if (headerRoot.windowTarget) headerRoot.windowTarget.showMinimized()
                }

                WindowControlButton {
                    mode: "maximize"
                    onClicked: headerRoot.toggleMaximized()
                }

                WindowControlButton {
                    mode: "close"
                    danger: true
                    onClicked: if (headerRoot.windowTarget) headerRoot.windowTarget.close()
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.divider
        opacity: 0.9
    }

    ExportDialog {
        id: exportDialog
        parent: Overlay.overlay
        timelineController: headerRoot.timelineController
        subtitleController: headerRoot.subtitleController
    }

    EmptyTimelineDialog {
        id: emptyTimelineDialog
    }

    component AppMenu: Menu {
        id: customMenu
        background: Rectangle {
            implicitWidth: 260
            color: Theme.surfaceInset
            border.color: Theme.divider
            border.width: 1
            radius: 4
        }
        
        // Define an arrow icon for submenus
        delegate: AppMenuItem { }
    }
    
    component AppMenuItem: MenuItem {
        id: menuItem
        property string shortcutText: ""
        
        implicitWidth: 260
        implicitHeight: 28
        
        background: Rectangle {
            color: menuItem.highlighted ? "#286082" : "transparent"
            radius: 3
            anchors.fill: parent
            anchors.margins: 2
        }
        
        contentItem: Item {
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                text: menuItem.text
                color: menuItem.highlighted ? Theme.text : Theme.textMuted
                font.pixelSize: 13
            }
            Text {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: menuItem.shortcutText
                color: menuItem.highlighted ? "#bbdff5" : "#6c8491"
                font.pixelSize: 12
            }
        }
        
        arrow: Text {
            x: menuItem.width - width - 12
            anchors.verticalCenter: parent.verticalCenter
            visible: menuItem.subMenu
            text: ">"
            color: menuItem.highlighted ? Theme.text : "#89979d"
            font.pixelSize: 12
            font.family: "monospace"
        }
    }

    component ActionChip: AbstractButton {
        id: actionChip
        implicitWidth: chipLabel.implicitWidth + 24
        implicitHeight: 26
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        background: Rectangle {
            radius: 3
            color: actionChip.hovered ? Theme.surfaceHover : Theme.surfaceRaised
            border.color: actionChip.hovered ? Theme.divider : Theme.dividerSoft
            border.width: 1
        }

        contentItem: Text {
            id: chipLabel
            text: actionChip.text
            color: actionChip.hovered ? headerRoot.textPrimary : "#c5ced2"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component IconTextButton: AbstractButton {
        id: iconButton
        property url iconSource

        implicitWidth: iconRow.implicitWidth + 14
        implicitHeight: 28
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        background: Rectangle {
            radius: 3
            color: iconButton.hovered ? Theme.surfaceHover : "transparent"
            border.color: iconButton.hovered ? Theme.divider : "transparent"
            border.width: 1
        }

        contentItem: RowLayout {
            id: iconRow
            spacing: 6

            Image {
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                source: iconButton.iconSource
                fillMode: Image.PreserveAspectFit
                opacity: iconButton.hovered ? 1 : 0.82
            }

            Text {
                text: iconButton.text
                color: iconButton.hovered ? headerRoot.textPrimary : headerRoot.textMuted
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    component WindowControlButton: AbstractButton {
        id: windowButton
        property string mode: "minimize"
        property bool danger: false

        implicitWidth: 46
        implicitHeight: 29
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        background: Rectangle {
            color: windowButton.danger && windowButton.hovered ? "#e81123" :
                   windowButton.danger && windowButton.pressed ? "#f1707a" :
                   windowButton.pressed ? Theme.surfacePressed :
                   windowButton.hovered ? Theme.surfaceHover : "transparent"
        }

        contentItem: Item {
            // Minimize icon
            Rectangle {
                visible: windowButton.mode === "minimize"
                width: 10
                height: 1
                color: windowButton.hovered && windowButton.danger ? Theme.text : headerRoot.textMuted
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 0
            }

            // Maximize icon
            Rectangle {
                visible: windowButton.mode === "maximize"
                width: 10
                height: 10
                color: "transparent"
                border.color: windowButton.hovered && windowButton.danger ? Theme.text : headerRoot.textMuted
                border.width: 1
                anchors.centerIn: parent
            }

            // Close icon
            Item {
                visible: windowButton.mode === "close"
                width: 10
                height: 10
                anchors.centerIn: parent
                
                Rectangle {
                    width: Math.sqrt(200) // diagonal of 10x10
                    height: 1
                    color: windowButton.hovered ? Theme.text : headerRoot.textMuted
                    anchors.centerIn: parent
                    rotation: 45
                    antialiasing: true
                }
                Rectangle {
                    width: Math.sqrt(200)
                    height: 1
                    color: windowButton.hovered ? Theme.text : headerRoot.textMuted
                    anchors.centerIn: parent
                    rotation: -45
                    antialiasing: true
                }
            }
        }
    }
}
