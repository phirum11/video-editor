// qmllint disable
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import VideoStudioUI
import VideoStudio.Core
import "header/menus"
import "inspector"
import "media_pool"
import "monitors"
import "timeline_view"
import "main_setting"

Window {
    id: mainWindow
    width: 1200
    height: 700
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: qsTr("Video Studio")
    color: '#f1eeee'
    flags: Qt.Window | Qt.FramelessWindowHint
    
    property bool isVerticalLayout: false
    property real _neededHeight: 800

    Connections {
        // qmllint disable unqualified
        target: typeof ActionManager !== "undefined" ? ActionManager : null
        function onActionTriggered(id) {
            if (id === "view.layout.default") {
                mainWindow.isVerticalLayout = false
            } else if (id === "view.layout.vertical") {
                mainWindow.isVerticalLayout = true
            }
        }
    }

    ScrollView {
        id: mainScrollView
        anchors.fill: parent
        contentWidth: Math.max(width, 1024)
        contentHeight: Math.max(height, mainWindow._neededHeight)
        clip: true

        ScrollBar.horizontal: ScrollBar {
            id: hbar
            policy: ScrollBar.AsNeeded
            background: Rectangle { color: Theme.background }
            contentItem: Rectangle {
                implicitHeight: 12
                radius: 6
                color: hbar.pressed ? Theme.textMuted : Theme.divider
            }
        }
        ScrollBar.vertical: ScrollBar {
            id: vbar
            policy: ScrollBar.AsNeeded
            background: Rectangle { color: Theme.background }
            contentItem: Rectangle {
                implicitWidth: 12
                radius: 6
                color: vbar.pressed ? Theme.textMuted : Theme.divider
            }
        }

        ColumnLayout {
            width: mainScrollView.contentWidth
            height: mainScrollView.contentHeight
            spacing: 0

        HeaderMenu {
            Layout.fillWidth: true
            windowTarget: mainWindow
            timelineController: timelineView.timelineController
            subtitleController: subtitleControllerObj
            onOpenSettings: mainSettingsDialog.open()
            onSaveProjectRequested: saveProjectDialog.open()
            onOpenProjectRequested: openProjectDialog.open()
        }

        ProjectManager {
            id: projectManager
        }

        FileDialog {
            id: saveProjectDialog
            title: "Save Project"
            fileMode: FileDialog.SaveFile
            nameFilters: ["Video Studio Projects (*.vsproj)", "All Files (*)"]
            onAccepted: {
                if (projectManager.saveProject(saveProjectDialog.selectedFile,
                                               projectBrowser.mediaPoolController,
                                               timelineView.timelineController,
                                               subtitleControllerObj)) {
                    console.log("Project saved successfully")
                } else {
                    console.log("Failed to save project")
                }
            }
        }

        FileDialog {
            id: openProjectDialog
            title: "Open Project"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Video Studio Projects (*.vsproj)", "All Files (*)"]
            onAccepted: {
                if (projectManager.loadProject(openProjectDialog.selectedFile,
                                               projectBrowser.mediaPoolController,
                                               timelineView.timelineController,
                                               subtitleControllerObj)) {
                    console.log("Project loaded successfully")
                } else {
                    console.log("Failed to load project")
                }
            }
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
                SplitView.preferredHeight: mainWindow.isVerticalLayout ? undefined : mainWindow.height * 0.50
                SplitView.minimumHeight: 300
                SplitView.preferredWidth: mainWindow.isVerticalLayout ? Math.round(mainWindow.width * 0.6) : undefined
                SplitView.fillWidth: mainWindow.isVerticalLayout ? false : true

                onHeightChanged: {
                    if (!mainWindow.isVerticalLayout) {
                        const minTimelineH = 250
                        const overhead = 40
                        mainWindow._neededHeight = Math.max(800, height + minTimelineH + overhead)
                    }
                }

                handle: Rectangle {
                    implicitWidth: 4
                    implicitHeight: 4
                    color: SplitHandle.pressed ? Theme.surfacePressed : (SplitHandle.hovered ? Theme.surfaceHover : "#000000")
                }

                Item {
                    id: ph_projectBrowser_default
                    visible: !mainWindow.isVerticalLayout
                    SplitView.preferredWidth: Math.round(mainWindow.width * 0.25)
                    SplitView.minimumWidth: 200
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
                    SplitView.preferredWidth: Math.round(mainWindow.width * 0.25)
                    SplitView.minimumWidth: 240
                    SplitView.fillHeight: true
                }

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
                SplitView.preferredHeight: mainWindow.height * 0.35
                SplitView.minimumHeight: 250
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

        MediaPool {
            id: projectBrowser
            parent: mainWindow.isVerticalLayout ? ph_projectBrowser_vert : ph_projectBrowser_default
            anchors.fill: parent
            onMediaActivated: function(name, filePath, duration, hasVideo, hasAudio) {
                if (filePath.toLowerCase().endsWith(".srt")) {
                    subtitleControllerObj.addSrtToTimeline(filePath, timelineView.timelineController)
                } else {
                    timelineView.addMediaClip(name, filePath, duration, hasVideo, hasAudio)
                }
            }
            onMediaDeleted: function(filePath) {
                timelineView.removeMediaClips(filePath)
                if (videoPreview.filePath === filePath)
                    videoPreview.clearPreview()
            }
            onEffectActivated: function(name, filePath) {
                effectControllerObj.addPresetToTimeline(name, filePath, videoPreview.previewTime, 0, 5.0)
            }
        }

        SubtitleController {
            id: subtitleControllerObj
        }

        EffectController {
            id: effectControllerObj
            timelineController: timelineView.timelineController
            playbackEngine: videoPreview.playbackEngine
        }

        ProgramMonitor {
            id: videoPreview
            parent: mainWindow.isVerticalLayout ? ph_videoPreview_vert : ph_videoPreview_default
            anchors.fill: parent
            timelineCtrl: timelineView.timelineController
            subtitleCtrl: subtitleControllerObj
            effectCtrl: effectControllerObj
        }

        Inspector {
            id: inspectorPanel
            parent: mainWindow.isVerticalLayout ? ph_inspector_vert : ph_inspector_default
            anchors.fill: parent
            effectController: effectControllerObj
        }

        TimelineView {
            id: timelineView
            parent: mainWindow.isVerticalLayout ? ph_timeline_vert : ph_timeline_default
            anchors.fill: parent
            playbackPosition: videoPreview.previewTime
            sequenceDuration: videoPreview.duration
            audioLevelLeft: videoPreview.audioLevelLeft
            audioLevelRight: videoPreview.audioLevelRight
            onPreviewRequested: function(name, filePath, duration, hasVideo, startOffset, sourceInPoint) {
                // Do nothing: keep displaying the sequence at the playhead
            }
            onPreviewCleared: {
                // Do nothing: deleting a clip shouldn't reset the program monitor playhead
            }
            onSeekRequested: function(seconds) {
                videoPreview.seekTo(seconds)
            }
            onSubtitleDropped: function(filePath, startSeconds, trackIndex) {
                subtitleControllerObj.addSrtToTimelineAt(filePath, timelineView.timelineController, startSeconds, trackIndex)
            }
            onEffectDropped: function(name, filePath, startSeconds, trackIndex) {
                effectControllerObj.addPresetToTimeline(name, filePath, startSeconds, trackIndex, 5.0)
            }
            onGenerateAudioRequested: function(language) {
                timelineView.timelineController.generateAIVoiceFromSrt(language)
            }
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
 
