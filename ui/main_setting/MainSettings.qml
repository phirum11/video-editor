pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Popup {
    id: settingsRoot
    modal: true
    dim: true

    readonly property color panelBg: Theme.background
    readonly property color sidebarBg: Theme.surfaceInset
    readonly property color headerBg: Theme.background
    readonly property color cardBg: Theme.surfaceRaised
    readonly property color borderColor: Theme.divider
    readonly property color textPrimary: "#dce5e8"
    readonly property color textMuted: "#8fa2aa"
    readonly property color accent: "#58a8d8"
    readonly property color accentDim: "#2a5a7a"
    readonly property color dangerRed: "#e04050"

    property int activeSection: 0

    Overlay.modal: Rectangle {
        color: "#80000000"
    }

    background: Rectangle {
        color: settingsRoot.panelBg
        border.color: settingsRoot.borderColor
        border.width: 1
        radius: 8
    }

    padding: 0

    contentItem: RowLayout {
        spacing: 0

        // ─── Left Sidebar ───
        Rectangle {
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: settingsRoot.sidebarBg
            radius: 8

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 8
                color: settingsRoot.sidebarBg
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 16
                spacing: 2

                Text {
                    Layout.leftMargin: 16
                    Layout.bottomMargin: 10
                    text: "Settings"
                    color: settingsRoot.textPrimary
                    font.pixelSize: 18
                    font.weight: Font.Bold
                }

                SidebarItem { label: "General";        iconSource: "qrc:/VideoStudioUI/assets/settings.svg";     sectionIndex: 0; active: settingsRoot.activeSection === 0 }
                SidebarItem { label: "Appearance";     iconSource: "qrc:/VideoStudioUI/assets/panel.svg";        sectionIndex: 1; active: settingsRoot.activeSection === 1 }
                SidebarItem { label: "Timeline";       iconSource: "qrc:/VideoStudioUI/assets/film.svg";         sectionIndex: 2; active: settingsRoot.activeSection === 2 }
                SidebarItem { label: "Playback";       iconSource: "qrc:/VideoStudioUI/assets/monitor-play.svg"; sectionIndex: 3; active: settingsRoot.activeSection === 3 }
                SidebarItem { label: "Audio";          iconSource: "qrc:/VideoStudioUI/assets/mic.svg";          sectionIndex: 4; active: settingsRoot.activeSection === 4 }
                SidebarItem { label: "Media Cache";    iconSource: "qrc:/VideoStudioUI/assets/folder.svg";       sectionIndex: 5; active: settingsRoot.activeSection === 5 }
                SidebarItem { label: "Keyboard";       iconSource: "qrc:/VideoStudioUI/assets/keyboard.svg";     sectionIndex: 6; active: settingsRoot.activeSection === 6 }

                Item { Layout.fillHeight: true }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    height: 1
                    color: settingsRoot.borderColor
                }

                Text {
                    Layout.leftMargin: 16
                    Layout.topMargin: 8
                    Layout.bottomMargin: 12
                    text: "Video Studio Pro v1.0"
                    color: settingsRoot.textMuted
                    font.pixelSize: 11
                }
            }
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: settingsRoot.borderColor
        }

        // ─── Right Content ───
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Header
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: settingsRoot.headerBg
                radius: 8
                
                Rectangle {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 8
                    color: settingsRoot.headerBg
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 12

                    Text {
                        text: {
                            var titles = ["General", "Appearance", "Timeline", "Playback", "Audio", "Media Cache", "Keyboard"]
                            return titles[settingsRoot.activeSection]
                        }
                        color: settingsRoot.textPrimary
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }

                    Item { Layout.fillWidth: true }

                    AbstractButton {
                        id: closeBtn
                        implicitWidth: 28
                        implicitHeight: 28
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 4
                            color: closeBtn.hovered ? Theme.surfaceHover : "transparent"
                        }

                        contentItem: Text {
                            text: "✕"
                            color: closeBtn.hovered ? settingsRoot.textPrimary : settingsRoot.textMuted
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: settingsRoot.close()
                    }
                }
            }

            // Content Area
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    Item { Layout.preferredHeight: 8 }

                    // ─── General Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 0
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Project Defaults"
                            SettingsRow { label: "Default Location"; value: "C:\\Users\\Videos\\Projects"; isPath: true }
                            SettingsRow { label: "Auto-Save Interval"; value: "5 minutes" }
                            SettingsRow { label: "Undo History Limit"; value: "50 steps" }
                        }

                        SettingsCard {
                            title: "Startup"
                            SettingsToggleRow { label: "Show Welcome Screen"; checked: true }
                            SettingsToggleRow { label: "Load Last Project on Start"; checked: false }
                            SettingsToggleRow { label: "Check for Updates Automatically"; checked: true }
                        }
                    }

                    // ─── Appearance Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 1
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Theme"
                            SettingsRow { label: "Color Theme"; value: "Dark (Default)" }
                            SettingsRow { label: "Accent Color"; value: "#58a8d8" }
                            SettingsRow { label: "UI Scale"; value: "100%" }
                        }

                        SettingsCard {
                            title: "Fonts"
                            SettingsRow { label: "Interface Font"; value: "System Default" }
                            SettingsRow { label: "Monospace Font"; value: "Consolas" }
                            SettingsRow { label: "Font Size"; value: "13px" }
                        }
                    }

                    // ─── Timeline Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 2
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Timeline Behavior"
                            SettingsToggleRow { label: "Snap to Timeline"; checked: true }
                            SettingsToggleRow { label: "Ripple Edit by Default"; checked: false }
                            SettingsToggleRow { label: "Auto-select Clips Under Playhead"; checked: true }
                            SettingsRow { label: "Default Track Height"; value: "Medium" }
                        }

                        SettingsCard {
                            title: "Sequence Defaults"
                            SettingsRow { label: "Frame Rate"; value: "24 fps" }
                            SettingsRow { label: "Resolution"; value: "1920 × 1080" }
                            SettingsRow { label: "Pixel Aspect Ratio"; value: "Square Pixels (1.0)" }
                        }
                    }

                    // ─── Playback Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 3
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Playback Performance"
                            SettingsRow { label: "Playback Resolution"; value: "Full" }
                            SettingsRow { label: "Video Renderer"; value: "Mercury GPU Acceleration" }
                            SettingsToggleRow { label: "Drop Frames to Maintain Realtime"; checked: true }
                            SettingsToggleRow { label: "Pre-roll Before Playback"; checked: false }
                        }
                    }

                    // ─── Audio Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 4
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Audio Hardware"
                            SettingsRow { label: "Default Audio Device"; value: "System Default" }
                            SettingsRow { label: "Sample Rate"; value: "48000 Hz" }
                            SettingsRow { label: "Buffer Size"; value: "1024 samples" }
                        }

                        SettingsCard {
                            title: "Audio Monitoring"
                            SettingsToggleRow { label: "Mute While Scrubbing"; checked: false }
                            SettingsToggleRow { label: "Audio Peak Hold"; checked: true }
                        }
                    }

                    // ─── Media Cache Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 5
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Cache Location"
                            SettingsRow { label: "Media Cache Path"; value: "C:\\Users\\AppData\\Local\\VideoStudio\\Cache"; isPath: true }
                            SettingsRow { label: "Current Cache Size"; value: "0 MB" }
                        }

                        SettingsCard {
                            title: "Cache Management"
                            SettingsToggleRow { label: "Auto-Delete Old Cache Files"; checked: true }
                            SettingsRow { label: "Maximum Cache Size"; value: "10 GB" }
                        }
                    }

                    // ─── Keyboard Section ───
                    ColumnLayout {
                        visible: settingsRoot.activeSection === 6
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 16

                        SettingsCard {
                            title: "Keyboard Shortcuts"
                            SettingsRow { label: "Preset"; value: "Video Studio Pro (Default)" }
                        }

                        SettingsCard {
                            title: "Common Shortcuts"
                            ShortcutRow { action: "Play / Pause"; shortcut: "Space" }
                            ShortcutRow { action: "Cut at Playhead"; shortcut: "Ctrl+K" }
                            ShortcutRow { action: "Razor Tool"; shortcut: "C" }
                            ShortcutRow { action: "Selection Tool"; shortcut: "V" }
                            ShortcutRow { action: "Undo"; shortcut: "Ctrl+Z" }
                            ShortcutRow { action: "Redo"; shortcut: "Ctrl+Shift+Z" }
                            ShortcutRow { action: "Export"; shortcut: "Ctrl+M" }
                            ShortcutRow { action: "Save"; shortcut: "Ctrl+S" }
                        }
                    }

                    Item { Layout.preferredHeight: 16 }
                }
            }
        }
    }

    // ─── Inline Components ───

    component SidebarItem: AbstractButton {
        id: sidebarItem
        property string label: ""
        property url iconSource: ""
        property int sectionIndex: 0
        property bool active: false

        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.rightMargin: 8
        implicitHeight: 34
        hoverEnabled: true

        background: Rectangle {
            radius: 6
            color: sidebarItem.active ? settingsRoot.accentDim : (sidebarItem.hovered ? Theme.surfaceHover : "transparent")
            border.color: sidebarItem.active ? settingsRoot.accent : "transparent"
            border.width: sidebarItem.active ? 1 : 0
            opacity: sidebarItem.active ? 0.5 : 1.0
        }

        contentItem: RowLayout {
            spacing: 10
            anchors.leftMargin: 10

            Image {
                source: sidebarItem.iconSource
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                sourceSize: Qt.size(16, 16)
                fillMode: Image.PreserveAspectFit
                opacity: sidebarItem.active ? 1.0 : (sidebarItem.hovered ? 0.8 : 0.5)
            }

            Text {
                text: sidebarItem.label
                color: sidebarItem.active ? settingsRoot.accent : (sidebarItem.hovered ? settingsRoot.textPrimary : settingsRoot.textMuted)
                font.pixelSize: 13
                font.weight: sidebarItem.active ? Font.DemiBold : Font.Normal
            }
        }

        onClicked: settingsRoot.activeSection = sidebarItem.sectionIndex
    }

    component SettingsCard: Rectangle {
        id: card
        property string title: ""
        default property list<Item> innerRows

        Layout.fillWidth: true
        implicitHeight: cardLayout.implicitHeight + 28
        color: settingsRoot.cardBg
        border.color: settingsRoot.borderColor
        border.width: 1
        radius: 6

        ColumnLayout {
            id: cardLayout
            y: 14
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 16

            Text {
                text: card.title
                color: settingsRoot.textPrimary
                font.pixelSize: 14
                font.weight: Font.DemiBold
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                children: card.innerRows
            }
        }
    }

    component SettingsRow: RowLayout {
        id: settingsRowRoot
        property string label: ""
        property string value: ""
        property bool isPath: false

        Layout.fillWidth: true
        Layout.preferredHeight: 32
        spacing: 16

        Text {
            Layout.preferredWidth: 200
            text: settingsRowRoot.label
            color: settingsRoot.textMuted
            font.pixelSize: 13
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: Theme.surfaceInset
            border.color: settingsRoot.borderColor
            border.width: 1
            radius: 4

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: settingsRowRoot.value
                color: settingsRoot.textPrimary
                font.pixelSize: 12
                font.family: settingsRowRoot.isPath ? "Consolas" : ""
                elide: Text.ElideMiddle
                width: parent.width - 20
            }
        }
    }

    component SettingsToggleRow: RowLayout {
        id: settingsToggleRowRoot
        property string label: ""
        property bool checked: false

        Layout.fillWidth: true
        Layout.preferredHeight: 32
        spacing: 16

        Text {
            Layout.fillWidth: true
            text: settingsToggleRowRoot.label
            color: settingsRoot.textMuted
            font.pixelSize: 13
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 20
            radius: 10
            color: settingsToggleRowRoot.checked ? settingsRoot.accent : "#2a3640"
            border.color: settingsToggleRowRoot.checked ? settingsRoot.accent : settingsRoot.borderColor
            border.width: 1

            Rectangle {
                x: settingsToggleRowRoot.checked ? parent.width - width - 2 : 2
                anchors.verticalCenter: parent.verticalCenter
                width: 16
                height: 16
                radius: 8
                color: "#ffffff"

                Behavior on x {
                    NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: settingsToggleRowRoot.checked = !settingsToggleRowRoot.checked
            }
        }
    }

    component ShortcutRow: RowLayout {
        id: shortcutRowRoot
        property string action: ""
        property string shortcut: ""

        Layout.fillWidth: true
        Layout.preferredHeight: 28
        spacing: 16

        Text {
            Layout.fillWidth: true
            text: shortcutRowRoot.action
            color: settingsRoot.textMuted
            font.pixelSize: 13
        }

        Rectangle {
            Layout.preferredWidth: shortcutLabel.implicitWidth + 16
            Layout.preferredHeight: 22
            radius: 4
            color: Theme.surfaceInset
            border.color: settingsRoot.borderColor
            border.width: 1

            Text {
                id: shortcutLabel
                anchors.centerIn: parent
                text: shortcutRowRoot.shortcut
                color: settingsRoot.accent
                font.pixelSize: 12
                font.family: "Consolas"
            }
        }
    }
}
