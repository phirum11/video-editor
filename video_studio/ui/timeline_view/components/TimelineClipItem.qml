pragma ComponentBehavior: Bound

import QtQuick
import VideoStudioUI

Rectangle {
    id: clipRoot

    required property int clipIndex
    required property string clipName
    required property string filePath
    property string originalFilePath: ""
    required property real startSeconds
    required property real durationSeconds
    required property int trackIndex
    required property bool hasVideo
    required property bool hasAudio
    required property bool selected
    property int vocalIsolationType: 0
    property int isolationProgress: -1

    property real pixelsPerSecond: 18
    property int trackHeight: 34
    property int separatorHeight: 4
    property int minClipWidth: 72
    property string activeTool: "selection"
    property bool linkedSelection: true
    property bool snapEnabled: true

    signal selectedRequested(int index)
    signal previewRequested(string name, string path, real duration, bool video)
    signal deleteRequested(int index, string path)
    signal splitRequested(int index, real seconds, bool linked)
    signal moveRequested(int index, real startSeconds, int trackIndex, bool linked)
    signal trimLeftRequested(int index, real deltaSeconds, bool linked)
    signal trimRightRequested(int index, real deltaSeconds, bool linked)

    readonly property bool isSubtitle: !hasVideo && !hasAudio
    
    x: startSeconds * pixelsPerSecond
    y: (trackIndex < 3 ? trackIndex * trackHeight : trackIndex * trackHeight + separatorHeight) + (isSubtitle ? trackHeight - 20 - 4 : 4)
    width: Math.max(minClipWidth, durationSeconds * pixelsPerSecond)
    height: isSubtitle ? 20 : trackHeight - 8
    radius: 4
    color: hasVideo ? "#0c3a4a" : "#2a1f3e"
    clip: true
    z: selected ? 4 : 3

    border.color: selected ? "#5ec4e8" : "transparent"
    border.width: selected ? 1.5 : 0

    readonly property int filmFrameWidth: 64
    readonly property int filmFrameCount: hasVideo && width > 80
        ? Math.max(1, Math.min(20, Math.ceil(width / filmFrameWidth)))
        : 0
    readonly property int waveformBarCount: hasAudio && width > 28
        ? Math.max(20, Math.min(400, Math.floor(width / 1.5)))
        : 0
    readonly property int audioStripHeight: hasVideo
        ? Math.max(20, Math.floor(height * 0.45))
        : (height - 18)

    function snapTime(seconds) {
        if (!snapEnabled)
            return Math.max(0, seconds)
        return Math.max(0, Math.round(seconds * 4) / 4)
    }

    function isEditTool() {
        return activeTool === "selection" || activeTool === "ripple" || activeTool === "slip"
    }

    // FILMSTRIP — fills the clip behind everything
    Row {
        id: filmstrip
        anchors.left: parent.left       
        anchors.right: parent.right     
        anchors.top: parent.top   
        anchors.bottom: clipRoot.hasAudio && clipRoot.hasVideo ? waveform.top : parent.bottom
        visible: clipRoot.hasVideo && clipRoot.filmFrameCount > 0
        spacing: 1
        clip: true

        Repeater {
            model: clipRoot.filmFrameCount

            Image {
                required property int index

                width: Math.max(1, (filmstrip.width - Math.max(0, clipRoot.filmFrameCount - 1)) / Math.max(1, clipRoot.filmFrameCount))
                height: filmstrip.height
                source: "image://media/" + encodeURIComponent(
                    (clipRoot.originalFilePath !== "" ? clipRoot.originalFilePath : clipRoot.filePath) + "|" + (clipRoot.durationSeconds * (index + 0.5) / Math.max(1, clipRoot.filmFrameCount))
                )
                asynchronous: true
                fillMode: Image.PreserveAspectCrop
            }
        }
    }

    // TITLE OVERLAY — semi-transparent bar over filmstrip top
    Rectangle {
        id: titleOverlay
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 18
        z: 5

        gradient: Gradient {
            GradientStop { position: 0.0; color: clipRoot.hasVideo ? "#cc0e3d4d" : "#cc1e1435" }
            GradientStop { position: 1.0; color: "transparent" }
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: durationText.visible ? durationText.left : parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            text: clipRoot.clipName
            color: "#f0f8fa"
            font.pixelSize: 11
            font.weight: Font.Medium
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: durationText
            anchors.right: parent.right
            anchors.rightMargin: clipRoot.selected ? 28 : 8
            anchors.verticalCenter: parent.verticalCenter
            text: clipRoot.formatDuration(clipRoot.durationSeconds)
            color: "#c8e8f0"
            font.pixelSize: 10
            visible: clipRoot.width > 140
        }
    }

    // AUDIO WAVEFORM STRIP — compact bar at very bottom
    Rectangle {
        id: waveform
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: clipRoot.hasVideo ? clipRoot.audioStripHeight : clipRoot.audioStripHeight
        visible: clipRoot.hasAudio
        color: clipRoot.hasVideo ? "#0a2a38" : "transparent"
        clip: true
        z: 4

        // Top divider line for video+audio clips
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: "#1a5a6a"
            opacity: 0.5
            visible: clipRoot.hasVideo
        }

        Image {
            id: waveformImage
            anchors.fill: parent
            anchors.margins: 2
            source: clipRoot.hasAudio ? "image://waveform/" + encodeURIComponent(clipRoot.filePath) + "?width=" + Math.floor(width) + "&height=" + Math.floor(height) : ""
            asynchronous: true
            fillMode: Image.Stretch
            visible: clipRoot.hasAudio
        }
    }

    // DELETE BUTTON — clean circle, top-right
    Rectangle {
        id: deleteClipButton
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 3
        width: 16
        height: 16
        radius: 8
        visible: clipRoot.selected
        color: deleteClipMouse.containsMouse ? "#e04050" : "#00000066"
        z: 10

        Text {
            anchors.centerIn: parent
            text: "\u00d7"
            color: "#ffffff"
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        MouseArea {
            id: deleteClipMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: clipRoot.deleteRequested(clipRoot.clipIndex, clipRoot.filePath)
        }
    }

    // SELECTION BORDER — subtle glow when selected
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: clipRoot.selected ? "#5ec4e8" : (clipRoot.hasVideo ? "#1a6a7a" : "#4a3a6a")
        border.width: clipRoot.selected ? 1.5 : 1
        radius: clipRoot.radius
        z: 8
        opacity: clipRoot.selected ? 1.0 : 0.6
    }

    MouseArea {
        id: clipMouse
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        z: 6

        property real pressX: 0
        property real pressY: 0
        property real originalStart: 0
        property int originalTrack: 0
        property bool draggingClip: false

        cursorShape: clipRoot.activeTool === "razor"
            ? Qt.CrossCursor
            : clipRoot.activeTool === "hand"
                ? Qt.OpenHandCursor
                : Qt.ArrowCursor

        onPressed: function(mouse) {
            clipRoot.selectedRequested(clipRoot.clipIndex)
            if (clipRoot.hasVideo || clipRoot.hasAudio) {
                clipRoot.previewRequested(clipRoot.clipName, clipRoot.filePath, clipRoot.durationSeconds, clipRoot.hasVideo)
            }
        }

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                clipContextMenu.popup()
                return
            }
            if (clipRoot.activeTool === "razor") {
                const splitAt = clipRoot.startSeconds + mouse.x / Math.max(1, clipRoot.pixelsPerSecond)
                clipRoot.splitRequested(clipRoot.clipIndex, splitAt, clipRoot.linkedSelection)
            }
        }
    }

    DragHandler {
        id: clipDrag
        target: null
        onActiveChanged: {
            if (active) {
                clipMouse.originalStart = clipRoot.startSeconds
                clipMouse.originalTrack = clipRoot.trackIndex
            }
        }
        onTranslationChanged: {
            if (!active || !clipRoot.isEditTool()) return
            const deltaX = translation.x
            const deltaY = translation.y
            const nextStart = clipRoot.snapTime(clipMouse.originalStart + deltaX / Math.max(1, clipRoot.pixelsPerSecond))
            const nextTrack = Math.max(0, Math.min(5, clipMouse.originalTrack + Math.round(deltaY / clipRoot.trackHeight)))
            clipRoot.moveRequested(clipRoot.clipIndex, nextStart, nextTrack, clipRoot.linkedSelection)
        }
    }
    Rectangle {
        id: leftTrimHandle
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 12
        color: "white"
        radius: 4
        visible: clipRoot.selected && (leftTrimMouse.containsMouse || leftTrimMouse.pressed || rightTrimMouse.containsMouse || rightTrimMouse.pressed || clipMouse.containsMouse)
        z: 9
        opacity: leftTrimMouse.containsMouse || leftTrimMouse.pressed ? 1.0 : 0.0

        Behavior on opacity { NumberAnimation { duration: 150 } }

        Image {
            anchors.centerIn: parent
            source: "qrc:/VideoStudioUI/assets/trim-handle.svg"
            width: 12
            height: 32
        }

        MouseArea {
            id: leftTrimMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            property real lastX: 0
        }

        DragHandler {
            id: leftTrimDrag
            target: null
            onActiveChanged: {
                if (active) {
                    leftTrimMouse.lastX = 0
                }
            }
            onTranslationChanged: {
                if (!active) return
                const deltaX = translation.x - leftTrimMouse.lastX
                leftTrimMouse.lastX = translation.x
                const deltaSeconds = deltaX / Math.max(1, clipRoot.pixelsPerSecond)
                clipRoot.trimLeftRequested(clipRoot.clipIndex, deltaSeconds, clipRoot.linkedSelection)
            }
        }
    }

    Rectangle {
        id: rightTrimHandle
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 12
        color: "white"
        radius: 4
        visible: leftTrimHandle.visible
        z: 9
        opacity: rightTrimMouse.containsMouse || rightTrimMouse.pressed ? 1.0 : 0.0

        Behavior on opacity { NumberAnimation { duration: 150 } }

        Image {
            anchors.centerIn: parent
            source: "qrc:/VideoStudioUI/assets/trim-handle.svg"
            width: 12
            height: 32
        }

        MouseArea {
            id: rightTrimMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            property real lastX: 0
        }

        DragHandler {
            id: rightTrimDrag
            target: null
            onActiveChanged: {
                if (active) {
                    rightTrimMouse.lastX = 0
                }
            }
            onTranslationChanged: {
                if (!active) return
                const deltaX = translation.x - rightTrimMouse.lastX
                rightTrimMouse.lastX = translation.x
                const deltaSeconds = deltaX / Math.max(1, clipRoot.pixelsPerSecond)
                clipRoot.trimRightRequested(clipRoot.clipIndex, deltaSeconds, clipRoot.linkedSelection)
            }
        }
    }
    
    function formatDuration(seconds) {
        const safeSeconds = Number.isFinite(seconds) && seconds > 0 ? Math.floor(seconds) : 0
        const secs = safeSeconds % 60
        const mins = Math.floor(safeSeconds / 60) % 60
        const hours = Math.floor(safeSeconds / 3600)
        return hours > 0
            ? String(hours).padStart(2, "0") + ":" + String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0")
            : String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0")
    }

    TimelineClipContextMenu {
        id: clipContextMenu
        clipIndex: clipRoot.clipIndex
        vocalIsolationType: clipRoot.vocalIsolationType
        // Resolve timelineController from parent scope (TimelineTracks.qml has backend)
        // qmllint disable unqualified
        timelineController: typeof backend !== "undefined" ? backend : null
        // qmllint enable unqualified
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        height: 6
        width: parent.width * (clipRoot.isolationProgress / 100.0)
        color: Theme.accent
        visible: clipRoot.isolationProgress >= 0
        z: 10
        
        Behavior on width {
            NumberAnimation { duration: 150 }
        }
    }
    
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: clipRoot.isolationProgress >= 0
        z: 9
        
        Text {
            anchors.centerIn: parent
            text: "Isolating... " + clipRoot.isolationProgress + "%"
            color: Theme.text
            font.pixelSize: 11
        }
    }
}
