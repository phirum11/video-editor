import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI
import "header"
import "inspector"
import "media_pool"
import "monitors"
import "timeline_view"
import "main_setting"

Window {
    id: mainWindow
    width: 1280
    height: 720
    minimumWidth: 900
    minimumHeight: 560
    visible: true
    title: qsTr("Video Studio")
    color: "#000000"
    flags: Qt.Window | Qt.FramelessWindowHint
    
    property bool isVerticalLayout: false

    Connections {
        target: typeof ActionManager !== "undefined" ? ActionManager : null
        function onActionTriggered(id) {
            if (id === "view.layout.default") {
                mainWindow.isVerticalLayout = false
            } else if (id === "view.layout.vertical") {
                mainWindow.isVerticalLayout = true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        HeaderMenu {
            Layout.fillWidth: true
            windowTarget: mainWindow
            timelineController: timelineView.timelineController
            subtitleController: subtitleCtrl
            onOpenSettings: mainSettingsDialog.open()
        }

        SplitView {
            id: workspace
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: mainWindow.isVerticalLayout ? Qt.Horizontal : Qt.Vertical

            handle: Rectangle {
                implicitWidth: 4
                implicitHeight: 4
                color: SplitHandle.pressed ? Theme.surfacePressed : (SplitHandle.hovered ? Theme.surfaceHover : "#000000")
            }

            // Block A (Top in Default, Left in Vertical)
            SplitView {
                id: blockA
                orientation: mainWindow.isVerticalLayout ? Qt.Vertical : Qt.Horizontal
                SplitView.preferredHeight: mainWindow.isVerticalLayout ? undefined : mainWindow.height * 0.55
                SplitView.preferredWidth: mainWindow.isVerticalLayout ? Math.round(mainWindow.width * 0.6) : undefined
                SplitView.fillHeight: true
                SplitView.fillWidth: mainWindow.isVerticalLayout ? false : true

                handle: Rectangle {
                    implicitWidth: 4
                    implicitHeight: 4
                    color: SplitHandle.pressed ? Theme.surfacePressed : (SplitHandle.hovered ? Theme.surfaceHover : "#000000")
                }

                // --- Default Layout Placeholders ---
                Item {
                    id: ph_projectBrowser_default
                    visible: !mainWindow.isVerticalLayout
                    SplitView.preferredWidth: Math.round(mainWindow.width * 0.28)
                    SplitView.minimumWidth: 250
                    SplitView.fillHeight: true
                }

                Item {
                    id: ph_videoPreview_default
                    visible: !mainWindow.isVerticalLayout
                    SplitView.fillWidth: true
                    SplitView.minimumWidth: 300
                    SplitView.fillHeight: true
                }

                Item {
                    id: ph_inspector_default
                    visible: !mainWindow.isVerticalLayout
                    SplitView.preferredWidth: 340
                    SplitView.minimumWidth: 280
                    SplitView.fillHeight: true
                }

                // --- Vertical Layout Placeholders ---
                SplitView {
                    id: blockA_inner
                    orientation: Qt.Horizontal
                    visible: mainWindow.isVerticalLayout
                    SplitView.fillHeight: true
                    SplitView.preferredHeight: Math.round(blockA.height * 0.55)
                    SplitView.fillWidth: true

                    handle: Rectangle {
                        implicitWidth: 4
                        implicitHeight: 4
                        color: SplitHandle.pressed ? Theme.surfacePressed : (SplitHandle.hovered ? Theme.surfaceHover : "#000000")
                    }

                    Item {
                        id: ph_projectBrowser_vert
                        SplitView.fillWidth: true
                        SplitView.fillHeight: true
                    }
                    Item {
                        id: ph_inspector_vert
                        SplitView.fillWidth: true
                        SplitView.fillHeight: true
                        SplitView.preferredWidth: 300
                    }
                }

                Item {
                    id: ph_timeline_vert
                    visible: mainWindow.isVerticalLayout
                    SplitView.fillWidth: true
                    SplitView.fillHeight: true
                    SplitView.preferredHeight: Math.round(blockA.height * 0.45)
                    SplitView.minimumHeight: 150
                }
            }

            // Block B (Bottom in Default, Right in Vertical)
            Item {
                id: ph_timeline_default
                visible: !mainWindow.isVerticalLayout
                SplitView.preferredHeight: mainWindow.height * 0.45
                SplitView.minimumHeight: 150
                SplitView.fillHeight: true
                SplitView.fillWidth: true
            }

            Item {
                id: ph_videoPreview_vert
                visible: mainWindow.isVerticalLayout
                SplitView.fillWidth: true
                SplitView.fillHeight: true
                SplitView.minimumWidth: 300
            }
        }

        // --- Actual Components ---
        MediaPool {
            id: projectBrowser
            parent: mainWindow.isVerticalLayout ? ph_projectBrowser_vert : ph_projectBrowser_default
            anchors.fill: parent
            onMediaActivated: function(name, filePath, duration, hasVideo, hasAudio) {
                if (filePath.toLowerCase().endsWith(".srt")) {
                    subtitleCtrl.addSrtToTimeline(filePath, timelineView.timelineController)
                } else {
                    timelineView.addMediaClip(name, filePath, duration, hasVideo, hasAudio)
                }
            }
            onMediaDeleted: function(filePath) {
                timelineView.removeMediaClips(filePath)
                if (videoPreview.filePath === filePath)
                    videoPreview.clearPreview()
            }
        }

        SubtitleController {
            id: subtitleCtrl
        }

        EffectController {
            id: effectCtrl
            timelineController: timelineView.timelineController
            playbackEngine: videoPreview.playbackEngine
        }

        ProgramMonitor {
            id: videoPreview
            parent: mainWindow.isVerticalLayout ? ph_videoPreview_vert : ph_videoPreview_default
            anchors.fill: parent
            timelineCtrl: timelineView.timelineController
            subtitleCtrl: subtitleCtrl
            effectCtrl: effectCtrl
        }

        Inspector {
            id: inspectorPanel
            parent: mainWindow.isVerticalLayout ? ph_inspector_vert : ph_inspector_default
            anchors.fill: parent
            effectController: effectCtrl
        }

        TimelineView {
            id: timelineView
            parent: mainWindow.isVerticalLayout ? ph_timeline_vert : ph_timeline_default
            anchors.fill: parent
            playbackPosition: videoPreview.previewTime
            sequenceDuration: videoPreview.duration
            audioLevelLeft: videoPreview.audioLevelLeft
            audioLevelRight: videoPreview.audioLevelRight
            onPreviewRequested: function(name, filePath, duration, hasVideo) {
                videoPreview.loadClip(name, filePath, duration, hasVideo)
            }
            onPreviewCleared: videoPreview.clearPreview()
            onSeekRequested: function(seconds) {
                videoPreview.seekTo(seconds)
            }
            onSubtitleDropped: function(filePath, startSeconds, trackIndex) {
                subtitleCtrl.addSrtToTimelineAt(filePath, timelineView.timelineController, startSeconds, trackIndex)
            }
        }
    }

    MainSettings {
        id: mainSettingsDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(780, mainWindow.width - 80)
        height: Math.min(520, mainWindow.height - 80)
    }
}
 
