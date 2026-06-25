pragma ComponentBehavior: Bound

import QtQuick

import VideoStudioUI

Rectangle {
    id: canvasRoot

    property var clipModel
    property var timelineBackend
    property var selectedClipIndices: []
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
    property real dragDeltaSeconds: 0
    property int dragDeltaTrack: 0
    property bool isDraggingClip: false
    property real activeSnapLineSeconds: -1
    property int _trackStateRevision: 0

    Connections {
        target: canvasRoot.timelineBackend
        function onTrackStateChanged() {
            canvasRoot._trackStateRevision++;
        }
    }

    // Mouse interaction properties
    property real dragStartX: 0
    property real dragStartY: 0
    property real dragCurrentX: 0
    property real dragCurrentY: 0
    property real dragLastX: 0
    property bool isDraggingPlayhead: false
    property bool isDraggingBox: false

    function getSnapTarget(targetTime) {
        if (!snapEnabled || !timelineBackend)
            return -1;
        const snapPoints = timelineBackend.getSnapPoints();
        const threshold = 15 / Math.max(1, pixelsPerSecond); // 15 pixels threshold
        let bestPoint = -1;
        let minDiff = threshold;

        for (let i = 0; i < snapPoints.length; i++) {
            const diff = Math.abs(targetTime - snapPoints[i]);
            if (diff < minDiff) {
                minDiff = diff;
                bestPoint = snapPoints[i];
            }
        }
        return bestPoint;
    }
    property var pendingDropQueue: []

    Timer {
        id: dropQueueTimer
        interval: 16
        repeat: true
        onTriggered: {
            if (canvasRoot.pendingDropQueue.length === 0) {
                stop();
                return;
            }
            // Create a copy to mutate to trigger bindings correctly
            let queue = canvasRoot.pendingDropQueue;
            let chunk = queue.splice(0, 5);
            canvasRoot.pendingDropQueue = queue; // update property

            for (let j = 0; j < chunk.length; j++) {
                let item = chunk[j];
                if (item.isSrt) {
                    canvasRoot.subtitleDropped(item.filePath, item.start, item.track);
                } else {
                    canvasRoot.mediaDropped(item.name, item.filePath, item.duration, item.hasVideo, item.hasAudio, item.start, item.track);
                }
            }
        }
    }
    readonly property real playheadX: playheadSeconds * pixelsPerSecond - scrollOffset
    readonly property real visibleDurationSeconds: Math.max(1, width / Math.max(0.001, pixelsPerSecond))
    readonly property real majorStepSeconds: chooseMajorStepSeconds(visibleDurationSeconds)
    readonly property int minorDivisions: chooseMinorDivisions(majorStepSeconds)
    readonly property real minorStepSeconds: majorStepSeconds / Math.max(1, minorDivisions)

    signal selectionCleared
    signal clipSelected(int index)
    signal selectionUpdated(var indices)
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
    signal effectDropped(string name, string filePath, real startSeconds, int trackIndex)
    signal trimLeftRequested(int index, real deltaSeconds, bool linked)
    signal trimRightRequested(int index, real deltaSeconds, bool linked)
    signal deleteSelectedClipsRequested

    color: Theme.background
    clip: true
    focus: true

    function chooseMajorStepSeconds(visibleSeconds) {
        const targetLabels = width >= 1200 ? 8 : 6;
        const minimumSeconds = Math.max(1, visibleSeconds / targetLabels);
        const steps = [1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 1200, 1800, 3600, 7200, 8200, 10000];
        for (let i = 0; i < steps.length; ++i) {
            if (steps[i] >= minimumSeconds)
                return steps[i];
        }
        return steps[steps.length - 1];
    }

    function chooseMinorDivisions(majorSeconds) {
        if (majorSeconds >= 600)
            return 10;
        if (majorSeconds >= 300)
            return 5;
        if (majorSeconds >= 60)
            return 6;
        if (majorSeconds >= 10)
            return 5;
        return Math.max(1, majorSeconds);
    }

    function secondsFromX(x) {
        return Math.max(0, (x + scrollOffset) / Math.max(0.001, pixelsPerSecond));
    }

    function isEditTool() {
        return activeTool === "selection" || activeTool === "ripple" || activeTool === "slip";
    }

    function trackY(trackIndex) {
        let vCount = timelineBackend ? timelineBackend.videoTrackCount : 3;
        if (trackIndex < 100) {
            return trackIndex * trackHeight - vScrollOffset;
        } else {
            return vCount * trackHeight + separatorHeight + (trackIndex - 100) * trackHeight - vScrollOffset;
        }
    }

    function trackIndexFromY(y, hasVideo) {
        let vCount = timelineBackend ? timelineBackend.videoTrackCount : 1;
        let aCount = timelineBackend ? timelineBackend.audioTrackCount : 1;
        let videoRegionHeight = vCount * trackHeight;
        let adjustedY = y + vScrollOffset;

        if (adjustedY < 0) {
            if (hasVideo === false)
                return 100;
            return vCount; // New top overlay track
        } else if (adjustedY < videoRegionHeight) {
            let visualIdx = Math.floor(adjustedY / trackHeight);
            if (hasVideo === false)
                return 100; // force to first audio
            return vCount - 1 - visualIdx;
        } else {
            let audioY = adjustedY - videoRegionHeight - separatorHeight;
            let aIdx = Math.floor(Math.max(0, audioY) / trackHeight);
            if (hasVideo === true)
                return 0; // force to main video track
            return 100 + aIdx;
        }
    }

    Keys.onDeletePressed: canvasRoot.deleteSelectedClipsRequested()

    Repeater {
        model: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 3

        Rectangle {
            id: trackDelegate
            required property int index

            y: canvasRoot.trackY(trackDelegate.index)
            width: canvasRoot.width
            height: canvasRoot.trackHeight
            color: "transparent"
            border.width: 0
            visible: canvasRoot.hasClips

            property bool hasClipsInTrack: false

            function updateHasClips() {
                if (canvasRoot.timelineBackend) {
                    hasClipsInTrack = !canvasRoot.timelineBackend.isTrackEmpty(true, trackDelegate.index);
                } else {
                    hasClipsInTrack = false;
                }
            }

            Connections {
                target: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.clipModel : null
                function onDataChanged() {
                    trackDelegate.updateHasClips();
                }
                function onRowsInserted() {
                    trackDelegate.updateHasClips();
                }
                function onRowsRemoved() {
                    trackDelegate.updateHasClips();
                }
                function onModelReset() {
                    trackDelegate.updateHasClips();
                }
            }

            Component.onCompleted: updateHasClips()

            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                color: "#181A20" // Box-background for all video tracks
                visible: trackDelegate.hasClipsInTrack
                radius: 4
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: '#222222'
                opacity: 0.45
            }
        }
    }

    Rectangle {
        y: (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 3) * canvasRoot.trackHeight - canvasRoot.vScrollOffset
        width: canvasRoot.width
        height: canvasRoot.separatorHeight
        color: Theme.surfaceInset
        border.width: 0
        visible: canvasRoot.hasClips
    }

    Repeater {
        model: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 3

        Rectangle {
            required property int index

            y: canvasRoot.trackY(100 + index)
            width: canvasRoot.width
            height: canvasRoot.trackHeight
            color: "transparent"
            border.width: 0
            visible: canvasRoot.hasClips

            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                color: "#181A20" // Box-background for all audio tracks
                visible: canvasRoot.timelineBackend && !canvasRoot.timelineBackend.isTrackEmpty(false, index)
                radius: 4
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: '#363535'
                opacity: 0.45
            }
        }
    }

    Rectangle {
        y: canvasRoot.trackY(100 + (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount - 1 : 2)) + canvasRoot.trackHeight
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

        hoverEnabled: true
        cursorShape: canvasRoot.isDraggingPlayhead || (!pressed && Math.abs(mouseX - canvasRoot.playheadX) < 10) ? Qt.SizeHorCursor : (canvasRoot.activeTool === "hand" ? (pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor) : (canvasRoot.activeTool === "zoom" ? Qt.PointingHandCursor : Qt.ArrowCursor))

        onPressed: function (mouse) {
            canvasRoot.forceActiveFocus();
            canvasRoot.dragLastX = mouse.x;
            if (canvasRoot.activeTool === "hand")
                return;
            if (canvasRoot.activeTool === "zoom") {
                canvasRoot.zoomRequested(mouse.x, mouse.button === Qt.RightButton ? -1 : 1);
                return;
            }
            if (canvasRoot.isEditTool()) {
                if (Math.abs(mouse.x - canvasRoot.playheadX) < 10) {
                    canvasRoot.isDraggingPlayhead = true;
                    canvasRoot.seekPreview(canvasRoot.secondsFromX(mouse.x));
                    return;
                }

                canvasRoot.dragStartX = mouse.x;
                canvasRoot.dragStartY = mouse.y;
                canvasRoot.dragCurrentX = mouse.x;
                canvasRoot.dragCurrentY = mouse.y;
                canvasRoot.isDraggingBox = false;

                if (!(mouse.modifiers & Qt.ControlModifier)) {
                    canvasRoot.selectionCleared();
                }

                // Seek logic if they didn't drag
                canvasRoot.seekPreview(canvasRoot.secondsFromX(mouse.x));
            }
        }

        onPositionChanged: function (mouse) {
            if (!pressed)
                return;
            if (canvasRoot.activeTool === "hand") {
                canvasRoot.panRequested(canvasRoot.dragLastX - mouse.x);
                canvasRoot.dragLastX = mouse.x;
                return;
            }
            if (canvasRoot.isEditTool()) {
                if (canvasRoot.isDraggingPlayhead) {
                    canvasRoot.seekPreview(canvasRoot.secondsFromX(mouse.x));
                    return;
                }

                canvasRoot.dragCurrentX = mouse.x;
                canvasRoot.dragCurrentY = mouse.y;
                if (!canvasRoot.isDraggingBox && (Math.abs(canvasRoot.dragCurrentX - canvasRoot.dragStartX) > 4 || Math.abs(canvasRoot.dragCurrentY - canvasRoot.dragStartY) > 4)) {
                    canvasRoot.isDraggingBox = true;
                }

                if (canvasRoot.isDraggingBox && canvasRoot.timelineBackend) {
                    let minX = Math.min(canvasRoot.dragStartX, canvasRoot.dragCurrentX);
                    let maxX = Math.max(canvasRoot.dragStartX, canvasRoot.dragCurrentX);
                    let minY = Math.min(canvasRoot.dragStartY, canvasRoot.dragCurrentY);
                    let maxY = Math.max(canvasRoot.dragStartY, canvasRoot.dragCurrentY);

                    let minTrack = canvasRoot.trackIndexFromY(minY, null);
                    let maxTrack = canvasRoot.trackIndexFromY(maxY, null);
                    let startSec = canvasRoot.secondsFromX(minX);
                    let endSec = canvasRoot.secondsFromX(maxX);

                    let newSelection = [];
                    for (let i = 0; i < canvasRoot.timelineBackend.clipCount; i++) {
                        let cStart = canvasRoot.timelineBackend.clipStartSeconds(i);
                        let cEnd = canvasRoot.timelineBackend.clipEndSeconds(i);
                        let cTrack = canvasRoot.timelineBackend.clipAt(i).trackIndex;

                        if (cTrack >= minTrack && cTrack <= maxTrack && cEnd >= startSec && cStart <= endSec) {
                            newSelection.push(i);
                        }
                    }
                    if (mouse.modifiers & Qt.ControlModifier) {
                        let combined = canvasRoot.selectedClipIndices.slice();
                        for (let i = 0; i < newSelection.length; i++) {
                            if (!combined.includes(newSelection[i])) {
                                combined.push(newSelection[i]);
                            }
                        }
                        canvasRoot.selectionUpdated(combined);
                    } else {
                        canvasRoot.selectionUpdated(newSelection);
                    }
                } else {
                    canvasRoot.seekPreview(canvasRoot.secondsFromX(mouse.x));
                }
            }
        }

        onReleased: function (mouse) {
            if (canvasRoot.isDraggingPlayhead) {
                canvasRoot.isDraggingPlayhead = false;
                canvasRoot.seekCommitted(canvasRoot.secondsFromX(mouse.x));
            } else if (canvasRoot.isDraggingBox) {
                canvasRoot.isDraggingBox = false;
            } else if (canvasRoot.isEditTool()) {
                canvasRoot.seekCommitted(canvasRoot.secondsFromX(mouse.x));
            }
        }
    }

    Rectangle {
        id: timelineRubberBand
        x: Math.min(canvasRoot.dragStartX, canvasRoot.dragCurrentX)
        y: Math.min(canvasRoot.dragStartY, canvasRoot.dragCurrentY)
        width: Math.abs(canvasRoot.dragCurrentX - canvasRoot.dragStartX)
        height: Math.abs(canvasRoot.dragCurrentY - canvasRoot.dragStartY)
        color: "#1A58a8d8"
        border.color: "#58a8d8"
        border.width: 1
        visible: canvasRoot.isDraggingBox
        z: 99
    }

    Rectangle {
        id: snapLine
        x: canvasRoot.activeSnapLineSeconds >= 0 ? canvasRoot.activeSnapLineSeconds * canvasRoot.pixelsPerSecond - canvasRoot.scrollOffset : 0
        y: 0
        width: 1
        height: parent.height
        color: "#5ec4e8"
        visible: canvasRoot.activeSnapLineSeconds >= 0
        z: 100

        Rectangle {
            anchors.centerIn: parent
            width: 3
            height: parent.height
            color: "#5ec4e8"
            opacity: 0.3
        }
    }

    DropArea {
        id: timelineDropArea
        anchors.fill: parent
        keys: ["videoStudio/media"]
        z: 2

        function mediaFromDrop(drop) {
            if (!drop || !drop.source)
                return null;
            if (drop.source.mediaFilePath)
                return drop.source;
            if (drop.source.sourceMedia && drop.source.sourceMedia.mediaFilePath)
                return drop.source.sourceMedia;
            return null;
        }

        onEntered: function (drag) {
            drag.acceptProposedAction();
        }

        onPositionChanged: function (drag) {
            let targetSec = canvasRoot.secondsFromX(drag.x);
            let snapped = canvasRoot.getSnapTarget(targetSec);
            canvasRoot.activeSnapLineSeconds = snapped;
        }

        onExited: function () {
            canvasRoot.activeSnapLineSeconds = -1;
        }

        onDropped: function (drop) {
            let mediaList = [];
            if (drop.source && drop.source["effectTitle"]) {
                let effectStart = canvasRoot.timelineBackend && canvasRoot.timelineBackend.clipCount === 0 ? 0 : canvasRoot.secondsFromX(drop.x);
                if (canvasRoot.activeSnapLineSeconds >= 0) {
                    effectStart = canvasRoot.activeSnapLineSeconds;
                }
                canvasRoot.activeSnapLineSeconds = -1;
                canvasRoot.effectDropped(drop.source["effectTitle"], drop.source["effectFilePath"], effectStart, canvasRoot.trackIndexFromY(drop.y, true));
                drop.acceptProposedAction();
                return;
            }

            if (drop.source && drop.source["selectedMediaList"] && drop.source["selectedMediaList"].length > 0) {
                mediaList = drop.source["selectedMediaList"];
            } else {
                const media = mediaFromDrop(drop);
                if (media && media.mediaFilePath) {
                    mediaList = [
                        {
                            mediaName: media.mediaName,
                            mediaFilePath: media.mediaFilePath,
                            mediaDuration: media.mediaDuration,
                            mediaHasVideo: media.mediaHasVideo,
                            mediaHasAudio: media.mediaHasAudio
                        }
                    ];
                }
            }

            if (mediaList.length === 0) {
                drop.acceptProposedAction();
                return;
            }

            let isTimelineEmpty = canvasRoot.timelineBackend && canvasRoot.timelineBackend.clipCount === 0;
            let currentStart = canvasRoot.secondsFromX(drop.x);

            if (isTimelineEmpty) {
                currentStart = 0;
            } else if (canvasRoot.activeSnapLineSeconds >= 0) {
                currentStart = canvasRoot.activeSnapLineSeconds;
            }

            canvasRoot.activeSnapLineSeconds = -1; // hide snap line

            // Build the queue of drop operations
            let newQueueItems = [];
            for (let i = 0; i < mediaList.length; i++) {
                let m = mediaList[i];
                if (m.mediaFilePath.toLowerCase().endsWith(".srt")) {
                    newQueueItems.push({
                        isSrt: true,
                        filePath: m.mediaFilePath,
                        start: currentStart,
                        track: canvasRoot.trackIndexFromY(drop.y, false)
                    });
                    currentStart += 5;
                } else {
                    let targetTrack = canvasRoot.trackIndexFromY(drop.y, m.mediaHasVideo);
                    if (isTimelineEmpty) {
                        let vCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 3;
                        targetTrack = m.mediaHasVideo ? vCount - 1 : vCount; // Main track or first audio track
                    }
                    let isImage = m.mediaHasVideo && (!m.mediaHasAudio || m.mediaDuration <= 0.1) && (m.mediaFilePath.toLowerCase().endsWith(".jpg") || m.mediaFilePath.toLowerCase().endsWith(".jpeg") || m.mediaFilePath.toLowerCase().endsWith(".png") || m.mediaFilePath.toLowerCase().endsWith(".webp") || m.mediaFilePath.toLowerCase().endsWith(".bmp") || m.mediaFilePath.toLowerCase().endsWith(".gif"));

                    let clipDuration = isImage ? 5.0 : (m.mediaDuration > 0.01 ? m.mediaDuration : 5.0);

                    newQueueItems.push({
                        isSrt: false,
                        name: m.mediaName,
                        filePath: m.mediaFilePath,
                        duration: clipDuration,
                        hasVideo: m.mediaHasVideo,
                        hasAudio: m.mediaHasAudio,
                        start: currentStart,
                        track: targetTrack
                    });
                    currentStart += clipDuration;
                }
            }

            canvasRoot.pendingDropQueue = canvasRoot.pendingDropQueue.concat(newQueueItems);
            dropQueueTimer.start();

            drop.acceptProposedAction();
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
                id: delegateRoot
                required property int index
                required property string clipName
                required property string filePath
                required property string originalFilePath
                required property real startSeconds
                required property real durationSeconds
                required property real sourceInPoint
                required property real sourceDuration
                required property int trackIndex
                required property bool hasVideo
                required property bool hasAudio
                required property int vocalIsolationType
                required property int isolationProgress
                readonly property bool clipSelected: canvasRoot.selectedClipIndices.includes(index)
                readonly property bool isSubtitle: !hasVideo && !hasAudio
                readonly property bool isDragging: canvasRoot.isDraggingClip && clipSelected
                readonly property real effectiveDragSec: isDragging ? canvasRoot.dragDeltaSeconds : 0
                readonly property int effectiveDragTrack: isDragging ? canvasRoot.dragDeltaTrack : 0

                // Layer 2: The Placeholder Background Box (Drop Indicator)
                // This stays behind when the clip is dragged
                Rectangle {
                    x: delegateRoot.startSeconds * canvasRoot.pixelsPerSecond
                    y: {
                        let t = delegateRoot.trackIndex;
                        let vCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 3;
                        let aCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 3;
                        if (t < 100) {
                            t = Math.max(0, Math.min(vCount - 1, t));
                        } else {
                            t = Math.max(100, Math.min(100 + aCount - 1, t));
                        }
                        return canvasRoot.trackY(t) + (delegateRoot.isSubtitle ? canvasRoot.trackHeight - 20 - 4 : 4);
                    }
                    width: Math.max(delegateRoot.isSubtitle ? 24 : 1, delegateRoot.durationSeconds * canvasRoot.pixelsPerSecond)
                    height: delegateRoot.isSubtitle ? 20 : canvasRoot.trackHeight - 8
                    radius: 4
                    color: Theme.surfaceRaised
                    visible: delegateRoot.isDragging // Only show the empty hole when dragging
                    z: delegateRoot.clipSelected ? 2 : 1
                }

                // Layer 3: The actual Media Content Clip
                // This moves when dragged
                TimelineClipItem {
                    clipIndex: delegateRoot.index
                    clipName: delegateRoot.clipName
                    filePath: delegateRoot.filePath
                    originalFilePath: delegateRoot.originalFilePath
                    startSeconds: delegateRoot.startSeconds
                    durationSeconds: delegateRoot.durationSeconds
                    sourceInPoint: delegateRoot.sourceInPoint
                    sourceDuration: delegateRoot.sourceDuration
                    trackIndex: delegateRoot.trackIndex
                    hasVideo: delegateRoot.hasVideo
                    hasAudio: delegateRoot.hasAudio
                    selected: delegateRoot.clipSelected
                    vocalIsolationType: delegateRoot.vocalIsolationType
                    isolationProgress: delegateRoot.isolationProgress

                    x: (delegateRoot.startSeconds + delegateRoot.effectiveDragSec) * canvasRoot.pixelsPerSecond
                    y: {
                        let t = delegateRoot.trackIndex + delegateRoot.effectiveDragTrack;
                        let vCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 3;
                        let aCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 3;
                        if (t < 100) {
                            t = Math.max(0, Math.min(vCount - 1, t));
                        } else {
                            t = Math.max(100, Math.min(100 + aCount - 1, t));
                        }
                        return canvasRoot.trackY(t) + (delegateRoot.isSubtitle ? canvasRoot.trackHeight - 20 - 4 : 4);
                    }
                    width: Math.max(delegateRoot.isSubtitle ? 24 : 1, delegateRoot.durationSeconds * canvasRoot.pixelsPerSecond)
                    height: delegateRoot.isSubtitle ? 20 : canvasRoot.trackHeight - 8
                    z: delegateRoot.clipSelected ? 4 : 3

                    pixelsPerSecond: canvasRoot.pixelsPerSecond
                    trackHeight: canvasRoot.trackHeight
                    separatorHeight: canvasRoot.separatorHeight
                    minClipWidth: canvasRoot.minClipWidth
                    activeTool: canvasRoot.activeTool
                    linkedSelection: canvasRoot.linkedSelection
                    snapEnabled: canvasRoot.snapEnabled
                    dragOffsetSeconds: canvasRoot.isDraggingClip && delegateRoot.clipSelected ? canvasRoot.dragDeltaSeconds : 0
                    dragOffsetTrack: canvasRoot.isDraggingClip && delegateRoot.clipSelected ? canvasRoot.dragDeltaTrack : 0
                    isTrackLocked: canvasRoot._trackStateRevision >= 0 && canvasRoot.timelineBackend ? canvasRoot.timelineBackend.isTrackLocked(delegateRoot.hasVideo, delegateRoot.trackIndex >= 100 ? delegateRoot.trackIndex - 100 : delegateRoot.trackIndex) : false
                    isTrackHidden: canvasRoot._trackStateRevision >= 0 && canvasRoot.timelineBackend ? canvasRoot.timelineBackend.isTrackHidden(delegateRoot.hasVideo, delegateRoot.trackIndex >= 100 ? delegateRoot.trackIndex - 100 : delegateRoot.trackIndex) : false

                    onDragStarted: function () {
                        canvasRoot.isDraggingClip = true;
                        canvasRoot.dragDeltaSeconds = 0;
                        canvasRoot.dragDeltaTrack = 0;
                    }
                    onDragUpdated: function (deltaSec, deltaTrack) {
                        let targetSec = delegateRoot.startSeconds + deltaSec;
                        let snapped = canvasRoot.getSnapTarget(targetSec);
                        if (snapped >= 0) {
                            canvasRoot.dragDeltaSeconds = snapped - delegateRoot.startSeconds;
                            canvasRoot.activeSnapLineSeconds = snapped;
                        } else {
                            canvasRoot.dragDeltaSeconds = deltaSec;
                            canvasRoot.activeSnapLineSeconds = -1;
                        }
                        canvasRoot.dragDeltaTrack = deltaTrack;
                    }
                    onDragFinished: function () {
                        canvasRoot.isDraggingClip = false;
                        canvasRoot.dragDeltaSeconds = 0;
                        canvasRoot.dragDeltaTrack = 0;
                        canvasRoot.activeSnapLineSeconds = -1;
                    }

                    onSelectedRequested: function (selectedIndex, toggle) {
                        if (toggle) {
                            let current = canvasRoot.selectedClipIndices.slice();
                            let idx = current.indexOf(selectedIndex);
                            if (idx >= 0)
                                current.splice(idx, 1);
                            else
                                current.push(selectedIndex);
                            canvasRoot.selectionUpdated(current);
                        } else {
                            canvasRoot.clipSelected(selectedIndex);
                        }
                    }
                    onPreviewRequested: function (name, path, duration, video) {
                        canvasRoot.previewRequested(name, path, duration, video);
                    }
                    onDeleteRequested: function (selectedIndex, path) {
                        canvasRoot.clipDeleted(selectedIndex, path);
                    }
                    onSplitRequested: function (selectedIndex, seconds, linked) {
                        canvasRoot.clipSplit(selectedIndex, seconds, linked);
                    }
                    onMoveRequested: function (selectedIndex, startSecondsValue, trackIndexValue, linked) {
                        canvasRoot.clipMoved(selectedIndex, startSecondsValue, trackIndexValue, linked);
                    }
                    onTrimLeftRequested: function (selectedIndex, deltaSeconds, linked) {
                        canvasRoot.trimLeftRequested(selectedIndex, deltaSeconds, linked);
                    }
                    onTrimRightRequested: function (selectedIndex, deltaSeconds, linked) {
                        canvasRoot.trimRightRequested(selectedIndex, deltaSeconds, linked);
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
                color: Theme.text
                font.pixelSize: 13
                opacity: 0.88
            }
        }
    }
}
