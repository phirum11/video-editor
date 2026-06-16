pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI
import "components"

import "../settings"

Rectangle {
    id: tracksRoot

    readonly property int trackHeaderWidth: 96
    readonly property int trackHeight: 72
    readonly property int markerHeight: 36
    readonly property int separatorHeight: 6
    readonly property int minClipWidth: 140
    readonly property color panelBody: Theme.background
    readonly property color panelHeader: Theme.surface
    readonly property color panelLine: Theme.divider
    readonly property color textPrimary: "#d6e0e4"
    readonly property color textMuted: "#7f939c"
    readonly property color accent: "#58a8d8"

    property alias timelineController: backend
    property string activeTool: "selection"
    property bool snapEnabled: true
    property bool linkedSelection: true
    property real playbackPosition: 0
    property real sequenceDuration: 0
    property real timelinePosition: 0
    property bool scrubbing: false
    property real scrubPosition: 0
    property real scrollOffset: 0
    property real vScrollOffset: 0
    property real zoomValue: 0
    property var markers: []
    property string previewFilePath: ""
    property real timelineEndSeconds: backend.timelineEndSeconds

    readonly property real contentHeight: 6 * trackHeight + separatorHeight + markerHeight
    readonly property real maxVScroll: Math.max(0, contentHeight - timelineViewport.height)

    readonly property real visiblePosition: scrubbing ? scrubPosition : timelinePosition
    readonly property real timelineSpanSeconds: Math.max(sequenceDuration, timelineEndSeconds, visiblePosition)
    readonly property real displayDuration: chooseDisplayDuration(timelineSpanSeconds)
    readonly property real fitPixelsPerSecond: Math.max(
        0.05,
        Math.max(1, timelineViewport.width) / Math.max(1, displayDuration)
    )
    readonly property real maxPixelsPerSecond: 72
    readonly property real pixelsPerSecond: pixelsForZoomValue(zoomValue)
    readonly property real contentWidth: Math.max(Math.max(1, timelineViewport.width), displayDuration * pixelsPerSecond + 220)
    readonly property real maxScroll: Math.max(0, contentWidth - Math.max(1, timelineViewport.width))

    signal previewRequested(string name, string filePath, real duration, bool hasVideo, real startOffset, real sourceInPoint)
    signal previewCleared()
    signal seekRequested(real seconds)
    signal toolSelected(string tool)
    signal snapToggleRequested(bool enabled)
    signal linkedSelectionToggleRequested(bool enabled)
    signal subtitleDropped(string filePath, real startSeconds, int trackIndex)

    color: panelBody
    border.color: panelLine
    border.width: 1
    clip: true

    TimelineController {
        id: backend
    }

    ClipTrimController {
        id: trimController
    }

    function clampScroll() {
        scrollOffset = Math.max(0, Math.min(scrollOffset, maxScroll))
        vScrollOffset = Math.max(0, Math.min(vScrollOffset, maxVScroll))
    }

    function setScrollOffset(value) {
        scrollOffset = Math.max(0, Math.min(value, maxScroll))
    }

    function setVScrollOffset(value) {
        vScrollOffset = Math.max(0, Math.min(value, maxVScroll))
    }

    function chooseDisplayDuration(spanSeconds) {
        const defaultEmptyDuration = 10 * 60
        const safeSpan = Math.max(0, spanSeconds)
        if (safeSpan <= 0)
            return defaultEmptyDuration

        return Math.max(60, roundUpTimelineDuration(Math.max(safeSpan + 30, safeSpan * 1.15)))
    }

    function roundUpTimelineDuration(seconds) {
        const step = seconds <= 60 ? 10
            : (seconds <= 300 ? 30
            : (seconds <= 1800 ? 60
            : (seconds <= 3600 ? 300
            : (seconds <= 7200 ? 600 : 1800))))
        return Math.ceil(seconds / step) * step
    }

    function pixelsForZoomValue(value) {
        const clampedZoom = Math.max(0, Math.min(1, value))
        const ratio = Math.max(1, maxPixelsPerSecond / Math.max(0.001, fitPixelsPerSecond))
        return fitPixelsPerSecond * Math.pow(ratio, clampedZoom)
    }

    function setZoom(value, anchorX) {
        const viewportWidth = Math.max(1, timelineViewport.width)
        const safeAnchor = Math.max(0, Math.min(anchorX >= 0 ? anchorX : viewportWidth / 2, viewportWidth))
        const anchorSeconds = (scrollOffset + safeAnchor) / Math.max(0.001, pixelsPerSecond)
        const targetZoom = Math.max(0, Math.min(1, value))
        zoomValue = targetZoom
        setScrollOffset(anchorSeconds * pixelsForZoomValue(targetZoom) - safeAnchor)
    }

    function zoomIn() {
        setZoom(zoomValue + 0.08, 0)
    }

    function zoomOut() {
        setZoom(zoomValue - 0.08, 0)
    }

    function centerOnSeconds(seconds) {
        setScrollOffset(seconds * pixelsPerSecond - timelineViewport.width / 2)
    }

    function keepPlayheadVisible() {
        if (scrubbing || timelineViewport.width <= 0)
            return

        const playheadPixel = visiblePosition * pixelsPerSecond
        const leftGuard = scrollOffset + 72
        const rightGuard = scrollOffset + timelineViewport.width - 72
        if (playheadPixel > rightGuard)
            setScrollOffset(playheadPixel - timelineViewport.width + 72)
        else if (playheadPixel < leftGuard)
            setScrollOffset(playheadPixel - 72)
    }

    function syncPlaybackPosition() {
        if (scrubbing)
            return

        const safePosition = Number.isFinite(playbackPosition)
            ? Math.max(0, playbackPosition)
            : 0
        if (Math.abs(timelinePosition - safePosition) < 0.001)
            return

        timelinePosition = safePosition
        keepPlayheadVisible()
    }

    function requestPreview(name, filePath, duration, hasVideo, startOffset, sourceInPoint) {
        previewFilePath = filePath
        previewRequested(name, filePath, duration, hasVideo, startOffset !== undefined ? startOffset : 0, sourceInPoint !== undefined ? sourceInPoint : 0)
    }

    function addMediaClip(name, filePath, duration, hasVideo, hasAudio, startSeconds, trackIndex) {
        const safeDuration = Math.max(duration, 1)
        const safeStart = startSeconds >= 0 ? startSeconds : backend.timelineEndSeconds
        const safeTrack = trackIndex >= 0 ? trackIndex : (hasVideo ? 2 : 3)
        const selectedRow = backend.addMediaAsset(
            name,
            filePath,
            safeDuration,
            hasVideo,
            hasAudio,
            safeStart,
            safeTrack
        )
        if (selectedRow >= 0) {
            if (hasVideo || hasAudio) {
                requestPreview(name, filePath, safeDuration, hasVideo, safeStart, 0)
            }
            seekRequested(safeStart)
        }
        timelinePosition = safeStart
        scrubPosition = safeStart
        scrubbing = false
        Qt.callLater(function() {
            centerOnSeconds(safeStart + Math.min(1.0, safeDuration * 0.02))
            keepPlayheadVisible()
        })
        return selectedRow
    }

    function deleteSelectedClip() {
        deleteClipAt(backend.selectedClipIndex, "")
    }

    function deleteClipAt(index, filePath) {
        const deletedPath = filePath.length > 0 ? filePath : backend.clipFilePath(index)
        const removed = linkedSelection
            ? backend.deleteLinkedClip(index)
            : backend.deleteClip(index)



        if(removed && deletedPath === previewFilePath) {
            previewFilePath = ""
            previewCleared()
        }
    }

    function removeMediaClips(filePath) {
        const removedCount = backend.removeClipsByFilePath(filePath)
        if (removedCount > 0 && filePath === previewFilePath) {
            previewFilePath = ""
            previewCleared()
        }
        return removedCount
    }

    function splitClipAt(index, seconds, linked) {
        const splitRow = backend.splitClip(index, seconds, linked)
        if (splitRow >= 0) {
            timelinePosition = seconds
            Qt.callLater(function() { centerOnSeconds(seconds) })
        }
    }

    function moveClipAt(index, startSeconds, trackIndex, linked) {
        backend.moveClip(index, startSeconds, trackIndex, linked)
    }

    function addMarkerAtCurrentTime() {
        markers = markers.concat([visiblePosition])
    }

    function queueSeek(seconds) {
        scrubPosition = Math.max(0, seconds)
        scrubbing = true
        timelinePosition = scrubPosition
        seekDebounce.restart()
    }

    function commitSeek(seconds) {
        scrubPosition = Math.max(0, seconds)
        timelinePosition = scrubPosition
        scrubbing = false
        seekDebounce.stop()
        seekRequested(scrubPosition)
        keepPlayheadVisible()
    }

    function formatTime(seconds) {
        const fps = 25
        const totalFrames = Math.max(0, Math.floor(seconds * fps))
        const frames = totalFrames % fps
        const totalSeconds = Math.floor(totalFrames / fps)
        const secs = totalSeconds % 60
        const mins = Math.floor(totalSeconds / 60) % 60
        const hours = Math.floor(totalSeconds / 3600)
        return String(hours).padStart(2, "0") + ":"
            + String(mins).padStart(2, "0") + ":"
            + String(secs).padStart(2, "0") + ":"
            + String(frames).padStart(2, "0")
    }

    onContentWidthChanged: Qt.callLater(clampScroll)
    onMaxScrollChanged: Qt.callLater(clampScroll)
    onWidthChanged: Qt.callLater(clampScroll)
    onPlaybackPositionChanged: syncPlaybackPosition()

    Timer {
        id: seekDebounce
        interval: 80
        repeat: false
        onTriggered: tracksRoot.seekRequested(tracksRoot.scrubPosition)
    }

    TimelineSettingsMenu {
        id: timelineSettings
    }

    function clipIndexUnderPlayhead() {
        if (backend.selectedClipIndex >= 0 && backend.clipContains(backend.selectedClipIndex, tracksRoot.timelinePosition)) {
            return backend.selectedClipIndex;
        }
        for (let i = backend.clipCount - 1; i >= 0; i--) {
            if (backend.clipContains(i, tracksRoot.timelinePosition)) {
                return i;
            }
        }
        return -1;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        TimelineHeader {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            panelLine: tracksRoot.panelLine
            textPrimary: tracksRoot.textPrimary
            textMuted: tracksRoot.textMuted
            activeTool: tracksRoot.activeTool
            snapEnabled: tracksRoot.snapEnabled
            zoomValue: tracksRoot.zoomValue
            
            property int activeClipIndex: backend.selectedClipIndex >= 0 ? backend.selectedClipIndex : tracksRoot.clipIndexUnderPlayhead()
            
            hasSelection: backend.selectedClipIndex >= 0 || activeClipIndex >= 0
            playheadOverSelection: activeClipIndex >= 0
            
            onToolSelected: function(tool) { tracksRoot.toolSelected(tool) }
            onSnapToggled: function(enabled) { tracksRoot.snapToggleRequested(enabled) }
            onMarkerRequested: tracksRoot.addMarkerAtCurrentTime()
            onZoomInRequested: tracksRoot.zoomIn()
            onZoomOutRequested: tracksRoot.zoomOut()
            onZoomValueRequested: function(value) { tracksRoot.setZoom(value, 0) }
            // qmllint disable unqualified
            onUndoRequested: { if (typeof ActionManager !== "undefined") ActionManager.executeAction("edit.undo") }
            onRedoRequested: { if (typeof ActionManager !== "undefined") ActionManager.executeAction("edit.redo") }
            // qmllint enable unqualified
            onSplitRequested: {    
                if (activeClipIndex >= 0) {
                    tracksRoot.splitClipAt(activeClipIndex, tracksRoot.timelinePosition, tracksRoot.linkedSelection)
                }
            }
            onDeleteLeftRequested: {
                if (activeClipIndex >= 0) {
                    const clipStart = backend.clipStartSeconds(activeClipIndex)
                    if (clipStart >= 0 && tracksRoot.timelinePosition > clipStart) {
                        const delta = tracksRoot.timelinePosition - clipStart
                        trimController.trimClipLeft(backend, activeClipIndex, delta)
                    }
                }
            }
            onDeleteRightRequested: {
                if (activeClipIndex >= 0) {
                    const startSec = backend.clipStartSeconds(activeClipIndex)
                    const durationSec = backend.clipDurationSeconds(activeClipIndex)
                    const clipEnd = startSec + durationSec
                    if (clipEnd >= 0 && tracksRoot.timelinePosition < clipEnd) {
                        const delta = clipEnd - tracksRoot.timelinePosition
                        trimController.trimClipRight(backend, activeClipIndex, delta)
                    }
                }
            }
            onDeleteRequested: {
                if (backend.selectedClipIndex >= 0) {
                    tracksRoot.deleteSelectedClip()
                } else if (activeClipIndex >= 0) {
                    tracksRoot.deleteClipAt(activeClipIndex, "")
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            TimelineTrackHeaders {
                id: trackHeaders
                Layout.preferredWidth: tracksRoot.trackHeaderWidth
                Layout.fillHeight: true
                vScrollOffset: tracksRoot.vScrollOffset
                trackHeight: tracksRoot.trackHeight
                markerHeight: tracksRoot.markerHeight
                timecode: tracksRoot.formatTime(tracksRoot.visiblePosition)
                durationTimecode: tracksRoot.formatTime(backend.timelineEndSeconds)
                snapEnabled: tracksRoot.snapEnabled
                linkedSelection: tracksRoot.linkedSelection
                hasTimelineClips: backend.clipCount > 0
                panelLine: tracksRoot.panelLine
                textPrimary: tracksRoot.textPrimary
                accent: tracksRoot.accent
                onSnapToggled: tracksRoot.snapToggleRequested(!tracksRoot.snapEnabled)
                onLinkedToggled: tracksRoot.linkedSelectionToggleRequested(!tracksRoot.linkedSelection)
                onMarkerRequested: tracksRoot.addMarkerAtCurrentTime()
                onSettingsRequested: function(x, y) {
                    const point = trackHeaders.mapToItem(tracksRoot, x, y)
                    timelineSettings.x = point.x
                    timelineSettings.y = point.y
                    timelineSettings.open()
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                TimelineRuler {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    pixelsPerSecond: tracksRoot.pixelsPerSecond
                    scrollOffset: tracksRoot.scrollOffset
                    contentWidth: tracksRoot.contentWidth
                    playheadSeconds: tracksRoot.visiblePosition
                    panelLine: tracksRoot.panelLine
                    textMuted: tracksRoot.textMuted
                    accent: tracksRoot.accent
                    onSeekPreview: function(seconds) { tracksRoot.queueSeek(seconds) }
                    onSeekCommitted: function(seconds) { tracksRoot.commitSeek(seconds) }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    TimelineCanvas {
                        id: timelineViewport
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        vScrollOffset: tracksRoot.vScrollOffset
                        clipModel: backend.clipModel
                        selectedClipIndex: backend.selectedClipIndex
                        pixelsPerSecond: tracksRoot.pixelsPerSecond
                        scrollOffset: tracksRoot.scrollOffset
                        contentWidth: tracksRoot.contentWidth
                        playheadSeconds: tracksRoot.visiblePosition
                        trackHeight: tracksRoot.trackHeight
                        markerHeight: tracksRoot.markerHeight
                        separatorHeight: tracksRoot.separatorHeight
                        minClipWidth: tracksRoot.minClipWidth
                        activeTool: tracksRoot.activeTool
                        linkedSelection: tracksRoot.linkedSelection
                        snapEnabled: tracksRoot.snapEnabled
                        markers: tracksRoot.markers
                        hasClips: backend.clipCount > 0

                        onSelectionCleared: backend.selectedClipIndex = -1
                        onClipSelected: function(index) { backend.selectedClipIndex = index }
                        onPreviewRequested: function(name, filePath, duration, hasVideo) {
                            const clipIdx = backend.selectedClipIndex
                            const clipStart = clipIdx >= 0 ? backend.clipStartSeconds(clipIdx) : 0
                            const clipData = clipIdx >= 0 ? backend.clipAt(clipIdx) : null
                            const inPoint = clipData ? (clipData.sourceInPoint || 0) : 0
                            tracksRoot.requestPreview(name, filePath, duration, hasVideo, clipStart, inPoint)
                        }
                        onClipDeleted: function(index, filePath) { tracksRoot.deleteClipAt(index, filePath) }
                        onClipSplit: function(index, seconds, linked) { tracksRoot.splitClipAt(index, seconds, linked) }
                        onClipMoved: function(index, startSeconds, trackIndex, linked) {
                            backend.moveClip(index, startSeconds, trackIndex, linked)
                        }
                        onTrimLeftRequested: function(index, deltaSeconds, linked) {
                            trimController.trimClipLeft(backend, index, deltaSeconds)
                        }
                        onTrimRightRequested: function(index, deltaSeconds, linked) {
                            trimController.trimClipRight(backend, index, deltaSeconds)
                        }
                        onSeekPreview: function(seconds) { tracksRoot.queueSeek(seconds) }
                        onSeekCommitted: function(seconds) { tracksRoot.commitSeek(seconds) }
                        onPanRequested: function(deltaPixels) { tracksRoot.setScrollOffset(tracksRoot.scrollOffset + deltaPixels) }
                        onZoomRequested: function(anchorX, direction) {
                            tracksRoot.setZoom(tracksRoot.zoomValue + direction * 0.08, anchorX)
                        }
                        onMediaDropped: function(name, filePath, duration, hasVideo, hasAudio, startSeconds, trackIndex) {
                            tracksRoot.addMediaClip(name, filePath, duration, hasVideo, hasAudio, startSeconds, trackIndex)
                            timelineViewport.forceActiveFocus()
                        }
                        onSubtitleDropped: function(filePath, startSeconds, trackIndex) {
                            tracksRoot.subtitleDropped(filePath, startSeconds, trackIndex)
                            timelineViewport.forceActiveFocus()
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 15
                        Layout.fillHeight: true
                        color: Theme.surfaceInset
                        border.color: tracksRoot.panelLine
                        border.width: 1

                        Slider {
                            id: vScrollSlider
                            anchors.fill: parent
                            anchors.topMargin: 4
                            anchors.bottomMargin: 4
                            orientation: Qt.Vertical
                            from: 0
                            to: Math.max(1, tracksRoot.maxVScroll)
                            value: tracksRoot.vScrollOffset
                            enabled: tracksRoot.maxVScroll > 0
                            onMoved: tracksRoot.setVScrollOffset(value)
                            background: Item {}
                            handle: Rectangle {
                                x: parent.width / 2 - width / 2
                                y: vScrollSlider.topPadding + vScrollSlider.visualPosition * (vScrollSlider.availableHeight - height)
                                width: 7
                                height: tracksRoot.maxVScroll <= 0
                                    ? vScrollSlider.availableHeight
                                    : Math.max(40, vScrollSlider.availableHeight * Math.min(1, timelineViewport.height / tracksRoot.contentHeight))
                                radius: 4
                                color: vScrollSlider.pressed ? "#758a94" : vScrollSlider.hovered ? "#607682" : "#52656e"
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 17
                    color: Theme.surfaceInset
                    border.color: tracksRoot.panelLine
                    border.width: 1

                    Slider {
                        id: scrollSlider
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        from: 0
                        to: Math.max(1, tracksRoot.maxScroll)
                        value: tracksRoot.scrollOffset
                        enabled: tracksRoot.maxScroll > 0
                        onMoved: tracksRoot.setScrollOffset(value)

                        background: Item {}

                        handle: Rectangle {
                            x: scrollSlider.leftPadding + scrollSlider.visualPosition * (scrollSlider.availableWidth - width)
                            anchors.verticalCenter: parent.verticalCenter
                            width: tracksRoot.maxScroll <= 0
                                ? scrollSlider.availableWidth
                                : Math.max(70, scrollSlider.availableWidth * Math.min(1, timelineViewport.width / tracksRoot.contentWidth))
                            height: 7
                            radius: 4
                            color: scrollSlider.pressed ? "#758a94" : scrollSlider.hovered ? "#607682" : "#52656e"
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: tracksRoot.timelineController
        function onTimelineChanged() {
            Qt.callLater(tracksRoot.clampScroll)
        }
    }
}

