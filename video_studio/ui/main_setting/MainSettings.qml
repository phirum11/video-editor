pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import VideoStudioUI
import "components"

Popup {
    id: settingsRoot
    modal: true
    dim: true

    property color panelBg: Theme.background
    property color sidebarBg: Theme.surfaceInset
    property color headerBg: Theme.background
    property color cardBg: Theme.surfaceRaised
    property color cardBorder: Qt.rgba(Theme.divider.r, Theme.divider.g, Theme.divider.b, 0.5)
    property color textPrimary: Theme.text
    property color textMuted: Theme.textMuted
    property color accent: Theme.accent
    property color accentDim: "#1a4a6a"
    property color dangerRed: "#e04050"

    property int activeSection: 0

    Overlay.modal: Rectangle {
        color: "#99000000"
        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
    }

    background: Rectangle {
        color: settingsRoot.panelBg
        border.color: Theme.divider
        border.width: 1
        radius: 12

        // Subtle drop shadow effect using nested rectangles
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            z: -1
            color: "transparent"
            border.color: "#30000000"
            border.width: 1
            radius: 13
        }
    }

    padding: 0

    contentItem: RowLayout {
        spacing: 0

        // ─── Left Sidebar ───
        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: settingsRoot.sidebarBg

            // Left side rounded corners
            layer.enabled: true
            radius: 12
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 12
                color: settingsRoot.sidebarBg
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 24
                spacing: 4

                Text {
                    Layout.leftMargin: 20
                    Layout.bottomMargin: 16
                    text: "Settings"
                    color: settingsRoot.textPrimary
                    font.pixelSize: 20
                    font.weight: Font.Bold
                }

                SidebarItem {
                    label: "General"
                    iconSource: "qrc:/VideoStudioUI/assets/settings.svg"
                    sectionIndex: 0
                    active: settingsRoot.activeSection === 0
                }
                SidebarItem {
                    label: "Appearance"
                    iconSource: "qrc:/VideoStudioUI/assets/panel.svg"
                    sectionIndex: 1
                    active: settingsRoot.activeSection === 1
                }
                SidebarItem {
                    label: "Timeline"
                    iconSource: "qrc:/VideoStudioUI/assets/film.svg"
                    sectionIndex: 2
                    active: settingsRoot.activeSection === 2
                }
                SidebarItem {
                    label: "Playback"
                    iconSource: "qrc:/VideoStudioUI/assets/monitor-play.svg"
                    sectionIndex: 3
                    active: settingsRoot.activeSection === 3
                }
                SidebarItem {
                    label: "Audio"
                    iconSource: "qrc:/VideoStudioUI/assets/mic.svg"
                    sectionIndex: 4
                    active: settingsRoot.activeSection === 4
                }
                SidebarItem {
                    label: "Media Cache"
                    iconSource: "qrc:/VideoStudioUI/assets/folder.svg"
                    sectionIndex: 5
                    active: settingsRoot.activeSection === 5
                }
                SidebarItem {
                    label: "Keyboard"
                    iconSource: "qrc:/VideoStudioUI/assets/keyboard.svg"
                    sectionIndex: 6
                    active: settingsRoot.activeSection === 6
                }

                Item {
                    Layout.fillHeight: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    height: 1
                    color: settingsRoot.cardBorder
                }

                Text {
                    Layout.leftMargin: 20
                    Layout.topMargin: 12
                    Layout.bottomMargin: 20
                    text: "Video Studio Pro v1.0"
                    color: settingsRoot.textMuted
                    font.pixelSize: 11
                }
            }
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: Theme.divider
        }

        // ─── Right Content ───
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Header
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 32
                    anchors.rightMargin: 24

                    Text {
                        text: {
                            var titles = ["General", "Appearance", "Timeline", "Playback", "Audio", "Media Cache", "Keyboard"];
                            return titles[settingsRoot.activeSection];
                        }
                        color: settingsRoot.textPrimary
                        font.pixelSize: 18
                        font.weight: Font.DemiBold
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    AbstractButton {
                        id: closeBtn
                        implicitWidth: 32
                        implicitHeight: 32
                        hoverEnabled: true

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Rectangle {
                            radius: 16
                            color: closeBtn.hovered ? Theme.surfaceHover : "transparent"
                            Behavior on color {
                                ColorAnimation {
                                    duration: 150
                                }
                            }
                        }

                        contentItem: Text {
                            text: "✕"
                            color: closeBtn.hovered ? settingsRoot.textPrimary : settingsRoot.textMuted
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: settingsRoot.close()
                    }
                }
            }

            // Content Area
            ScrollView {
                id: settingsScrollView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                Column {
                    width: settingsScrollView.availableWidth
                    spacing: 24
                    padding: 32
                    topPadding: 8

                    // ─── General Section ───
                    Column {
                        visible: settingsRoot.activeSection === 0
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Project Defaults"
                            SettingsRow {
                                label: "Default Location"
                                value: "C:\\Users\\Videos\\Projects"
                                isPath: true
                            }
                            SettingsRow {
                                label: "Auto-Save Interval"
                                value: "5 minutes"
                            }
                            SettingsRow {
                                label: "Undo History Limit"
                                value: "50 steps"
                            }
                        }

                        SettingsCard {
                            title: "Startup"
                            SettingsToggleRow {
                                label: "Show Welcome Screen"
                                checked: true
                            }
                            SettingsToggleRow {
                                label: "Load Last Project on Start"
                                checked: false
                            }
                            SettingsToggleRow {
                                label: "Check for Updates Automatically"
                                checked: true
                            }
                        }
                    }

                    // ─── Appearance Section ───
                    Column {
                        visible: settingsRoot.activeSection === 1
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Theme"
                            SettingsComboBoxRow {
                                label: "Color Theme"
                                model: ["Dark (Default)", "Light", "System Default"]
                                currentIndex: Theme.isDarkMode ? 0 : 1
                                onCurrentIndexChanged: {
                                    if (currentIndex === 0 || currentIndex === 2) {
                                        Theme.isDarkMode = true;
                                    } else if (currentIndex === 1) {
                                        Theme.isDarkMode = false;
                                    }
                                }
                            }
                            SettingsColorRow {
                                label: "Accent Color"
                                colorValue: "#58a8d8"
                            }
                            SettingsComboBoxRow {
                                label: "UI Scale"
                                model: ["75%", "100%", "125%", "150%"]
                                currentIndex: 1
                            }
                        }

                        SettingsCard {
                            title: "Fonts"
                            SettingsComboBoxRow {
                                label: "Interface Font"
                                model: ["System Default", "Inter", "Roboto", "Segoe UI"]
                                currentIndex: 0
                            }
                            SettingsComboBoxRow {
                                label: "Monospace Font"
                                model: ["Consolas", "Fira Code", "Courier New"]
                                currentIndex: 0
                            }
                            SettingsComboBoxRow {
                                label: "Font Size"
                                model: ["11px", "12px", "13px", "14px", "15px"]
                                currentIndex: 2
                            }
                        }
                    }

                    // ─── Timeline Section ───
                    Column {
                        visible: settingsRoot.activeSection === 2
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Timeline Behavior"
                            SettingsToggleRow {
                                label: "Snap to Timeline"
                                checked: true
                            }
                            SettingsToggleRow {
                                label: "Ripple Edit by Default"
                                checked: false
                            }
                            SettingsToggleRow {
                                label: "Auto-select Clips Under Playhead"
                                checked: true
                            }
                            SettingsRow {
                                label: "Default Track Height"
                                value: "Medium"
                            }
                        }

                        SettingsCard {
                            title: "Sequence Defaults"
                            SettingsRow {
                                label: "Frame Rate"
                                value: "24 fps"
                            }
                            SettingsRow {
                                label: "Resolution"
                                value: "1920 × 1080"
                            }
                            SettingsRow {
                                label: "Pixel Aspect Ratio"
                                value: "Square Pixels (1.0)"
                            }
                        }
                    }
                    Column {
                        visible: settingsRoot.activeSection === 3
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Playback Performance"
                            SettingsRow {
                                label: "Playback Resolution"
                                value: "Full"
                            }
                            SettingsRow {
                                label: "Video Renderer"
                                value: "Mercury GPU Acceleration"
                            }
                            SettingsToggleRow {
                                label: "Drop Frames to Maintain Realtime"
                                checked: true
                            }
                            SettingsToggleRow {
                                label: "Pre-roll Before Playback"
                                checked: false
                            }
                        }
                    }
                    Column {
                        visible: settingsRoot.activeSection === 4
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Audio Hardware"
                            SettingsRow {
                                label: "Default Audio Device"
                                value: "System Default"
                            }
                            SettingsRow {
                                label: "Sample Rate"
                                value: "48000 Hz"
                            }
                            SettingsRow {
                                label: "Buffer Size"
                                value: "1024 samples"
                            }
                        }

                        SettingsCard {
                            title: "Audio Monitoring"
                            SettingsToggleRow {
                                label: "Mute While Scrubbing"
                                checked: false
                            }
                            SettingsToggleRow {
                                label: "Audio Peak Hold"
                                checked: true
                            }
                        }
                    }
                    Column {
                        visible: settingsRoot.activeSection === 5
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Cache Location"
                            SettingsRow {
                                label: "Media Cache Path"
                                value: "C:\\Users\\AppData\\Local\\VideoStudio\\Cache"
                                isPath: true
                            }
                            SettingsRow {
                                label: "Current Cache Size"
                                value: "0 MB"
                            }
                        }

                        SettingsCard {
                            title: "Cache Management"
                            SettingsToggleRow {
                                label: "Auto-Delete Old Cache Files"
                                checked: true
                            }
                            SettingsRow {
                                label: "Maximum Cache Size"
                                value: "10 GB"
                            }
                        }
                    }
                    Column {
                        visible: settingsRoot.activeSection === 6
                        width: parent.width - 64
                        spacing: 24

                        SettingsCard {
                            title: "Keyboard Shortcuts"
                            SettingsRow {
                                label: "Preset"
                                value: "Video Studio Pro (Default)"
                            }
                        }

                        SettingsCard {
                            title: "Common Shortcuts"
                            ShortcutRow {
                                action: "Play / Pause"
                                shortcut: "Space"
                            }
                            ShortcutRow {
                                action: "Cut at Playhead"
                                shortcut: "Ctrl+K"
                            }
                            ShortcutRow {
                                action: "Razor Tool"
                                shortcut: "C"
                            }
                            ShortcutRow {
                                action: "Selection Tool"
                                shortcut: "V"
                            }
                            ShortcutRow {
                                action: "Undo"
                                shortcut: "Ctrl+Z"
                            }
                            ShortcutRow {
                                action: "Redo"
                                shortcut: "Ctrl+Shift+Z"
                            }
                            ShortcutRow {
                                action: "Export"
                                shortcut: "Ctrl+M"
                            }
                            ShortcutRow {
                                action: "Save"
                                shortcut: "Ctrl+S"
                            }
                        }
                    }
                }
            }
        }
    }
}
