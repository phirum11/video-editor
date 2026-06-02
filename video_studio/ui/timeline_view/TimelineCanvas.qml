pragma ComponentBehavior: Bound

import QtQuick

import VideoStudioUI

Rectangle {
    id: canvasRoot

    property var clipModel
    property int selectedClipIndex: -1
    property real pixelsPerSecond: 18
    property real scrollOffset: 0
    property real vScrollOffset: 0
    property real contentWidth: width
    property real playheadSeconds: 0
    property int trackHeight: 34
    property int markerHeight: 28
    property int separatorHeight: 4
    property int minClipWidth: 72
    property string activeTool: "selection"
    property bool linkedSelection: true
    property bool snapEnabled: true
    property bool hasClips: false
    property var markers: []
    readonly property real playheadX: playheadSeconds * pixelsPerSecond - scrollOffset
    readonly property real visibleDurationSeconds: Math.max(1, width / Math.max(0.001, pixelsPerSecond))
    readonly property real majorStepSeconds: chooseMajorStepSeconds(visibleDurationSeconds)
    readonly property int minorDivisions: chooseMinorDivisions(majorStepSeconds)
    readonly property real minorStepSeconds: majorStepSeconds / Math.max(1, minorDivisions)

    signal selectionCleared()
    signal clipSelected(int index)
    signal previewRequested(string name, string filePath, real duration, bool hasVideo)
    signal clipDeleted(int index, string filePath)
    signal clipSplit(int index, real seconds, bool linked)
    signal clipMoved(int index, real startSeconds, int trackIndex, bool linked)
    signal seekPreview(real seconds)
    signal seekCommitted(real seconds)
    signal panRequested(real deltaPixels)
    signal zoomRequested(real anchorX, int direction)
    signal mediaDropped(string name, string filePath, real duration, bool hasVideo, bool hasAudio, real startSeconds, int trackIndex)
    signal subtitleDropped(string filePath, real startSeconds, int trackIndex)
    signal trimLeftRequested(int index, real deltaSeconds, bool linked)
    signal trimRightRequested(int index, real deltaSeconds, bool linked)

    color: Theme.background
    clip: true
    focus: true

    function chooseMajorStepSeconds(visibleSeconds) {
        const targetLabels = width >= 1200 ? 8 : 6
        const minimumSeconds = Math.max(1, visibleSeconds / targetLabels)
        const steps = [1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 1200, 1800, 3600, 7200 ,8200 ,10000]
        for (let i = 0; i < steps.length; ++i) {
            if (steps[i] >= minimumSeconds)
                return steps[i]
        }
        return steps[steps.length - 1]
    }

    function chooseMinorDivisions(majorSeconds) {
        if (majorSeconds >= 600)
            return 10
        if (majorSeconds >= 300)
            return 5
        if (majorSeconds >= 60)
            return 6
        if (majorSeconds >= 10)
            return 5
        return Math.max(1, majorSeconds)
    }

    function secondsFromX(x) {
        return Math.max(0, (x + scrollOffset) / Math.max(0.001, pixelsPerSecond))
    }

    function isEditTool() {
        return activeTool === "selection" || activeTool === "ripple" || activeTool === "slip"
    }

    function trackY(trackIndex) {
        const clampedTrack = Math.max(0, Math.min(trackIndex, 5))
        return (clampedTrack < 3
            ? clampedTrack * trackHeight
            : clampedTrack * trackHeight + separatorHeight) - vScrollOffset
    }

    function trackIndexFromY(y, hasVideo) {
        const adjustedY = (y + vScrollOffset) >= trackHeight * 3 + separatorHeight
            ? (y + vScrollOffset) - separatorHeight
            : (y + vScrollOffset)
        const rawTrack = Math.floor(Math.max(0, adjustedY) / trackHeight)
        if (hasVideo)
            return Math.max(0, Math.min(rawTrack, 2))
        return Math.max(3, Math.min(rawTrack, 5))
    }

    Keys.onDeletePressed: canvasRoot.clipDeleted(canvasRoot.selectedClipIndex, "")

    Repeater {
        model: 6

        Rectangle {
            required property int index

            y: canvasRoot.trackY(index)
            width: canvasRoot.width
            height: canvasRoot.trackHeight
            color: index === 2
                ? Theme.surfaceRaised
                : (index % 2 === 0 ? Theme.background : Theme.surface)
            border.width: 0
            visible: canvasRoot.hasClips

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#222222"
                opacity: 0.45
            }
        }
    }

    Rectangle {
        y: canvasRoot.trackHeight * 3 - canvasRoot.vScrollOffset
        width: canvasRoot.width
        height: canvasRoot.separatorHeight
        color: Theme.surfaceInset
        border.width: 0
        visible: canvasRoot.hasClips
    }

    Rectangle {
        y: canvasRoot.trackY(5) + canvasRoot.trackHeight
        width: canvasRoot.width
        height: canvasRoot.markerHeight
        color: Theme.surface
        border.width: 0
        visible: canvasRoot.hasClips
    }

    Repeater {
        model: Math.ceil(canvasRoot.contentWidth / Math.max(1, canvasRoot.minorStepSeconds * canvasRoot.pixelsPerSecond)) + 2

        Rectangle {
            required property int index

            readonly property bool major: index % canvasRoot.minorDivisions === 0
            x: index * canvasRoot.minorStepSeconds * canvasRoot.pixelsPerSecond - canvasRoot.scrollOffset
            width: 1
            height: canvasRoot.height
            color: "#333333"
            opacity: 0.24
            visible: canvasRoot.hasClips && major && x > -20 && x < canvasRoot.width + 20
        }
    }

    Repeater {
        model: canvasRoot.markers

        Rectangle {
            required property real modelData

            x: modelData * canvasRoot.pixelsPerSecond - canvasRoot.scrollOffset - width / 2
            y: 0
            width: 2
            height: canvasRoot.height
            color: "#d7b15c"
            opacity: 0.8
            visible: x > -10 && x < canvasRoot.width + 10
        }
    }

    MouseArea {
        id: canvasMouse
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        z: 1

        property real lastX: 0

        cursorShape: canvasRoot.activeTool === "hand"
            ? Qt.OpenHandCursor
            : canvasRoot.activeTool === "zoom"
                ? Qt.PointingHandCursor
                : Qt.ArrowCursor

        onPressed: function(mouse) {
            canvasRoot.forceActiveFocus()
            lastX = mouse.x
            if (canvasRoot.activeTool === "hand")
                return
            if (canvasRoot.activeTool === "zoom") {
                canvasRoot.zoomRequested(mouse.x, mouse.button === Qt.RightButton ? -1 : 1)
                return
            }
            if (canvasRoot.isEditTool()) {
                canvasRoot.selectionCleared()
                canvasRoot.seekPreview(canvasRoot.secondsFromX(mouse.x))
            }
        }

        onPositionChanged: function(mouse) {
            if (!pressed)
                return
            if (canvasRoot.activeTool === "hand") {
                canvasRoot.panRequested(lastX - mouse.x)
                lastX = mouse.x
                return
            }
            if (canvasRoot.isEditTool())
                canvasRoot.seekPreview(canvasRoot.secondsFromX(mouse.x))
        }

        onReleased: function(mouse) {
            if (canvasRoot.isEditTool())
                canvasRoot.seekCommitted(canvasRoot.secondsFromX(mouse.x))
        }
    }

    DropArea {
        id: timelineDropArea
        anchors.fill: parent
        keys: ["videoStudio/media"]
        z: 2

        function mediaFromDrop(drop) {
            if (!drop || !drop.source)
                return null
            if (drop.source.mediaFilePath)
                return drop.source
            if (drop.source.sourceMedia && drop.source.sourceMedia.mediaFilePath)
                return drop.source.sourceMedia
            return null
        }

        onEntered: function(drag) {
            drag.acceptProposedAction()
        }

        onDropped: function(drop) {
            const media = mediaFromDrop(drop)
            if (!media || !media.mediaFilePath)
                return

            if (media.mediaFilePath.toLowerCase().endsWith(".srt")) {
                canvasRoot.subtitleDropped(
                    media.mediaFilePath,
                    canvasRoot.secondsFromX(drop.x),
                    canvasRoot.trackIndexFromY(drop.y, false)
                )
                drop.acceptProposedAction()
                return
            }

            canvasRoot.mediaDropped(
                media.mediaName,
                media.mediaFilePath,
                media.mediaDuration,
                media.mediaHasVideo,
                media.mediaHasAudio,
                canvasRoot.secondsFromX(drop.x),
                canvasRoot.trackIndexFromY(drop.y, media.mediaHasVideo)
            )
            drop.acceptProposedAction()
        }
    }

    Item {
        id: clipLayer
        x: -canvasRoot.scrollOffset
        width: canvasRoot.contentWidth
        height: parent.height
        z: 3

        Repeater {
            id: clipRepeater
            model: canvasRoot.clipModel

            Item {
                required property int index
                required property string clipName
                required property string filePath
                required property real startSeconds
                required property real durationSeconds
                required property int trackIndex
                required property bool hasVideo
                required property bool hasAudio

                readonly property bool clipSelected: canvasRoot.selectedClipIndex === index
                readonly property bool isSubtitle: !hasVideo && !hasAudio

                x: startSeconds * canvasRoot.pixelsPerSecond
                y: (trackIndex < 3 ? trackIndex * canvasRoot.trackHeight : trackIndex * canvasRoot.trackHeight + canvasRoot.separatorHeight) + (isSubtitle ? canvasRoot.trackHeight - 20 - 4 : 4) - canvasRoot.vScrollOffset
                width: Math.max(canvasRoot.minClipWidth, durationSeconds * canvasRoot.pixelsPerSecond)
                height: isSubtitle ? 20 : canvasRoot.trackHeight - 8
                z: clipSelected ? 4 : 3

                TimelineClipItem {
                    anchors.fill: parent
                    clipIndex: parent.index
                    clipName: parent.clipName
                    filePath: parent.filePath
                    startSeconds: parent.startSeconds
                    durationSeconds: parent.durationSeconds
                    trackIndex: parent.trackIndex
                    hasVideo: parent.hasVideo
                    hasAudio: parent.hasAudio
                    selected: parent.clipSelected
                    pixelsPerSecond: canvasRoot.pixelsPerSecond
                    trackHeight: canvasRoot.trackHeight
                    separatorHeight: canvasRoot.separatorHeight
                    minClipWidth: canvasRoot.minClipWidth
                    activeTool: canvasRoot.activeTool
                    linkedSelection: canvasRoot.linkedSelection
                    snapEnabled: canvasRoot.snapEnabled

                    onSelectedRequested: function(selectedIndex) { canvasRoot.clipSelected(selectedIndex) }
                    onPreviewRequested: function(name, path, duration, video) { canvasRoot.previewRequested(name, path, duration, video) }
                    onDeleteRequested: function(selectedIndex, path) { canvasRoot.clipDeleted(selectedIndex, path) }
                    onSplitRequested: function(selectedIndex, seconds, linked) { canvasRoot.clipSplit(selectedIndex, seconds, linked) }
                    onMoveRequested: function(selectedIndex, startSecondsValue, trackIndexValue, linked) {
                        canvasRoot.clipMoved(selectedIndex, startSecondsValue, trackIndexValue, linked)
                    }
                    onTrimLeftRequested: function(selectedIndex, deltaSeconds, linked) {
                        canvasRoot.trimLeftRequested(selectedIndex, deltaSeconds, linked)
                    }
                    onTrimRightRequested: function(selectedIndex, deltaSeconds, linked) {
                        canvasRoot.trimRightRequested(selectedIndex, deltaSeconds, linked)
                    }
                }
            }
        }
    }

    Rectangle {
        x: canvasRoot.playheadX
        y: 0
        width: 2
        height: canvasRoot.height
        color: "#58a8d8"
        visible: x > -2 && x < canvasRoot.width
        z: 4
    }

    Rectangle {
        id: emptyDropTarget
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 36
        anchors.rightMargin: 36
        height: 72
        radius: 6
        visible: !canvasRoot.hasClips
        color: "transparent"
        border.color: timelineDropArea.containsDrag ? "#58a8d8" : Theme.divider
        border.width: 1
        opacity: timelineDropArea.containsDrag ? 0.92 : 0.68

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 28
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            Image {
                width: 17
                height: 17
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/VideoStudioUI/assets/film.svg"
                opacity: 0.78
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Drag material here and start to create")
                color: "#d6e0e4"
                font.pixelSize: 13
                opacity: 0.88
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: timelineDropArea.containsDrag
        color: "transparent"
        border.color: "#58a8d8"
        border.width: 2
        opacity: 0.85
        z: 5
    }
}
