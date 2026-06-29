pragma ComponentBehavior: Bound

import QtQuick
import VideoStudioUI

Rectangle {
    id: headersRoot

    property int trackHeight: 44
    property int subtitleTrackHeight: 32
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

    signal snapToggled
    signal linkedToggled
    signal markerRequested
    signal settingsRequested(real x, real y)

    property var timelineController
    property int videoTrackCount: timelineController ? timelineController.videoTrackCount : 0
    property int effectTrackCount: timelineController ? timelineController.effectTrackCount : 0

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
                model: (headersRoot.timelineController && headersRoot.timelineController.hasSubtitleTrack) ? 1 : 0
                delegate: TimelineTrackControl {
                    id: subtitleTrackControl
                    width: headersRoot.width
                    height: headersRoot.subtitleTrackHeight
                    isVideoTrack: false
                    isSubtitleTrack: true
                    hasTimelineClips: headersRoot.hasTimelineClips
                    trackName: "T"
                    timelineController: headersRoot.timelineController
                    logicalTrackIndex: 0
                    isTrackEmpty: false // Subtitle track is usually not considered empty if it exists

                    Connections {
                        target: headersRoot.timelineController.clipModel
                        function onRowsInserted() {
                            // Subtitle track doesn't need to check emptiness for now, but we can keep it consistent
                        }
                    }
                }
            }

            Repeater {
                model: headersRoot.effectTrackCount
                delegate: TimelineTrackControl {
                    id: effectTrackControl
                    width: headersRoot.width
                    height: headersRoot.subtitleTrackHeight
                    isVideoTrack: false
                    isSubtitleTrack: false
                    isEffectTrack: true
                    hasTimelineClips: headersRoot.hasTimelineClips
                    trackName: "E" + (index + 1)
                    timelineController: headersRoot.timelineController
                    logicalTrackIndex: index
                    isTrackEmpty: false
                }
            }

            Repeater {
                model: headersRoot.videoTrackCount
                delegate: TimelineTrackControl {
                    id: videoTrackControl
                    width: headersRoot.width
                    height: headersRoot.trackHeight
                    isVideoTrack: true
                    hasTimelineClips: headersRoot.hasTimelineClips
                    isMainTrack: logicalTrackIndex === 0
                    trackName: "V" + (logicalTrackIndex + 1)
                    timelineController: headersRoot.timelineController
                    logicalTrackIndex: headersRoot.videoTrackCount - 1 - index
                    isTrackEmpty: headersRoot.timelineController ? headersRoot.timelineController.isTrackEmpty(true, logicalTrackIndex) : true

                    Connections {
                        target: headersRoot.timelineController.clipModel
                        function onRowsInserted() {
                            videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex);
                        }
                        function onRowsRemoved() {
                            videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex);
                        }
                        function onDataChanged() {
                            videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex);
                        }
                        function onModelReset() {
                            videoTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(true, videoTrackControl.logicalTrackIndex);
                        }
                    }
                }
            }

            Repeater {
                model: headersRoot.timelineController ? headersRoot.timelineController.audioTrackCount : 0
                delegate: TimelineTrackControl {
                    id: audioTrackControl
                    width: headersRoot.width
                    height: headersRoot.trackHeight
                    isVideoTrack: false
                    hasTimelineClips: false
                    trackName: "A" + (index + 1)
                    timelineController: headersRoot.timelineController
                    logicalTrackIndex: index
                    isTrackEmpty: headersRoot.timelineController.isTrackEmpty(false, logicalTrackIndex)

                    Connections {
                        target: headersRoot.timelineController.clipModel
                        function onRowsInserted() {
                            audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex);
                        }
                        function onRowsRemoved() {
                            audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex);
                        }
                        function onDataChanged() {
                            audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex);
                        }
                        function onModelReset() {
                            audioTrackControl.isTrackEmpty = headersRoot.timelineController.isTrackEmpty(false, audioTrackControl.logicalTrackIndex);
                        }
                    }
                }
            }

            Item {
                width: parent.width
                height: headersRoot.markerHeight
            }
        }
    }
}
