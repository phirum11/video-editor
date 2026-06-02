pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import VideoStudioUI

Rectangle {
    id: headersRoot

    property int trackHeight: 44
    property int markerHeight: 36
    property string timecode: "00:00:00:00"
    property bool snapEnabled: true
    property bool linkedSelection: true
    property bool hasTimelineClips: false
    property color panelLine: Theme.divider
    property color textPrimary: "#d6e0e4"
    property color accent: "#58a8d8"

    property real vScrollOffset: 0

    signal snapToggled()
    signal linkedToggled()
    signal markerRequested()
    signal settingsRequested(real x, real y)

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

            Item { width: parent.width; height: headersRoot.trackHeight }
            Item { width: parent.width; height: headersRoot.trackHeight }

            Item {
                width: parent.width
                height: headersRoot.trackHeight

                Rectangle {
                    anchors.centerIn: parent
                    width: 48
                    height: 30
                    radius: 4
                    visible: headersRoot.hasTimelineClips
                    color: coverHover.hovered ? Theme.surfaceHover : Theme.surfaceRaised
                    border.color: coverHover.hovered ? Theme.divider : "transparent"
                    border.width: 1

                    Column {
                        anchors.centerIn: parent
                        spacing: 1

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "\u270e"
                            color: "#d6e0e4"
                            font.pixelSize: 13
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Cover")
                            color: "#d6e0e4"
                            font.pixelSize: 10
                        }
                    }

                    HoverHandler { id: coverHover }
                    ToolTip.visible: coverHover.hovered
                    ToolTip.text: qsTr("Edit Cover")
                }
            }

            Rectangle {
                width: parent.width
                height: 6
                color: Theme.surfaceInset
            }

            Item { width: parent.width; height: headersRoot.trackHeight }
            Item { width: parent.width; height: headersRoot.trackHeight }
            Item { width: parent.width; height: headersRoot.trackHeight }
            Item { width: parent.width; height: headersRoot.markerHeight }
        }
    }
}
