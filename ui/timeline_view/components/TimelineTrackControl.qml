// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import VideoStudioUI
import "../../../ui/header/dialogs"

Rectangle {
    id: controlRoot
    color: Theme.background
    clip: true

    property bool hasTimelineClips: false
    property bool isMainTrack: isVideoTrack && logicalTrackIndex === 0
    property bool isVideoTrack: true
    property bool isSubtitleTrack: false
    property bool isEffectTrack: false
    property string trackName: "Track"
    property var timelineController: null
    required property int index
    property int logicalTrackIndex: 0
    property bool isTrackEmpty: true

    property bool isLocked: false
    property bool isHidden: false
    property bool isMuted: false

    HoverHandler { id: rootHover }

    Connections {
        target: controlRoot.timelineController
        function onTrackStateChanged(isVideo, trackIdx) {
            if (isVideo === controlRoot.isVideoTrack && trackIdx === controlRoot.logicalTrackIndex) {
                controlRoot.isLocked = controlRoot.timelineController.isTrackLocked(isVideo, trackIdx)
                controlRoot.isHidden = controlRoot.timelineController.isTrackHidden(isVideo, trackIdx)
                controlRoot.isMuted = controlRoot.timelineController.isTrackMuted(isVideo, trackIdx)
            }
        }
    }
    
    onTimelineControllerChanged: {
        if (timelineController) {
            isLocked = timelineController.isTrackLocked(isVideoTrack, logicalTrackIndex)
            isHidden = timelineController.isTrackHidden(isVideoTrack, logicalTrackIndex)
            isMuted = timelineController.isTrackMuted(isVideoTrack, logicalTrackIndex)
        }
    }


    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.divider
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 12
        spacing: 12 

        // Icons
        RowLayout { 
            visible: !controlRoot.isTrackEmpty
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Image {
                source: controlRoot.isEffectTrack ? "qrc:/VideoStudioUI/assets/star.svg" : (controlRoot.isSubtitleTrack ? "qrc:/VideoStudioUI/assets/track-subtitle.svg" : (controlRoot.isVideoTrack ? "qrc:/VideoStudioUI/assets/track-video.svg" : "qrc:/VideoStudioUI/assets/track-audio.svg"))
                sourceSize: Qt.size(14, 14)
                Layout.preferredWidth: 14; Layout.preferredHeight: 14
                fillMode: Image.PreserveAspectFit
                opacity: 0.6
            }

            Image {
                source: controlRoot.isLocked ? "qrc:/VideoStudioUI/assets/lock.svg" : "qrc:/VideoStudioUI/assets/unlock.svg"
                sourceSize: Qt.size(14, 14)
                Layout.preferredWidth: 14; Layout.preferredHeight: 14
                fillMode: Image.PreserveAspectFit
                opacity: controlRoot.isLocked ? 1.0 : (rootHover.hovered ? (lockHover.hovered ? 1.0 : 0.7) : 0.4)
                HoverHandler { id: lockHover }
                ToolTip.visible: lockHover.hovered
                ToolTip.text: controlRoot.isLocked ? "Unlock" : "Lock"
                MouseArea { 
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor 
                    onClicked: {
                        if (controlRoot.timelineController) {
                            controlRoot.timelineController.setTrackLocked(controlRoot.isVideoTrack, controlRoot.logicalTrackIndex, !controlRoot.isLocked)
                        } else {
                            controlRoot.isLocked = !controlRoot.isLocked
                        }
                    }
                }
            }
            Image {
                source: controlRoot.isHidden ? "qrc:/VideoStudioUI/assets/eye-off.svg" : "qrc:/VideoStudioUI/assets/eye.svg"
                sourceSize: Qt.size(14, 14)
                Layout.preferredWidth: 14; Layout.preferredHeight: 14
                fillMode: Image.PreserveAspectFit
                opacity: (!controlRoot.isVideoTrack && !controlRoot.isSubtitleTrack) ? 0.0 : (controlRoot.isHidden ? 1.0 : (rootHover.hovered ? (hideHover.hovered ? 1.0 : 0.7) : 0.4))
                enabled: controlRoot.isVideoTrack || controlRoot.isSubtitleTrack
                HoverHandler { id: hideHover }
                ToolTip.visible: hideHover.hovered
                ToolTip.text: controlRoot.isHidden ? "Show Track" : "Hide Track"
                MouseArea { 
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor 
                    onClicked: {
                        if (controlRoot.timelineController) {
                            controlRoot.timelineController.setTrackHidden(controlRoot.isVideoTrack, controlRoot.logicalTrackIndex, !controlRoot.isHidden)
                        } else {
                            controlRoot.isHidden = !controlRoot.isHidden
                        }
                    }
                }
            }
            Image {
                visible: !controlRoot.isSubtitleTrack
                source: controlRoot.isMuted ? "qrc:/VideoStudioUI/assets/volume-x.svg" : "qrc:/VideoStudioUI/assets/volume-2.svg"
                sourceSize: Qt.size(14, 14)   
                Layout.preferredWidth: 14; Layout.preferredHeight: 14
                fillMode: Image.PreserveAspectFit
                opacity: controlRoot.isMuted ? 1.0 : (rootHover.hovered ? (muteHover.hovered ? 1.0 : 0.7) : 0.4)
                HoverHandler { id: muteHover }
                ToolTip.visible: muteHover.hovered
                ToolTip.text: controlRoot.isMuted ? "Unmute" : "Mute"
                MouseArea { 
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor 
                    onClicked: {
                        if (controlRoot.timelineController) {
                            controlRoot.timelineController.setTrackMuted(controlRoot.isVideoTrack, controlRoot.logicalTrackIndex, !controlRoot.isMuted)
                        } else {
                            controlRoot.isMuted = !controlRoot.isMuted
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // CapCut style Cover button
        Rectangle {
            visible: controlRoot.isMainTrack && controlRoot.isVideoTrack && !controlRoot.isSubtitleTrack && !controlRoot.isTrackEmpty
            width: 32
            height: 30
            radius: 4
            color: "#303033"
            border.color: "#4a4a4f"
            border.width: 1
            clip: true
            
            CoverSelectDialog {
                id: coverSelectDialog
                timelineController: controlRoot.timelineController
                parent: Overlay.overlay
            }

            Image {
                source: "qrc:/VideoStudioUI/assets/edit-2.svg"
                sourceSize: Qt.size(14, 14)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 2
                opacity: 0.8
            }
            Text {
                text: "Cover"
                color: "white"
                font.pixelSize: 9
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
                opacity: 0.8
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    coverSelectDialog.open()
                }
            }
        }
    }
}
