pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import VideoStudioUI

Rectangle {
    id: headersRoot

    property int trackHeight: 44
    property int markerHeight: 36
    property string timecode: "00:00:00:00"
    property string durationTimecode: "00:00:00:00"
    property bool snapEnabled: true
    property bool linkedSelection: true
    property bool hasTimelineClips: false
    property color panelLine: Theme.divider
    property color textPrimary: Theme.text
    property color accent: Theme.accent

    property real vScrollOffset: 0

    signal snapToggled()
    signal linkedToggled()
    signal markerRequested()
    signal settingsRequested(real x, real y)

    property var timelineController

    color: Theme.background
    clip: true

    Rectangle {
        id: rulerHeader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 38
        color: Theme.background
        border.color: "transparent"
        z: 10
        
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: Theme.divider
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: rulerHeader.bottom
        anchors.bottom: parent.bottom
        clip: true

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: -headersRoot.vScrollOffset
            spacing: 0

            Repeater {
                model: headersRoot.hasTimelineClips && headersRoot.timelineController ? headersRoot.timelineController.videoTrackCount : 0
                delegate: TimelineTrackControl {
                    id: videoTrackControl
                    width: headersRoot.width; height: headersRoot.trackHeight
                    isVideoTrack: true
                    hasTimelineClips: headersRoot.hasTimelineClips && index === headersRoot.timelineController.videoTrackCount - 1
                    trackName: "V" + (headersRoot.timelineController.videoTrackCount - index)
                    timelineController: headersRoot.timelineController
                    logicalTrackIndex: index
                    isTrackEmpty: headersRoot.timelineController.isTrackEmpty(true, logicalTrackIndex)
                    
                    Connections {
                        target: headersRoot.timelineController.clipModel
                        function onRowsInserted() { videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex) }
                        function onRowsRemoved() { videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex) }
                        function onDataChanged() { videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex) }
                        function onModelReset() { videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex) }
                    }
                }
            }

            Repeater {
                model: headersRoot.hasTimelineClips && headersRoot.timelineController ? headersRoot.timelineController.audioTrackCount : 0
                delegate: TimelineTrackControl {
                    id: audioTrackControl
                    width: headersRoot.width; height: headersRoot.trackHeight
                    isVideoTrack: false
                    hasTimelineClips: false
                    trackName: "A" + (index + 1)
                    timelineController: headersRoot.timelineController
                    logicalTrackIndex: index
                    isTrackEmpty: headersRoot.timelineController.isTrackEmpty(false, logicalTrackIndex)
                    
                    Connections {
                        target: headersRoot.timelineController.clipModel
                        function onRowsInserted() { audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex) }
                        function onRowsRemoved() { audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex) }
                        function onDataChanged() { audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex) }
                        function onModelReset() { audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex) }
                    }
                }
            }
            
            Item { width: parent.width; height: headersRoot.markerHeight }
        }
    }
}
