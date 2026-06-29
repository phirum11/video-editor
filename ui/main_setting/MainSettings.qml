// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import VideoStudioUI
import VideoStudio.Core
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
                                value: SettingsController.general.defaultLocation
                                isPath: true
                                onValueChanged: SettingsController.general.defaultLocation = value
                            }
                            SettingsRow {
                                label: "Auto-Save Interval"
                                value: SettingsController.general.autoSaveInterval
                                onValueChanged: SettingsController.general.autoSaveInterval = value
                            }
                            SettingsRow {
                                label: "Undo History Limit"
                                value: SettingsController.general.undoHistoryLimit
                                onValueChanged: SettingsController.general.undoHistoryLimit = value
                            }
                        }

                        SettingsCard {
                            title: "Startup"
                            SettingsToggleRow {
                                label: "Show Welcome Screen"
                                checked: SettingsController.general.showWelcomeScreen
                                onCheckedChanged: SettingsController.general.showWelcomeScreen = checked
                            }
                            SettingsToggleRow {
                                label: "Load Last Project on Start"
                                checked: SettingsController.general.loadLastProjectOnStart
                                onCheckedChanged: SettingsController.general.loadLastProjectOnStart = checked
                            }
                            SettingsToggleRow {
                                label: "Check for Updates Automatically"
                                checked: SettingsController.general.checkForUpdatesAutomatically
                                onCheckedChanged: SettingsController.general.checkForUpdatesAutomatically = checked
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
                                currentIndex: SettingsController.appearance.colorTheme
                                onCurrentIndexChanged: {
                                    SettingsController.appearance.colorTheme = currentIndex;
                                    if (currentIndex === 0 || currentIndex === 2) {
                                        Theme.isDarkMode = true;
                                    } else if (currentIndex === 1) {
                                        Theme.isDarkMode = false;
                                    }
                                }
                            }
                            SettingsColorRow {
                                label: "Accent Color"
                                colorValue: SettingsController.appearance.accentColor
                            }
                            SettingsComboBoxRow {
                                label: "UI Scale"
                                model: ["75%", "100%", "125%", "150%"]
                                currentIndex: SettingsController.appearance.uiScale
                                onCurrentIndexChanged: SettingsController.appearance.uiScale = currentIndex
                            }
                        }

                        SettingsCard {
                            title: "Fonts"
                            SettingsComboBoxRow {
                                label: "Interface Font"
                                model: ["System Default", "Inter", "Roboto", "Segoe UI"]
                                currentIndex: SettingsController.appearance.interfaceFont
                                onCurrentIndexChanged: SettingsController.appearance.interfaceFont = currentIndex
                            }
                            SettingsComboBoxRow {
                                label: "Monospace Font"
                                model: ["Consolas", "Fira Code", "Courier New"]
                                currentIndex: SettingsController.appearance.monospaceFont
                                onCurrentIndexChanged: SettingsController.appearance.monospaceFont = currentIndex
                            }
                            SettingsComboBoxRow {
                                label: "Font Size"
                                model: ["11px", "12px", "13px", "14px", "15px"]
                                currentIndex: SettingsController.appearance.fontSize
                                onCurrentIndexChanged: SettingsController.appearance.fontSize = currentIndex
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
                                checked: SettingsController.timeline.snapToTimeline
                                onCheckedChanged: SettingsController.timeline.snapToTimeline = checked
                            }
                            SettingsToggleRow {
                                label: "Ripple Edit by Default"
                                checked: SettingsController.timeline.rippleEditByDefault
                                onCheckedChanged: SettingsController.timeline.rippleEditByDefault = checked
                            }
                            SettingsToggleRow {
                                label: "Auto-select Clips Under Playhead"
                                checked: SettingsController.timeline.autoSelectClipsUnderPlayhead
                                onCheckedChanged: SettingsController.timeline.autoSelectClipsUnderPlayhead = checked
                            }
                            SettingsRow {
                                label: "Default Track Height"
                                value: SettingsController.timeline.defaultTrackHeight
                                onValueChanged: SettingsController.timeline.defaultTrackHeight = value
                            }
                        }

                        SettingsCard {
                            title: "Sequence Defaults"
                            SettingsRow {
                                label: "Frame Rate"
                                value: SettingsController.timeline.frameRate
                                onValueChanged: SettingsController.timeline.frameRate = value
                            }
                            SettingsRow {
                                label: "Resolution"
                                value: SettingsController.timeline.resolution
                                onValueChanged: SettingsController.timeline.resolution = value
                            }
                            SettingsRow {
                                label: "Pixel Aspect Ratio"
                                value: SettingsController.timeline.pixelAspectRatio
                                onValueChanged: SettingsController.timeline.pixelAspectRatio = value
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
                                value: SettingsController.playback.playbackResolution
                                onValueChanged: SettingsController.playback.playbackResolution = value
                            }
                            SettingsRow {
                                label: "Video Renderer"
                                value: SettingsController.playback.videoRenderer
                                onValueChanged: SettingsController.playback.videoRenderer = value
                            }
                            SettingsToggleRow {
                                label: "Drop Frames to Maintain Realtime"
                                checked: SettingsController.playback.dropFramesToMaintainRealtime
                                onCheckedChanged: SettingsController.playback.dropFramesToMaintainRealtime = checked
                            }
                            SettingsToggleRow {
                                label: "Pre-roll Before Playback"
                                checked: SettingsController.playback.preRollBeforePlayback
                                onCheckedChanged: SettingsController.playback.preRollBeforePlayback = checked
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
                                value: SettingsController.audio.defaultAudioDevice
                                onValueChanged: SettingsController.audio.defaultAudioDevice = value
                            }
                            SettingsRow {
                                label: "Sample Rate"
                                value: SettingsController.audio.sampleRate
                                onValueChanged: SettingsController.audio.sampleRate = value
                            }
                            SettingsRow {
                                label: "Buffer Size"
                                value: SettingsController.audio.bufferSize
                                onValueChanged: SettingsController.audio.bufferSize = value
                            }
                        }

                        SettingsCard {
                            title: "Audio Monitoring"
                            SettingsToggleRow {
                                label: "Mute While Scrubbing"
                                checked: SettingsController.audio.muteWhileScrubbing
                                onCheckedChanged: SettingsController.audio.muteWhileScrubbing = checked
                            }
                            SettingsToggleRow {
                                label: "Audio Peak Hold"
                                checked: SettingsController.audio.audioPeakHold
                                onCheckedChanged: SettingsController.audio.audioPeakHold = checked
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
                                value: SettingsController.cache.mediaCachePath
                                isPath: true
                                onValueChanged: SettingsController.cache.mediaCachePath = value
                            }
                            SettingsRow {
                                label: "Current Cache Size"
                                value: SettingsController.cache.currentCacheSize
                                onValueChanged: SettingsController.cache.currentCacheSize = value
                            }
                        }

                        SettingsCard {
                            title: "Cache Management"
                            SettingsToggleRow {
                                label: "Auto-Delete Old Cache Files"
                                checked: SettingsController.cache.autoDeleteOldCacheFiles
                                onCheckedChanged: SettingsController.cache.autoDeleteOldCacheFiles = checked
                            }
                            SettingsRow {
                                label: "Maximum Cache Size"
                                value: SettingsController.cache.maximumCacheSize
                                onValueChanged: SettingsController.cache.maximumCacheSize = value
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
                                value: SettingsController.keyboard.keyboardPreset
                                onValueChanged: SettingsController.keyboard.keyboardPreset = value
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
