pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick

import VideoStudioUI

Rectangle {
    id: canvasRoot

    property var clipModel
    property var timelineBackend
    property var dragController
    property var selectedClipIndices: []
    property real pixelsPerSecond: 18
    property real scrollOffset: 0
    property real vScrollOffset: 0
    property real contentWidth: width
    property real playheadSeconds: 0
    property int trackHeight: 34
    property int subtitleTrackHeight: 32
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
    property int dragMaxVideoTrackCount: 1
    property int dynamicVideoTrackCount: isDraggingClip ? dragMaxVideoTrackCount : (timelineBackend ? timelineBackend.videoTrackCount : 1)
    property real activeSnapLineSeconds: -1
    property int activeSnapTrackIndex: -1
    property real activeDragSnappedStart: -1
    property int _trackStateRevision: 0

    // Ghost Image properties
    property var activeDragMediaList: []
    property real activeDragX: 0
    property real activeDragY: 0

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
    signal trimSeekPreview(real seconds)
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
        let vCount = isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (timelineBackend ? timelineBackend.videoTrackCount : 1);
        let subOffset = (timelineBackend && timelineBackend.hasSubtitleTrack) ? 1 : 0;
        let subHeight = subOffset * subtitleTrackHeight;
        let effCount = timelineBackend ? timelineBackend.effectTrackCount : 0;
        let effHeight = effCount * subtitleTrackHeight;

        if (trackIndex >= 300) {
            // Effect tracks
            return subHeight + (trackIndex - 300) * subtitleTrackHeight - vScrollOffset;
        } else if (trackIndex >= 200) {
            // Subtitle tracks
            return 0 - vScrollOffset;
        } else if (trackIndex === vCount) {
            // Subtitle track index might be vCount
            return 0 - vScrollOffset;
        } else if (trackIndex < 100) {
            return subHeight + effHeight + (vCount - 1 - trackIndex) * trackHeight - vScrollOffset;
        } else {
            return subHeight + effHeight + vCount * trackHeight + separatorHeight + (trackIndex - 100) * trackHeight - vScrollOffset;
        }
    }

    function trackIndexFromY(y, hasVideo) {
        let vCount = isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (timelineBackend ? timelineBackend.videoTrackCount : 1);
        let subOffset = (timelineBackend && timelineBackend.hasSubtitleTrack) ? 1 : 0;
        let subHeight = subOffset * subtitleTrackHeight;
        let effCount = timelineBackend ? timelineBackend.effectTrackCount : 0;
        let effHeight = effCount * subtitleTrackHeight;
        let aCount = timelineBackend ? timelineBackend.audioTrackCount : 1;
        let videoRegionHeight = subHeight + effHeight + vCount * trackHeight;

        let adjustedY = y + vScrollOffset;

        if (adjustedY < 0) {
            if (hasVideo === false)
                return 100;
            return vCount; // New top overlay track
        } else if (adjustedY < subHeight) {
            return vCount; // Subtitle track area
        } else if (adjustedY < subHeight + effHeight) {
            let effIdx = Math.floor((adjustedY - subHeight) / subtitleTrackHeight);
            return 300 + Math.max(0, Math.min(effCount - 1, effIdx));
        } else if (adjustedY < videoRegionHeight) {
            let visualIdx = Math.floor((adjustedY - subHeight - effHeight) / trackHeight);
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
        model: (canvasRoot.timelineBackend && canvasRoot.timelineBackend.hasSubtitleTrack) ? 1 : 0
        Rectangle {
            y: canvasRoot.trackY(200) // 200 or vCount, using 200 to match trackY logic which catches >= 200
            width: canvasRoot.width
            height: canvasRoot.subtitleTrackHeight
            color: "transparent"
            border.width: 0
            visible: canvasRoot.hasClips
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 2
                anchors.bottomMargin: 2
                color: '#1a1a23'
                visible: true
                radius: 4
            }
        }
    }

    Repeater {
        model: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.effectTrackCount : 0
        Rectangle {
            required property int index
            y: canvasRoot.trackY(300 + index)
            width: canvasRoot.width
            height: canvasRoot.subtitleTrackHeight
            color: "transparent"
            border.width: 0
            visible: canvasRoot.hasClips
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 2
                anchors.bottomMargin: 2
                color: '#1a1a23'
                visible: true
                radius: 4
            }
        }
    }

    Repeater {
        model: canvasRoot.isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1)

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
                anchors.topMargin: 2
                anchors.bottomMargin: 2
                color: '#1a1a23' // Box-background for all video tracks
                visible: true
                radius: 4
            }
        }
    }

    Rectangle {
        y: (canvasRoot.isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1)) * canvasRoot.trackHeight + (canvasRoot.timelineBackend && canvasRoot.timelineBackend.hasSubtitleTrack ? canvasRoot.subtitleTrackHeight : 0) + (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.effectTrackCount * canvasRoot.subtitleTrackHeight : 0) - canvasRoot.vScrollOffset
        width: canvasRoot.width
        height: canvasRoot.separatorHeight
        color: Theme.surfaceInset
        border.width: 0
        visible: canvasRoot.hasClips
    }

    Repeater {
        model: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 1

        Rectangle {
            id: trackRect
            required property int index

            y: canvasRoot.trackY(100 + index)
            width: canvasRoot.width
            height: canvasRoot.trackHeight
            color: "transparent"
            border.width: 0
            visible: canvasRoot.hasClips

            property bool hasClipsInTrack: false

            function updateHasClips() {
                if (canvasRoot.timelineBackend) {
                    hasClipsInTrack = !canvasRoot.timelineBackend.isTrackEmpty(false, trackRect.index);
                } else {
                    hasClipsInTrack = false;
                }
            }

            Connections {
                target: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.clipModel : null
                function onDataChanged() {
                    trackRect.updateHasClips();
                }
                function onRowsInserted() {
                    trackRect.updateHasClips();
                }
                function onRowsRemoved() {
                    trackRect.updateHasClips();
                }
                function onModelReset() {
                    trackRect.updateHasClips();
                }
            }

            Component.onCompleted: updateHasClips()

            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 2
                anchors.bottomMargin: 2
                color: '#1a1a23' // Box-background for all audio tracks
                visible: trackRect.hasClipsInTrack
                radius: 4
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

    HoverHandler {
        id: canvasCursorHandler
        cursorShape: {
            if (canvasRoot.isDraggingPlayhead)
                return Qt.SizeAllCursor;
            if (Math.abs(point.position.x - canvasRoot.playheadX) < 10)
                return Qt.SizeAllCursor;
            if (canvasRoot.activeTool === "hand")
                return Qt.OpenHandCursor;
            if (canvasRoot.activeTool === "zoom")
                return Qt.PointingHandCursor;
            return Qt.ArrowCursor;
        }
    }

    MouseArea {
        id: canvasMouse
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        z: 1

        hoverEnabled: false

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
        y: canvasRoot.activeSnapTrackIndex >= 0 ? canvasRoot.trackY(canvasRoot.activeSnapTrackIndex) : 0
        width: 1
        height: canvasRoot.activeSnapTrackIndex >= 0 ? (parent.height - y) : parent.height
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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.topMargin: -100

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

        function getDraggedMediaDuration() {
            if (!canvasRoot.activeDragMediaList || canvasRoot.activeDragMediaList.length === 0)
                return 5.0;
            let total = 0;
            for (let i = 0; i < canvasRoot.activeDragMediaList.length; i++) {
                let m = canvasRoot.activeDragMediaList[i];
                let isSub = m.mediaFilePath ? m.mediaFilePath.toLowerCase().endsWith(".srt") : false;
                let isEff = m.isEffect !== undefined;
                let hasVid = m.mediaHasVideo !== undefined ? m.mediaHasVideo : false;
                let hasAud = m.mediaHasAudio !== undefined ? m.mediaHasAudio : false;
                let dur = 5.0;
                if (!isEff && !isSub) {
                    let isImg = hasVid && (!hasAud || m.mediaDuration <= 0.1) && m.mediaFilePath && (m.mediaFilePath.toLowerCase().endsWith(".jpg") || m.mediaFilePath.toLowerCase().endsWith(".png") || m.mediaFilePath.toLowerCase().endsWith(".webp") || m.mediaFilePath.toLowerCase().endsWith(".jpeg"));
                    dur = isImg ? 5.0 : (m.mediaDuration > 0.01 ? m.mediaDuration : 5.0);
                }
                total += dur;
            }
            return total;
        }

        onEntered: function (drag) {
            drag.acceptProposedAction();
            let list = [];
            if (drag.source && drag.source["effectTitle"]) {
                list.push({
                    isEffect: true,
                    duration: 2.0
                });
            } else if (drag.source && drag.source["selectedMediaList"] && drag.source["selectedMediaList"].length > 0) {
                list = drag.source["selectedMediaList"];
            } else {
                let mockDrop = {
                    source: drag.source
                };
                const media = timelineDropArea.mediaFromDrop(mockDrop);
                if (media && media.mediaFilePath) {
                    list.push({
                        mediaName: media.mediaName,
                        mediaFilePath: media.mediaFilePath,
                        mediaDuration: media.mediaDuration,
                        mediaHasVideo: media.mediaHasVideo,
                        mediaHasAudio: media.mediaHasAudio
                    });
                }
            }
            canvasRoot.activeDragMediaList = list;
            canvasRoot.activeDragX = drag.x;
            canvasRoot.activeDragY = drag.y - 100;
        }

        onPositionChanged: function (drag) {
            let targetSec = canvasRoot.secondsFromX(drag.x);
            let duration = timelineDropArea.getDraggedMediaDuration();
            let startSnapped = canvasRoot.getSnapTarget(targetSec);
            let endSnapped = canvasRoot.getSnapTarget(targetSec + duration);

            if (startSnapped >= 0) {
                canvasRoot.activeSnapLineSeconds = startSnapped;
                canvasRoot.activeDragSnappedStart = startSnapped;
            } else if (endSnapped >= 0 && endSnapped - duration >= 0) {
                canvasRoot.activeSnapLineSeconds = endSnapped;
                canvasRoot.activeDragSnappedStart = endSnapped - duration;
            } else {
                canvasRoot.activeSnapLineSeconds = -1;
                canvasRoot.activeDragSnappedStart = -1;
            }
            canvasRoot.activeDragX = drag.x;
            canvasRoot.activeDragY = drag.y - 100;
        }

        onExited: function () {
            canvasRoot.activeSnapLineSeconds = -1;
            canvasRoot.activeDragSnappedStart = -1;
            canvasRoot.activeDragMediaList = [];
        }

        // qmllint disable missing-property
        onDropped: function (drop) {
            let mediaList = [];
            if (drop.source && drop.source["effectTitle"]) {
                let effectStart = canvasRoot.timelineBackend && canvasRoot.timelineBackend.clipCount === 0 ? 0 : canvasRoot.secondsFromX(drop.x);
                if (canvasRoot.activeDragSnappedStart >= 0) {
                    effectStart = canvasRoot.activeDragSnappedStart;
                }
                canvasRoot.activeSnapLineSeconds = -1;
                canvasRoot.activeDragSnappedStart = -1;
                canvasRoot.effectDropped(drop.source["effectTitle"], drop.source["effectFilePath"], effectStart, 300);
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
            } else if (canvasRoot.activeDragSnappedStart >= 0) {
                currentStart = canvasRoot.activeDragSnappedStart;
            }

            canvasRoot.activeSnapLineSeconds = -1; // hide snap line
            canvasRoot.activeDragSnappedStart = -1; // reset snapped start
            canvasRoot.activeDragMediaList = []; // hide ghost images

            // Build the queue of drop operations
            let newQueueItems = [];
            for (let i = 0; i < mediaList.length; i++) {
                let m = mediaList[i];
                if (m.mediaFilePath.toLowerCase().endsWith(".srt")) {
                    newQueueItems.push({
                        isSrt: true,
                        filePath: m.mediaFilePath,
                        start: currentStart,
                        track: canvasRoot.trackIndexFromY(drop.y - 100, false)
                    });
                    currentStart += 5;
                } else {
                    let targetTrack = canvasRoot.trackIndexFromY(drop.y - 100, m.mediaHasVideo);
                    if (isTimelineEmpty) {
                        let vCount = canvasRoot.isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1);
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
                required property bool isEffect
                required property int vocalIsolationType
                required property int isolationProgress
                readonly property bool clipSelected: canvasRoot.selectedClipIndices.includes(index)
                readonly property bool isSubtitle: !hasVideo && !hasAudio && !isEffect
                readonly property bool isDragging: canvasRoot.isDraggingClip && clipSelected
                readonly property real effectiveDragSec: isDragging ? canvasRoot.dragDeltaSeconds : 0
                readonly property int effectiveDragTrack: isDragging ? canvasRoot.dragDeltaTrack : 0

                // Layer 2: The Placeholder Background Box (Drop Indicator)
                // This stays behind when the clip is dragged
                Rectangle {
                    x: (delegateRoot.startSeconds + delegateRoot.effectiveDragSec) * canvasRoot.pixelsPerSecond
                    y: {
                        if (delegateRoot.isSubtitle) {
                            return canvasRoot.trackY(200) + (canvasRoot.subtitleTrackHeight - 20) / 2;
                        }
                        if (delegateRoot.isEffect) {
                            let t = delegateRoot.trackIndex + delegateRoot.effectiveDragTrack;
                            if (t < 300) t = 300;
                            return canvasRoot.trackY(t) + (canvasRoot.subtitleTrackHeight - 20) / 2;
                        }
                        let t = delegateRoot.trackIndex + delegateRoot.effectiveDragTrack;
                        let vCount = canvasRoot.isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1);
                        let aCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 1;
                        if (t < 100) {
                            t = Math.max(0, t); // Allow dragging above vCount - 1
                        } else {
                            t = Math.max(100, Math.min(100 + aCount - 1, t));
                        }
                        return canvasRoot.trackY(t) + 4;
                    }
                    width: Math.max((delegateRoot.isSubtitle || delegateRoot.isEffect) ? 24 : 1, delegateRoot.durationSeconds * canvasRoot.pixelsPerSecond)
                    height: (delegateRoot.isSubtitle || delegateRoot.isEffect) ? 20 : canvasRoot.trackHeight - 8
                    radius: 4
                    color: Theme.surfaceRaised
                    border.color: Theme.accent
                    border.width: 1
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
                    isEffect: delegateRoot.isEffect
                    selected: delegateRoot.clipSelected
                    videoTrackCount: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1
                    audioTrackCount: canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 1
                    vocalIsolationType: delegateRoot.vocalIsolationType
                    isolationProgress: delegateRoot.isolationProgress

                    x: delegateRoot.isDragging && canvasRoot.dragController ? canvasRoot.dragController.ghostX : delegateRoot.startSeconds * canvasRoot.pixelsPerSecond
                    y: {
                        if (delegateRoot.isDragging && canvasRoot.dragController) {
                            return canvasRoot.dragController.ghostY;
                        }
                        if (delegateRoot.isSubtitle) {
                            return canvasRoot.trackY(200) + (canvasRoot.subtitleTrackHeight - 20) / 2;
                        }
                        if (delegateRoot.isEffect) {
                            let t = delegateRoot.trackIndex + delegateRoot.effectiveDragTrack;
                            if (t < 300) t = 300;
                            return canvasRoot.trackY(t) + (canvasRoot.subtitleTrackHeight - 20) / 2;
                        }
                        let t = delegateRoot.trackIndex + delegateRoot.effectiveDragTrack;
                        let vCount = canvasRoot.isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1);
                        let aCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 1;
                        if (t < 100) {
                            t = Math.max(0, t); // Allow dragging above vCount - 1
                        } else {
                            t = Math.max(100, Math.min(100 + aCount - 1, t));
                        }
                        return canvasRoot.trackY(t) + 4;
                    }
                    width: Math.max((delegateRoot.isSubtitle || delegateRoot.isEffect) ? 24 : 1, delegateRoot.durationSeconds * canvasRoot.pixelsPerSecond)
                    height: (delegateRoot.isSubtitle || delegateRoot.isEffect) ? 20 : canvasRoot.trackHeight - 8
                    z: delegateRoot.clipSelected ? 4 : 3
                    opacity: delegateRoot.isDragging ? 0.75 : (delegateRoot.isTrackHidden ? 0.35 : 1.0)

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

                    onDragStarted: function (startX, startY) {
                        if (canvasRoot.dragController) {
                            canvasRoot.dragController.beginDrag(canvasRoot.timelineBackend, startX, startY);
                        }
                        let vCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1;
                        canvasRoot.dragMaxVideoTrackCount = vCount;
                        canvasRoot.isDraggingClip = true;
                        canvasRoot.dragDeltaSeconds = 0;
                        canvasRoot.dragDeltaTrack = 0;
                    }
                    onDragUpdated: function (deltaSec, deltaTrack, curX, curY, origStart, origTrack) {
                        let vCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1;
                        let newTrack = origTrack + deltaTrack;
                        if (origTrack < 100 && newTrack >= vCount) {
                            canvasRoot.dragMaxVideoTrackCount = newTrack + 1;
                        } else {
                            canvasRoot.dragMaxVideoTrackCount = vCount;
                        }

                        let targetSec = origStart + deltaSec;
                        let snapped = canvasRoot.getSnapTarget(targetSec);
                        let finalDeltaSec = deltaSec;
                        if (snapped >= 0) {
                            finalDeltaSec = snapped - origStart;
                            canvasRoot.activeSnapLineSeconds = snapped;
                            canvasRoot.activeSnapTrackIndex = newTrack;
                        } else {
                            canvasRoot.activeSnapLineSeconds = -1;
                            canvasRoot.activeSnapTrackIndex = -1;
                        }
                        canvasRoot.dragDeltaSeconds = finalDeltaSec;
                        canvasRoot.dragDeltaTrack = deltaTrack;

                        if (canvasRoot.dragController) {
                            canvasRoot.dragController.updateDrag(canvasRoot.timelineBackend, finalDeltaSec, deltaTrack, curX, curY);
                        }
                    }
                    onDragFinished: function () {
                        if (canvasRoot.dragController) {
                            canvasRoot.dragController.endDrag(canvasRoot.timelineBackend, canvasRoot.dragDeltaSeconds, canvasRoot.dragDeltaTrack, canvasRoot.linkedSelection);
                        }
                        canvasRoot.isDraggingClip = false;
                        canvasRoot.dragDeltaSeconds = 0;
                        canvasRoot.dragDeltaTrack = 0;
                        canvasRoot.activeSnapLineSeconds = -1;
                        canvasRoot.activeSnapTrackIndex = -1;
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
                    onSeekPreviewRequested: function (seconds) {
                        canvasRoot.trimSeekPreview(seconds);
                    }
                }
            }
        }

        Repeater {
            id: ghostDropRepeater
            model: canvasRoot.activeDragMediaList

            Rectangle {
                required property var modelData
                required property int index

                property bool isSubtitle: modelData.mediaFilePath ? modelData.mediaFilePath.toLowerCase().endsWith(".srt") : false
                property bool isEffect: modelData.isEffect !== undefined
                property bool hasVideo: modelData.mediaHasVideo !== undefined ? modelData.mediaHasVideo : false
                property bool hasAudio: modelData.mediaHasAudio !== undefined ? modelData.mediaHasAudio : false

                property real clipDuration: {
                    if (isEffect)
                        return 5.0;
                    if (isSubtitle)
                        return 5.0;
                    let isImage = hasVideo && (!hasAudio || modelData.mediaDuration <= 0.1) && (modelData.mediaFilePath.toLowerCase().endsWith(".jpg") || modelData.mediaFilePath.toLowerCase().endsWith(".png") || modelData.mediaFilePath.toLowerCase().endsWith(".webp") || modelData.mediaFilePath.toLowerCase().endsWith(".jpeg"));
                    return isImage ? 5.0 : (modelData.mediaDuration > 0.01 ? modelData.mediaDuration : 5.0);
                }

                property real queueOffsetSec: {
                    let offset = 0;
                    for (let i = 0; i < index; i++) {
                        let m = canvasRoot.activeDragMediaList[i];
                        let mIsSub = m.mediaFilePath ? m.mediaFilePath.toLowerCase().endsWith(".srt") : false;
                        let mIsEff = m.isEffect !== undefined;
                        let mHasVid = m.mediaHasVideo !== undefined ? m.mediaHasVideo : false;
                        let mHasAud = m.mediaHasAudio !== undefined ? m.mediaHasAudio : false;

                        let mDur = 5.0;
                        if (!mIsEff && !mIsSub) {
                            let mIsImg = mHasVid && (!mHasAud || m.mediaDuration <= 0.1);
                            mDur = mIsImg ? 5.0 : (m.mediaDuration > 0.01 ? m.mediaDuration : 5.0);
                        }
                        offset += mDur;
                    }
                    return offset;
                }

                property real baseStartSec: {
                    let isTimelineEmpty = canvasRoot.timelineBackend && canvasRoot.timelineBackend.clipCount === 0;
                    if (isTimelineEmpty)
                        return 0;
                    if (canvasRoot.activeDragSnappedStart >= 0)
                        return canvasRoot.activeDragSnappedStart;
                    return canvasRoot.secondsFromX(canvasRoot.activeDragX);
                }

                property int targetTrack: {
                    let isTimelineEmpty = canvasRoot.timelineBackend && canvasRoot.timelineBackend.clipCount === 0;
                    let t = canvasRoot.trackIndexFromY(canvasRoot.activeDragY, hasVideo);
                    if (isTimelineEmpty && !isSubtitle && !isEffect) {
                        let vCount = canvasRoot.isDraggingClip ? canvasRoot.dynamicVideoTrackCount : (canvasRoot.timelineBackend ? canvasRoot.timelineBackend.videoTrackCount : 1);
                        t = hasVideo ? vCount - 1 : vCount;
                    } else if (isSubtitle || isEffect) {
                        t = canvasRoot.trackIndexFromY(canvasRoot.activeDragY, true);
                    }
                    return t;
                }

                x: (baseStartSec + queueOffsetSec) * canvasRoot.pixelsPerSecond - canvasRoot.scrollOffset
                y: {
                    if (isSubtitle) {
                        return canvasRoot.trackY(200) + (canvasRoot.subtitleTrackHeight - 20) / 2;
                    }
                    if (isEffect) {
                        return canvasRoot.trackY(300) + (canvasRoot.subtitleTrackHeight - 20) / 2;
                    }
                    let t = targetTrack;
                    let aCount = canvasRoot.timelineBackend ? canvasRoot.timelineBackend.audioTrackCount : 3;
                    if (t >= 100) {
                        t = Math.max(100, Math.min(100 + aCount - 1, t));
                    }
                    return canvasRoot.trackY(t) + 4;
                }

                width: Math.max((isSubtitle || isEffect) ? 24 : 1, clipDuration * canvasRoot.pixelsPerSecond)
                height: (isSubtitle || isEffect) ? 20 : canvasRoot.trackHeight - 8

                radius: 4
                color: isEffect ? "#8a49a7" : (isSubtitle ? "#dc6550" : (hasVideo ? "#0d4a5a" : "#3a2f4e"))
                opacity: 0.75
                border.color: Theme.accent
                border.width: 2
                z: 50

                Row {
                    anchors.left: parent.left
                    anchors.leftMargin: 6
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4
                    visible: isEffect || isSubtitle
                    Image {
                        source: isEffect ? "qrc:/VideoStudioUI/assets/star.svg" : "qrc:/VideoStudioUI/assets/track-subtitle.svg"
                        width: 12
                        height: 12
                        sourceSize: Qt.size(12, 12)
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        text: isEffect ? qsTr("Effect") : qsTr("Subtitle")
                        color: "#ffffff"
                        font.pixelSize: 11
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                        width: Math.max(0, parent.parent.width - 24)
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
        height: 80
        radius: 8
        visible: !canvasRoot.hasClips && !timelineDropArea.containsDrag
        color: timelineDropArea.containsDrag ? "#1a58a8d8" : Theme.surface
        border.color: timelineDropArea.containsDrag ? Theme.accent : Theme.divider
        border.width: timelineDropArea.containsDrag ? 2 : 1
        opacity: timelineDropArea.containsDrag ? 1.0 : 0.75

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: 150
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 150
            }
        }

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
