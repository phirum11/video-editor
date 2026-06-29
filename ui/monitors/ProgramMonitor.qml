// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI
import "overlays"

Rectangle {
    id: previewRoot

    readonly property bool isPlaying: playback.playing
    readonly property string clipName: audioOnlySelectionActive ? audioOnlySelectionName : playback.clipName
    readonly property string filePath: audioOnlySelectionActive ? audioOnlySelectionPath : playback.filePath
    readonly property real duration: (previewRoot.timelineCtrl && previewRoot.timelineCtrl.timelineEndSeconds > 0) ? previewRoot.timelineCtrl.timelineEndSeconds : playback.duration
    readonly property real previewTime: playback.position
    readonly property bool hasVideo: !audioOnlySelectionActive && playback.hasVideo
    readonly property bool hasAudio: audioOnlySelectionActive || playback.hasAudio
    readonly property bool hasClip: audioOnlySelectionActive || playback.filePath.length > 0
    readonly property real audioLevelLeft: playback.audioLevelLeft
    readonly property real audioLevelRight: playback.audioLevelRight

    property var timelineCtrl: null
    property var subtitleCtrl: null
    property var effectCtrl: null
    property alias playbackEngine: playback

    property color panelBody: Theme.background
    property color panelHeader: Theme.surface
    property color panelLine: Theme.divider
    property color textPrimary: Theme.text
    property color textMuted: Theme.textMuted
    property color accent: Theme.accent

    color: previewRoot.panelBody
    border.color: previewRoot.panelLine
    border.width: 1
    clip: true

    property int currentlyLoadedVideoRow: -1
    property var currentlyLoadedAuxRows: [-1, -1, -1, -1, -1, -1, -1, -1]
    property bool isSwappingClip: false
    property bool audioOnlySelectionActive: false
    property string audioOnlySelectionName: ""
    property string audioOnlySelectionPath: ""

    function loadClip(name, path, clipDuration, clipHasVideo, origPath = "", isMuted = false) {
        audioOnlySelectionActive = false
        audioOnlySelectionName = ""
        audioOnlySelectionPath = ""
        playback.setSourceInPoint(0)
        playback.isMuted = isMuted
        playback.loadClip(name, path, clipDuration, clipHasVideo, origPath)
        refreshTimelineEffects(playback.position, currentlyLoadedVideoRow)
    }

    function loadClipWithOffset(name, path, clipDuration, clipHasVideo, startOffset, sourceInPoint, rowIndex, isMuted = false) {
        if (!clipHasVideo) {
            audioOnlySelectionActive = true
            audioOnlySelectionName = name
            audioOnlySelectionPath = path
            currentlyLoadedVideoRow = -1
            playback.pause()
            playback.unloadClip()
            playback.setSourceInPoint(sourceInPoint !== undefined ? sourceInPoint : 0)
            playback.setClipStartOffset(startOffset !== undefined ? startOffset : playback.position)
            if (previewRoot.timelineCtrl) {
                previewRoot.scanClipsAtPosition();
            }
            refreshTimelineEffects(startOffset !== undefined ? startOffset : playback.position, -1)
            return;
        }
        audioOnlySelectionActive = false
        audioOnlySelectionName = ""
        audioOnlySelectionPath = ""
        if (rowIndex !== undefined) {
            currentlyLoadedVideoRow = rowIndex;
        } else {
            currentlyLoadedVideoRow = -1;
            if (timelineCtrl) {
                for (let i = 0; i < timelineCtrl.clipCount; ++i) {
                    if (timelineCtrl.clipContains(i, startOffset + 0.001)) {
                        let cData = timelineCtrl.clipAt(i);
                        if (cData && cData.hasVideo === clipHasVideo && !cData.isEffect && cData.trackIndex < 300) {
                            currentlyLoadedVideoRow = i;
                            break;
                        }
                    }
                }
            }
        }
        let origPath = "";
        if (currentlyLoadedVideoRow >= 0 && timelineCtrl) {
            let clipData = timelineCtrl.clipAt(currentlyLoadedVideoRow);
            origPath = clipData ? (clipData.originalFilePath || "") : "";
            isMuted = clipData ? (clipData.isMuted === true) : false;
        }
        isSwappingClip = true;
        playback.setSourceInPoint(sourceInPoint !== undefined ? sourceInPoint : 0)
        playback.setClipStartOffset(startOffset)
        playback.isMuted = isMuted
        playback.loadClip(name, path, clipDuration, clipHasVideo, origPath)
        refreshTimelineEffects(startOffset !== undefined ? startOffset : playback.position, currentlyLoadedVideoRow)
        isSwappingClip = false;
    }

    function clearPreview() {
        audioOnlySelectionActive = false
        audioOnlySelectionName = ""
        audioOnlySelectionPath = ""
        currentlyLoadedVideoRow = -1
        playback.clear()
        if (auxAudioRepeater) {
            for (let i = 0; i < auxAudioRepeater.count; ++i) {
                let engine = auxAudioRepeater.itemAt(i) as PlaybackEngine;
                if (engine) engine.clear();
            }
        }
    }

    function pausePreview() {
        playback.pause()
        if (auxAudioRepeater) {
            for (let i = 0; i < auxAudioRepeater.count; ++i) {
                let engine = auxAudioRepeater.itemAt(i) as PlaybackEngine;
                if (engine) engine.pause();
            }
        }
    }

    function seekTo(seconds) {
        playback.seek(seconds)
        if (auxAudioRepeater) {
            for (let i = 0; i < auxAudioRepeater.count; ++i) {
                let engine = auxAudioRepeater.itemAt(i) as PlaybackEngine;
                if (engine) engine.seek(seconds);
            }
        }
        refreshTimelineEffects(seconds, currentlyLoadedVideoRow)
    }

    function refreshTimelineEffects(seconds, baseVideoRow) {
        if (previewRoot.effectCtrl) {
            previewRoot.effectCtrl.refreshPlaybackEffects(seconds, baseVideoRow !== undefined ? baseVideoRow : -1)
        }
    }

    Binding {
        target: playback
        property: "sequenceDuration"
        value: previewRoot.timelineCtrl ? previewRoot.timelineCtrl.timelineEndSeconds : 0.0
    }

    // Throttle timer for clip scanning — runs at most every 100ms
    Timer {
        id: clipScanTimer
        interval: 100
        repeat: false
        onTriggered: previewRoot.scanClipsAtPosition()
    }

    function scanClipsAtPosition() {
        if (previewRoot.isSwappingClip || !previewRoot.timelineCtrl)
            return;

        let pos = playback.position;
        let currentVideoRow = -1;
        let currentAudioRows = [];

        for (let i = 0; i < previewRoot.timelineCtrl.clipCount; ++i) {
            if (previewRoot.timelineCtrl.clipContains(i, pos)) {
                let clipData = previewRoot.timelineCtrl.clipAt(i);
                if (clipData.isEffect || clipData.trackIndex >= 300) {
                    continue;
                }
                
                let isVideoTrack = clipData.hasVideo;
                let trackIdx = clipData.trackIndex >= 100 ? clipData.trackIndex - 100 : clipData.trackIndex;
                
                if (isVideoTrack) {
                    if (previewRoot.audioOnlySelectionActive) {
                        continue;
                    }
                    if (previewRoot.timelineCtrl.isTrackHidden(true, trackIdx)) {
                        continue;
                    }
                    if (currentVideoRow === -1) {
                        currentVideoRow = i;
                    } else if (clipData.hasAudio) {
                        if (!previewRoot.timelineCtrl.isTrackMuted(true, trackIdx) && !clipData.isMuted) {
                            currentAudioRows.push(i);
                        }
                    }
                } else if (clipData.hasAudio) {
                    if (previewRoot.timelineCtrl.isTrackMuted(false, trackIdx) || clipData.isMuted) {
                        continue;
                    }
                    // Only include clips that actually have audio — skip SRT/subtitle clips
                    currentAudioRows.push(i);
                }
            }
        }

        // Update main video engine
        if (currentVideoRow >= 0 && (currentVideoRow !== previewRoot.currentlyLoadedVideoRow || !playback.hasVideo)) {
            previewRoot.isSwappingClip = true;
            previewRoot.currentlyLoadedVideoRow = currentVideoRow;

            let clipData = previewRoot.timelineCtrl.clipAt(currentVideoRow);
            let wasPlaying = playback.playing;
            let currentPos = playback.position;

            let trackIdx = clipData.trackIndex >= 100 ? clipData.trackIndex - 100 : clipData.trackIndex;
            playback.setSourceInPoint(clipData.sourceInPoint || 0);
            playback.setClipStartOffset(clipData.startSeconds || 0);
            playback.isMuted = (clipData.isMuted === true || previewRoot.timelineCtrl.isTrackMuted(clipData.hasVideo, trackIdx));
            playback.loadClip(clipData.clipName, clipData.filePath, clipData.durationSeconds, clipData.hasVideo, clipData.originalFilePath || "");

            playback.seek(currentPos);
            if (wasPlaying) playback.play();

            previewRoot.isSwappingClip = false;
        } else if (currentVideoRow === -1 && previewRoot.currentlyLoadedVideoRow !== -1) {
            // All video tracks are hidden — stop showing video but keep position
            previewRoot.currentlyLoadedVideoRow = -1;
            playback.unloadClip();
        }

        // Update auxiliary audio engines
        let newAuxRows = previewRoot.currentlyLoadedAuxRows.slice();
        let wasPlaying = playback.playing;

        for (let j = 0; j < auxAudioRepeater.count; ++j) {
            let row = j < currentAudioRows.length ? currentAudioRows[j] : -1;
            let engine = auxAudioRepeater.itemAt(j) as PlaybackEngine;
            if (!engine) continue;

            if (row >= 0 && row !== newAuxRows[j]) {
                previewRoot.isSwappingClip = true;
                newAuxRows[j] = row;

                let clipData = previewRoot.timelineCtrl.clipAt(row);
                let trackIdx = clipData.trackIndex >= 100 ? clipData.trackIndex - 100 : clipData.trackIndex;

                engine.setSourceInPoint(clipData.sourceInPoint || 0);
                engine.setClipStartOffset(clipData.startSeconds || 0);
                engine.isMuted = (clipData.isMuted === true || previewRoot.timelineCtrl.isTrackMuted(clipData.hasVideo, trackIdx));
                engine.loadClip(clipData.clipName, clipData.filePath, clipData.durationSeconds, false, "");

                engine.seek(playback.position);
                if (wasPlaying) engine.play();

                previewRoot.isSwappingClip = false;
            } else if (row === -1 && newAuxRows[j] !== -1) {
                newAuxRows[j] = -1;
                engine.clear();
            }
        }
        previewRoot.currentlyLoadedAuxRows = newAuxRows;
        refreshTimelineEffects(playback.position, previewRoot.currentlyLoadedVideoRow);
    }

    Connections {
        target: playback
        function onPlayingChanged() {
            for (let i = 0; i < auxAudioRepeater.count; ++i) {
                let engine = auxAudioRepeater.itemAt(i) as PlaybackEngine;
                if (!engine) continue;
                if (playback.playing) {
                    engine.play();
                } else {
                    engine.pause();
                }
            }
        }

        function onPositionChanged() {
            // Throttle: only scan clips periodically, not every frame
            if (!clipScanTimer.running) {
                clipScanTimer.start();
            }
        }
    }

    Connections {
        target: previewRoot.timelineCtrl
        function onTrackStateChanged() {
            previewRoot.scanClipsAtPosition();
        }
        function onTimelineChanged() {
            if (previewRoot.currentlyLoadedVideoRow >= 0 && previewRoot.currentlyLoadedVideoRow < previewRoot.timelineCtrl.clipCount) {
                let clipData = previewRoot.timelineCtrl.clipAt(previewRoot.currentlyLoadedVideoRow);
                if (clipData) {
                    let trackIdx = clipData.trackIndex >= 100 ? clipData.trackIndex - 100 : clipData.trackIndex;
                    let shouldBeMuted = (clipData.isMuted === true || previewRoot.timelineCtrl.isTrackMuted(clipData.hasVideo, trackIdx));
                    if (playback.isMuted !== shouldBeMuted) {
                        playback.isMuted = shouldBeMuted;
                    }
                }
            }
            for (let i = 0; i < auxAudioRepeater.count; ++i) {
                let engine = auxAudioRepeater.itemAt(i) as PlaybackEngine;
                let row = previewRoot.currentlyLoadedAuxRows[i];
                if (engine && row >= 0 && row < previewRoot.timelineCtrl.clipCount) {
                    let clipData = previewRoot.timelineCtrl.clipAt(row);
                    if (clipData) {
                        let trackIdx = clipData.trackIndex >= 100 ? clipData.trackIndex - 100 : clipData.trackIndex;
                        let shouldBeMuted = (clipData.isMuted === true || previewRoot.timelineCtrl.isTrackMuted(clipData.hasVideo, trackIdx));
                        if (engine.isMuted !== shouldBeMuted) {
                            engine.isMuted = shouldBeMuted;
                        }
                    }
                }
            }
        }
    }

    function stepFrames(frames) {
        playback.stepFrame(frames)
    }

    function formatTime(seconds) {
        const fps = playback.frameRate > 0 ? playback.frameRate : 25 
        const totalFrames = Math.max(0, Math.floor(seconds * fps))   
        const frames = totalFrames % Math.round(fps)   
        const totalSeconds = Math.floor(totalFrames / fps)  
        const secs = totalSeconds % 60   
        const mins = Math.floor(totalSeconds / 60) % 60   
        const hours = Math.floor(totalSeconds / 3600) 
        return String(hours).padStart(2, "0") + ":"  
            + String(mins).padStart(2, "0") + ":"
            + String(secs).padStart(2, "0") + ":"
            + String(frames).padStart(2, "0")
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: previewRoot.panelHeader

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 10
                spacing: 8

                Image {
                    Layout.preferredWidth: 17
                    Layout.preferredHeight: 17
                    source: "qrc:/VideoStudioUI/assets/monitor-play.svg"
                    opacity: 0.78
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Video Preview")
                    color: previewRoot.textPrimary
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }

                Text {
                    text: previewRoot.hasClip ? previewRoot.clipName : qsTr("No clip selected")
                    color: previewRoot.textMuted
                    font.pixelSize: 12
                    visible: previewRoot.width >= 560
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: previewRoot.panelBody

            PlaybackEngine {
                id: playback
                backgroundColor: previewRoot.panelBody
                anchors.fill: parent
                anchors.margins: 0
                visible: previewRoot.hasClip && previewRoot.hasVideo
            }

            Repeater {
                id: auxAudioRepeater
                model: 8
                PlaybackEngine {
                    visible: false
                    sequenceDuration: playback.sequenceDuration
                }
            }

            LiveSubtitleOverlay {
                id: subtitleOverlay
                anchors.fill: playback
                visible: previewRoot.hasClip && previewRoot.hasVideo
                timelineCtrl: previewRoot.timelineCtrl
                subtitleCtrl: previewRoot.subtitleCtrl
                currentTime: previewRoot.previewTime
            }
            
            RegionSelectorOverlay {
                anchors.fill: playback
                
                property real sWidth: playback.sourceVideoWidth
                property real sHeight: playback.sourceVideoHeight
                
                // Calculate actual video rect because playback engine letterboxes it
                function getVideoRect() {
                    if (sWidth <= 0 || sHeight <= 0) {
                         return Qt.rect(0, 0, playback.width, playback.height)
                    }
                    let scale = Math.min(playback.width / sWidth, playback.height / sHeight)
                    let w = sWidth * scale
                    let h = sHeight * scale
                    let x = (playback.width - w) / 2
                    let y = (playback.height - h) / 2
                    return Qt.rect(x, y, w, h)
                }

                property rect vRect: Qt.rect(0, 0, 0, 0)
                
                onWidthChanged: vRect = getVideoRect()
                onHeightChanged: vRect = getVideoRect()
                onSWidthChanged: vRect = getVideoRect()
                onSHeightChanged: vRect = getVideoRect()

                videoX: vRect.x
                videoY: vRect.y
                videoWidth: vRect.width
                videoHeight: vRect.height
                
                blurEffect: previewRoot.effectCtrl ? previewRoot.effectCtrl.blur : null
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 260)
                spacing: 10
                visible: !previewRoot.hasClip || !previewRoot.hasVideo

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 42
                    Layout.preferredHeight: 42
                    source: "qrc:/VideoStudioUI/assets/monitor.svg"
                    opacity: 0.24
                }

                Text {
                    Layout.fillWidth: true
                    text: previewRoot.hasClip && !previewRoot.hasVideo
                        ? qsTr("Audio clip selected")
                        : qsTr("Preview will appear here")
                    color: Theme.textMuted
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            color: Theme.background
            border.color: Theme.divider
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 0

                // LEFT: Timecode
                Row {
                    spacing: 6
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Text {
                        text: previewRoot.formatTime(previewRoot.previewTime)
                        color: "#00e5e5"
                        font.pixelSize: 13
                        font.family: "Consolas"
                    }
                    Text {
                        text: "/"
                        color: Theme.textMuted
                        font.pixelSize: 13
                        font.family: "Consolas"
                    }
                    Text {
                        text: previewRoot.formatTime(previewRoot.duration)
                        color: previewRoot.textPrimary
                        font.pixelSize: 13
                        font.family: "Consolas"
                    }
                }

                Item { Layout.fillWidth: true } // Left spacer

                // CENTER: Play/Pause button
                TransportButton {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    iconSource: previewRoot.isPlaying ? "qrc:/VideoStudioUI/assets/pause.svg" : "qrc:/VideoStudioUI/assets/play.svg"
                    toolTipText: previewRoot.isPlaying ? "Pause" : "Play"
                    primary: true
                    enabled: (previewRoot.hasClip && (previewRoot.hasVideo || previewRoot.hasAudio)) || (previewRoot.timelineCtrl && previewRoot.timelineCtrl.clipCount > 0)
                    onClicked: playback.togglePlayback()
                }

                Item { Layout.fillWidth: true } // Right spacer

                // RIGHT: View Options
                RowLayout {
                    spacing: 12
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                    // "Full" button mock
                    Rectangle {
                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 32
                        color: "transparent"
                        border.color: previewRoot.textPrimary
                        border.width: 1
                        radius: 2
                        opacity: 0.8
                        Text {
                            anchors.centerIn: parent
                            text: "Full"
                            color: previewRoot.textPrimary
                            font.pixelSize: 11
                        }
                    }

                    // Ratio button mock
                    Rectangle {
                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 38
                        color: "transparent"
                        border.color: previewRoot.textPrimary
                        border.width: 1
                        radius: 2
                        opacity: 0.8
                        Text {
                            anchors.centerIn: parent
                            text: "Ratio"
                            color: previewRoot.textPrimary
                            font.pixelSize: 11
                        }
                    }
                    
                    // Fullscreen mock icon
                    Image {
                        source: "qrc:/VideoStudioUI/assets/grid-view.svg" // Closest available icon
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        opacity: 0.8
                    }
                }
            }
        }
    }

    component TransportButton: AbstractButton {
        id: transportButton
        property url iconSource: ""
        property bool primary: false
        property string toolTipText: ""

        implicitWidth: transportButton.primary ? 26 : 22
        implicitHeight: transportButton.primary ? 26 : 22
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        ToolTip.visible: hovered && toolTipText !== ""
        ToolTip.text: toolTipText

        background: Item {}

        contentItem: Image {
            anchors.centerIn: parent
            width: transportButton.primary ? 14 : 11
            height: transportButton.primary ? 14 : 11
            source: transportButton.iconSource
            opacity: !transportButton.enabled ? 0.28 : (transportButton.hovered ? 1 : 0.65)
        }
    }
}
