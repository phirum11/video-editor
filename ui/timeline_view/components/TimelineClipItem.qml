pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import VideoStudioUI
import VideoStudio.Media 1.0

Rectangle {
    id: clipRoot
    required property int clipIndex
    required property string clipName
    required property string filePath
    property string originalFilePath: ""
    required property real startSeconds
    required property real durationSeconds
    property real sourceInPoint: 0
    property real sourceDuration: durationSeconds
    required property int trackIndex
    required property bool hasVideo
    required property bool hasAudio
    required property bool isEffect
    required property bool selected
    property int vocalIsolationType: 0
    property int isolationProgress: -1

    property int videoTrackCount: 1
    property int audioTrackCount: 1

    property real pixelsPerSecond: 18
    property int trackHeight: 34
    property int separatorHeight: 4
    property int minClipWidth: 72
    property string activeTool: "selection"
    property bool linkedSelection: true
    property bool snapEnabled: true
    property bool isTrackLocked: false
    property bool isTrackHidden: false

    signal selectedRequested(int index, bool toggle)
    signal previewRequested(string name, string path, real duration, bool video)
    signal deleteRequested(int index, string path)
    signal splitRequested(int index, real seconds, bool linked)
    signal moveRequested(int index, real startSeconds, int trackIndex, bool linked)
    signal trimLeftRequested(int index, real deltaSeconds, bool linked)
    signal trimRightRequested(int index, real deltaSeconds, bool linked)
    signal seekPreviewRequested(real seconds)

    signal dragStarted(real startX, real startY)
    signal dragUpdated(real deltaSeconds, int deltaTrack, real curX, real curY, real origStart, int origTrack)
    signal dragFinished()

    property real dragOffsetSeconds: 0
    property int dragOffsetTrack: 0
    property real trimOffsetLeft: 0
    property real trimOffsetRight: 0

    readonly property bool isSubtitle: !hasVideo && !hasAudio && !isEffect

    readonly property real unclampedPixelStartX: (startSeconds + dragOffsetSeconds + trimOffsetLeft) * pixelsPerSecond
    readonly property real pixelStartX: Math.max(0, unclampedPixelStartX)
    readonly property real pixelEndX: (startSeconds + dragOffsetSeconds + durationSeconds + trimOffsetRight) * pixelsPerSecond + (pixelStartX - unclampedPixelStartX)

    x: pixelStartX
    y: {
        if (isSubtitle) {
            return (trackIndex >= 200 ? (trackIndex - 200) : 0) * 32 + (32 - 20) / 2;
        }
        if (isEffect) {
            let effIdx = trackIndex >= 300 ? trackIndex - 300 : 0;
            let subOffset = (typeof backend !== "undefined" && backend && backend.hasSubtitleTrack) ? 1 : 0;
            return subOffset * 32 + effIdx * 32 + (32 - 20) / 2;
        }
        let t = trackIndex + dragOffsetTrack;
        let vCount = clipRoot.videoTrackCount;
        let subOffset = (typeof backend !== "undefined" && backend && backend.hasSubtitleTrack) ? 1 : 0;
        let effCount = (typeof backend !== "undefined" && backend && backend.effectTrackCount) ? backend.effectTrackCount : 0;
        let topOffset = subOffset * 32 + effCount * 32;
        t = Math.max(0, t);
        return (t < 100 ? topOffset + (vCount - 1 - t) * trackHeight : topOffset + vCount * trackHeight + separatorHeight + (t - 100) * trackHeight) + 4;
    }
    width: Math.max((isSubtitle || isEffect) ? 24 : 1, pixelEndX - pixelStartX)
    height: (isSubtitle || isEffect) ? 20 : trackHeight - 8
    radius: 2
    color: isEffect ? "#7a4997" : (isSubtitle ? "#cc5540" : (hasVideo ? "#0c3a4a" : "#2a1f3e"))
    clip: false
    z: selected ? 4 : 3
    opacity: clipRoot.isTrackHidden ? 0.35 : 1.0

    HoverHandler {
        cursorShape: clipRoot.isTrackLocked ? Qt.ForbiddenCursor : Qt.PointingHandCursor
        enabled: !clipRoot.isTrackLocked
    }

    border.color: selected ? "white" : "transparent"
    border.width: selected ? 1.5 : 0

    readonly property int waveformBarCount: hasAudio && width > 28 ? Math.max(20, Math.min(400, Math.floor(width / 1.5))) : 0
    readonly property int audioStripHeight: hasVideo ? Math.min(20, Math.floor(height * 0.45)) : (height - 18)

    function snapTime(seconds) {
        return Math.max(0, seconds);
    }

    function isEditTool() {
        return activeTool === "selection" || activeTool === "ripple" || activeTool === "slip";
    }

    // We use a fixed number of thumbnails (around 40) per clip to prevent
    // re-rendering and re-fetching from FFmpeg when the user zooms in or out.
    // The frames will just resize and tile instantly without lag.
    // We base interval on total source media duration, so density is consistent across all clips from same source
    readonly property real thumbInterval: Math.max(0.5, clipRoot.sourceDuration / 40)
    readonly property int filmFrameCount: hasVideo && sourceDuration > 0 ? Math.ceil(clipRoot.sourceDuration / thumbInterval) : 0

    // FILMSTRIP — fills the clip behind everything
    Item {
        id: filmstripContainer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: clipRoot.hasAudio && clipRoot.hasVideo ? waveform.top : parent.bottom
        visible: clipRoot.hasVideo
        clip: true

        readonly property bool isImageClip: clipRoot.filePath.match(/\.(jpg|jpeg|png|webp|bmp)$/i) !== null

        // Static Images - ONE tiled image that repeats perfectly at its native aspect ratio
        Image {
            x: -(clipRoot.sourceInPoint + clipRoot.trimOffsetLeft) * clipRoot.pixelsPerSecond
            width: clipRoot.sourceDuration * clipRoot.pixelsPerSecond
            height: parent.height
            visible: parent.isImageClip

            sourceSize.height: Math.max(1, parent.height)

            source: (parent.isImageClip && parent.height > 0) ? "image://media/" + encodeURIComponent((clipRoot.originalFilePath !== "" ? clipRoot.originalFilePath : clipRoot.filePath) + "|0.0") : ""
            asynchronous: true
            fillMode: Image.Tile
        }

        // Video clips - dynamically instantiates only visible thumbnails using ListView
        ListView {
            id: filmstripList
            x: 0
            width: parent.width
            height: parent.height
            visible: !parent.isImageClip
            spacing: 0
            orientation: ListView.Horizontal
            interactive: false

            // This is the magical part: we offset the content by the source media offset + drag offset!
            contentX: (clipRoot.sourceInPoint + clipRoot.trimOffsetLeft) * clipRoot.pixelsPerSecond

            model: parent.visible && clipRoot.sourceDuration > 0 ? Math.ceil(clipRoot.sourceDuration / clipRoot.thumbInterval) : 0

            delegate: Image {
                required property int index

                readonly property real startTime: index * clipRoot.thumbInterval
                readonly property real endTime: Math.min((index + 1) * clipRoot.thumbInterval, clipRoot.sourceDuration)

                width: Math.max(1, (endTime - startTime) * clipRoot.pixelsPerSecond)
                height: filmstripList.height

                sourceSize.height: Math.max(1, filmstripList.height)

                source: (!filmstripContainer.isImageClip && filmstripList.height > 0) ? "image://media/" + encodeURIComponent((clipRoot.originalFilePath !== "" ? clipRoot.originalFilePath : clipRoot.filePath) + "|" + (startTime + 0.1)) : ""
                asynchronous: true
                fillMode: Image.Tile
                verticalAlignment: Image.AlignTop
                horizontalAlignment: Image.AlignLeft
                clip: true
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
        clip: true
        z: 5

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: clipRoot.hasVideo ? "#cc0e3d4d" : "#cc1e1435"
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }

        Rectangle {
            id: subtitleIcon
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 16
            height: 16
            radius: 2
            color: "#e86a53"
            visible: clipRoot.isSubtitle

            Text {
                anchors.centerIn: parent
                text: "A"
                color: "white"
                font.pixelSize: 10
                font.weight: Font.Bold
            }
        }

        Rectangle {
            id: effectIcon
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 16
            height: 16
            radius: 2
            color: "#9b59b6"
            visible: clipRoot.isEffect

            Text {
                anchors.centerIn: parent
                text: "★"
                color: "white"
                font.pixelSize: 11
                font.weight: Font.Bold
            }
        }

        Text {
            anchors.left: clipRoot.isSubtitle ? subtitleIcon.right : (clipRoot.isEffect ? effectIcon.right : parent.left)
            anchors.leftMargin: 8
            anchors.right: durationText.visible ? durationText.left : parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            text: clipRoot.clipName
            color: Theme.text
            font.pixelSize: 11
            font.weight: Font.Medium
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            visible: clipRoot.width > ((clipRoot.isSubtitle || clipRoot.isEffect) ? 32 : 16)
        }

        Text {
            id: durationText
            anchors.right: parent.right
            anchors.rightMargin: 8
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

        HardwareWaveformItem {
            id: waveformImage
            x: -(clipRoot.sourceInPoint + clipRoot.trimOffsetLeft) * clipRoot.pixelsPerSecond + 2
            y: 2
            width: clipRoot.sourceDuration * clipRoot.pixelsPerSecond - 4
            height: parent.height - 4
            filePath: clipRoot.hasAudio && clipRoot.sourceDuration > 0 ? clipRoot.filePath : ""
            sourceDuration: clipRoot.sourceDuration
            pixelsPerSecond: clipRoot.pixelsPerSecond
            visible: clipRoot.hasAudio
        }
    }

    // MUTED OVERLAY — dims clip and shows muted indicator
    Rectangle {
        anchors.fill: waveform
        // qmllint disable unqualified
        visible: clipRoot.hasAudio && (typeof isMuted !== "undefined" ? isMuted : false)
        // qmllint enable unqualified
        color: "#cc000000"
        z: 5
// qmllint disable
        Text {
            anchors.centerIn: parent
            text: "🔇"
            font.pixelSize: 14
            visible: parent.height >= 16
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
        visible: false
        color: deleteClipMouse.containsMouse ? "#e04050" : "#00000066"
        z: 10
// qmllint disable
        Text {
            anchors.centerIn: parent
            text: "\u00d7"
            color: Theme.text
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        MouseArea {
            id: deleteClipMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: clipRoot.deleteRequested(clipRoot.clipIndex, clipRoot.filePath)
        }
    }

    // SELECTION BORDER — subtle glow when selected
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: clipRoot.selected ? "#5ec4e8" : (clipRoot.isEffect ? "#b97be0" : (clipRoot.hasVideo ? "#2a8a9a" : "#6a5a8a"))
        border.width: clipRoot.selected ? 1.5 : 1
        radius: clipRoot.radius
        z: 8
        opacity: clipRoot.selected ? 1.0 : 0.8
    }

    MouseArea {
        id: clipMouse
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        enabled: !clipRoot.isTrackLocked
        z: 6

        property real pressX: 0
        property real pressY: 0
        property real originalStart: 0
        property int originalTrack: 0
        property real originalX: 0
        property real originalY: 0
        property bool draggingClip: false

        cursorShape: Qt.PointingHandCursor

        onPressed: function (mouse) {
            if (mouse.button === Qt.RightButton) {
                if (!clipRoot.selected) {
                    clipRoot.selectedRequested(clipRoot.clipIndex, false);
                }
            } else {
                const toggle = (mouse.modifiers & Qt.ControlModifier) || (mouse.modifiers & Qt.ShiftModifier);
                clipRoot.selectedRequested(clipRoot.clipIndex, toggle !== 0);
            }
            if (clipRoot.hasVideo || clipRoot.hasAudio) {
                clipRoot.previewRequested(clipRoot.clipName, clipRoot.filePath, clipRoot.durationSeconds, clipRoot.hasVideo);
            }
        }

        onClicked: function (mouse) {
            if (mouse.button === Qt.RightButton) {
                clipContextMenu.popup();
                return;
            }
            if (clipRoot.activeTool === "razor") {
                const splitAt = clipRoot.startSeconds + mouse.x / Math.max(1, clipRoot.pixelsPerSecond);
                clipRoot.splitRequested(clipRoot.clipIndex, splitAt, clipRoot.linkedSelection);
            }
        }
    }

    DragHandler {
        id: clipDrag
        target: null
        dragThreshold: 3
        cursorShape: Qt.PointingHandCursor
        enabled: !clipRoot.isTrackLocked && !(leftTrimMouse.containsMouse || leftTrimMouse.pressed || rightTrimMouse.containsMouse || rightTrimMouse.pressed)
        // qmllint disable missing-property
        onActiveChanged: {
            if (active) {
                clipMouse.originalStart = clipRoot.startSeconds;
                clipMouse.originalTrack = clipRoot.trackIndex;
                clipMouse.originalX = clipRoot.x;
                clipMouse.originalY = clipRoot.y;
                clipRoot.dragStarted(clipMouse.originalX, clipMouse.originalY);
            } else {
                clipRoot.dragFinished();
            }
        }
        onTranslationChanged: {
            if (!active || !clipRoot.isEditTool())
                return;
            const deltaX = translation.x;
            const deltaY = translation.y;
            const deltaSec = deltaX / Math.max(1, clipRoot.pixelsPerSecond);
            let deltaTrack = 0;
            let vCount = clipRoot.videoTrackCount;
            let aCount = clipRoot.audioTrackCount;
            
            if (clipMouse.originalTrack >= 300) {
                // Effect tracks: each track is 32px height
                deltaTrack = Math.round(deltaY / 32);
                let maxDeltaUpEff = -(clipMouse.originalTrack - 300); // Cannot drop below track 300
                let effCount = (typeof backend !== "undefined" && backend && backend.effectTrackCount) ? backend.effectTrackCount : 1;
                let maxDeltaDownEff = effCount - (clipMouse.originalTrack - 300);
                deltaTrack = Math.max(maxDeltaUpEff, Math.min(maxDeltaDownEff, deltaTrack));
            } else if (clipMouse.originalTrack >= 200) {
                // Subtitle tracks
                deltaTrack = 0;
            } else if (clipMouse.originalTrack < 100) {
                // Video tracks: Dragging UP (negative deltaY) increases track index
                deltaTrack = Math.round(-deltaY / clipRoot.trackHeight);
                let maxDeltaDown = -clipMouse.originalTrack; // Cannot drop below track 0
                let maxDeltaUp = vCount - clipMouse.originalTrack; // Cap to 1 new track above current max
                deltaTrack = Math.max(maxDeltaDown, Math.min(maxDeltaUp, deltaTrack));
            } else {
                // Audio tracks: Dragging DOWN (positive deltaY) increases audio track index
                deltaTrack = Math.round(deltaY / clipRoot.trackHeight);
                let maxDeltaUpAudio = -(clipMouse.originalTrack - 100);
                let maxDeltaDownAudio = aCount - (clipMouse.originalTrack - 100);
                deltaTrack = Math.max(maxDeltaUpAudio, Math.min(maxDeltaDownAudio, deltaTrack));
            }
            let curX = clipMouse.originalX + deltaX;
            let curY = clipMouse.originalY + deltaY;
            clipRoot.dragUpdated(deltaSec, deltaTrack, curX, curY, clipMouse.originalStart, clipMouse.originalTrack);
        }
    }
    Item {
        id: leftTrimHandle
        anchors.right: parent.left
        anchors.rightMargin: -4
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 18
        visible: clipRoot.selected && !clipRoot.isTrackLocked
        z: 15

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 4
            color: "white"
            radius: 2
        }

        Rectangle {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 14
            height: Math.min(28, clipRoot.height)
            color: "#111111"
            border.color: "white"
            border.width: 1
            radius: 4

            Text {
                anchors.centerIn: parent
                text: "\u276e"
                color: "white"
                font.pixelSize: 10
                font.weight: Font.Bold
            }
        }

        MouseArea {
            id: leftTrimMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            preventStealing: true

            property real startSceneX: 0

            onPressed: function (mouse) {
                startSceneX = mouse.x + clipRoot.x;
            }

            onPositionChanged: function (mouse) {
                if (pressed) {
                    let sceneX = mouse.x + clipRoot.x;
                    let deltaX = sceneX - startSceneX;

                    const minSec = Math.max(0.1, 12 / Math.max(1, clipRoot.pixelsPerSecond));
                    const maxPositiveDelta = Math.max(0, clipRoot.durationSeconds - minSec);
                    const maxNegativeDelta = (clipRoot.isEffect || clipRoot.isSubtitle) ? -clipRoot.startSeconds : -clipRoot.sourceInPoint;

                    let validDeltaSec = deltaX / Math.max(1, clipRoot.pixelsPerSecond);
                    if (validDeltaSec > maxPositiveDelta) {
                        validDeltaSec = maxPositiveDelta;
                    } else if (validDeltaSec < maxNegativeDelta) {
                        validDeltaSec = maxNegativeDelta;
                    }

                    clipRoot.trimOffsetLeft = validDeltaSec;
                    clipRoot.seekPreviewRequested(Math.max(0, clipRoot.startSeconds + validDeltaSec));
                }
            }

            onReleased: function (mouse) {
                const deltaSeconds = clipRoot.trimOffsetLeft;
                if (deltaSeconds !== 0) {
                    clipRoot.trimLeftRequested(clipRoot.clipIndex, deltaSeconds, clipRoot.linkedSelection);
                }
                clipRoot.trimOffsetLeft = 0;
            }
        }
    }

    Item {
        id: rightTrimHandle
        anchors.left: parent.right
        anchors.leftMargin: -4
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 18
        visible: clipRoot.selected && !clipRoot.isTrackLocked
        z: 15

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 4
            color: "white"
            radius: 2
        }

        Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 14
            height: Math.min(28, clipRoot.height)
            color: "#111111"
            border.color: "white"
            border.width: 1
            radius: 4

            Text {
                anchors.centerIn: parent
                text: "\u276f"
                color: "white"
                font.pixelSize: 10
                font.weight: Font.Bold
            }
        }

        MouseArea {
            id: rightTrimMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            preventStealing: true

            property real startSceneX: 0

            onPressed: function (mouse) {
                startSceneX = mouse.x + clipRoot.x + clipRoot.width;
            }

            onPositionChanged: function (mouse) {
                if (pressed) {
                    let sceneX = mouse.x + clipRoot.x + clipRoot.width;
                    let deltaX = sceneX - startSceneX;

                    const minSec = Math.max(0.1, 12 / Math.max(1, clipRoot.pixelsPerSecond));
                    const maxNegativeDelta = -Math.max(0, clipRoot.durationSeconds - minSec);
                    
                    let maxPositiveDelta = Infinity;
                    if (clipRoot.sourceDuration > 0 && !clipRoot.isEffect && !clipRoot.isSubtitle) {
                        maxPositiveDelta = clipRoot.sourceDuration - (clipRoot.sourceInPoint + clipRoot.durationSeconds);
                    }

                    let validDeltaSec = deltaX / Math.max(1, clipRoot.pixelsPerSecond);
                    if (validDeltaSec < maxNegativeDelta) {
                        validDeltaSec = maxNegativeDelta;
                    } else if (validDeltaSec > maxPositiveDelta) {
                        validDeltaSec = maxPositiveDelta;
                    }

                    clipRoot.trimOffsetRight = validDeltaSec;
                    clipRoot.seekPreviewRequested(Math.max(0, clipRoot.startSeconds + clipRoot.durationSeconds + validDeltaSec));
                }
            }

            onReleased: function (mouse) {
                const deltaSeconds = clipRoot.trimOffsetRight;
                if (deltaSeconds !== 0) {
                    clipRoot.trimRightRequested(clipRoot.clipIndex, deltaSeconds, clipRoot.linkedSelection);
                }
                clipRoot.trimOffsetRight = 0;
            }
        }
    }

    function formatDuration(seconds) {
        const safeSeconds = Number.isFinite(seconds) && seconds > 0 ? Math.floor(seconds) : 0;
        const secs = safeSeconds % 60;
        const mins = Math.floor(safeSeconds / 60) % 60;
        const hours = Math.floor(safeSeconds / 3600);
        return hours > 0 ? String(hours).padStart(2, "0") + ":" + String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0") : String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0");
    }

    TimelineClipContextMenu {
        id: clipContextMenu
        clipIndex: clipRoot.clipIndex
        vocalIsolationType: clipRoot.vocalIsolationType
        clipHasAudio: clipRoot.hasAudio
        // qmllint disable unqualified
        clipIsMuted: typeof isMuted !== "undefined" ? isMuted : false
        // Resolve timelineController from parent scope (TimelineTracks.qml has backend)
        timelineController: typeof backend !== "undefined" ? backend : null
        // qmllint enable unqualified
    }
 // qmllint disable
    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        height: 6
        width: parent.width * (clipRoot.isolationProgress / 100.0)
        color: Theme.accent
        visible: clipRoot.isolationProgress >= 0
        z: 10

        Behavior on width {
            NumberAnimation {
                duration: 150
            }
        }
    }
// qmllint disable
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

    // LOCKED TRACK OVERLAY — shows lock icon and blocks all interaction
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: clipRoot.isTrackLocked
        z: 20
        radius: clipRoot.radius

        Image {
            anchors.centerIn: parent
            source: "qrc:/VideoStudioUI/assets/lock.svg"
            sourceSize: Qt.size(16, 16)
            opacity: 0.6
            visible: parent.width > 30
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            cursorShape: Qt.ForbiddenCursor
        }
    }

    // HIDDEN TRACK INDICATOR — shows eye-off icon
    Image {
        anchors.centerIn: parent
        source: "qrc:/VideoStudioUI/assets/eye-off.svg"
        sourceSize: Qt.size(14, 14)
        opacity: 0.5
        visible: clipRoot.isTrackHidden && parent.width > 30
        z: 15
    }
}
