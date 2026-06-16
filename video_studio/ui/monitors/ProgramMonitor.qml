pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI
import "overlays"

Rectangle {
    id: previewRoot

    readonly property bool isPlaying: playback.playing
    readonly property string clipName: playback.clipName
    readonly property string filePath: playback.filePath
    readonly property real duration: (previewRoot.timelineCtrl && previewRoot.timelineCtrl.timelineEndSeconds > 0) ? previewRoot.timelineCtrl.timelineEndSeconds : playback.duration
    readonly property real previewTime: playback.position
    readonly property bool hasVideo: playback.hasVideo
    readonly property bool hasAudio: playback.hasAudio
    readonly property bool hasClip: playback.filePath.length > 0
    readonly property real audioLevelLeft: playback.audioLevelLeft
    readonly property real audioLevelRight: playback.audioLevelRight

    property var timelineCtrl: null
    property var subtitleCtrl: null
    property var effectCtrl: null
    property alias playbackEngine: playback

    readonly property color panelBody: Theme.background
    readonly property color panelHeader: Theme.surface
    readonly property color panelLine: Theme.divider
    readonly property color textPrimary: "#dce5e8"
    readonly property color textMuted: "#8fa2aa"
    readonly property color accent: "#66aacf"

    color: previewRoot.panelBody
    border.color: previewRoot.panelLine
    border.width: 1
    clip: true

    function loadClip(name, path, clipDuration, clipHasVideo, origPath = "") {
        playback.setSourceInPoint(0)
        playback.loadClip(name, path, clipDuration, clipHasVideo, origPath)
    }

    function loadClipWithOffset(name, path, clipDuration, clipHasVideo, startOffset, sourceInPoint, rowIndex) {
        if (rowIndex !== undefined) {
            currentlyLoadedRow = rowIndex;
        } else {
            // Find row
            currentlyLoadedRow = -1;
            if (timelineCtrl) {
                for (let i = 0; i < timelineCtrl.clipCount; ++i) {
                    if (timelineCtrl.clipContains(i, startOffset + 0.001)) {
                        currentlyLoadedRow = i;
                        break;
                    }
                }
            }
        }
        let origPath = "";
        if (currentlyLoadedRow >= 0 && timelineCtrl) {
            let clipData = timelineCtrl.clipAt(currentlyLoadedRow);
            origPath = clipData ? (clipData.originalFilePath || "") : "";
        }
        
        isSwappingClip = true;
        playback.setSourceInPoint(sourceInPoint !== undefined ? sourceInPoint : 0)
        playback.setClipStartOffset(startOffset)
        playback.loadClip(name, path, clipDuration, clipHasVideo, origPath)
        isSwappingClip = false;
    }

    function clearPreview() {
        playback.clear()
    }

    function pausePreview() {
        playback.pause()
    }

    function seekTo(seconds) {
        playback.seek(seconds)
    }

    property int currentlyLoadedRow: -1
    property bool isSwappingClip: false

    Binding {
        target: playback
        property: "sequenceDuration"
        value: previewRoot.timelineCtrl ? previewRoot.timelineCtrl.timelineEndSeconds : 0.0
    }

    Connections {
        target: playback
        function onPositionChanged() {
            if (previewRoot.isSwappingClip || !previewRoot.timelineCtrl || previewRoot.timelineCtrl.clipCount === 0)
                return;
                
            let foundRow = -1;
            // Only auto-swap if we are outside the current clip's range, or if the current clip doesn't match
            if (previewRoot.currentlyLoadedRow >= 0 && previewRoot.currentlyLoadedRow < previewRoot.timelineCtrl.clipCount) {
                if (previewRoot.timelineCtrl.clipContains(previewRoot.currentlyLoadedRow, playback.position)) {
                    return; // Still in the same clip
                }
            }

            for (let i = 0; i < previewRoot.timelineCtrl.clipCount; ++i) {
                if (previewRoot.timelineCtrl.clipContains(i, playback.position)) {
                    foundRow = i;
                    break;
                }
            }
            
            if (foundRow >= 0 && foundRow !== previewRoot.currentlyLoadedRow) {
                previewRoot.isSwappingClip = true;
                previewRoot.currentlyLoadedRow = foundRow;
                
                let clipData = previewRoot.timelineCtrl.clipAt(foundRow);
                let wasPlaying = playback.playing;
                let currentPos = playback.position;
                
                playback.setSourceInPoint(clipData.sourceInPoint || 0);
                playback.setClipStartOffset(clipData.startSeconds || 0);
                playback.loadClip(clipData.clipName, clipData.filePath, clipData.durationSeconds, clipData.hasVideo, clipData.originalFilePath || "");
                
                playback.seek(currentPos);
                if (wasPlaying) {
                    playback.play();
                }
                
                previewRoot.isSwappingClip = false;
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
            color: "#0b1115"

            PlaybackEngine {
                id: playback
                anchors.fill: parent
                anchors.margins: 8
                visible: previewRoot.hasClip && previewRoot.hasVideo
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
                    color: "#647780"
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
                        color: "#647780"
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
                    enabled: previewRoot.hasClip && (previewRoot.hasVideo || previewRoot.hasAudio)
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

