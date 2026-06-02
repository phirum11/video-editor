pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: previewRoot

    readonly property bool isPlaying: playback.playing
    readonly property string clipName: playback.clipName
    readonly property string filePath: playback.filePath
    readonly property real duration: playback.duration
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

    function loadClip(name, path, clipDuration, clipHasVideo) {
        playback.loadClip(name, path, clipDuration, clipHasVideo)
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
                
                blurEffect: effectCtrl ? effectCtrl.blur : null
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
                anchors.leftMargin: 10
                anchors.rightMargin: 14
                spacing: 14

                RowLayout {
                    spacing: 4

                    TransportButton {
                        iconSource: "qrc:/VideoStudioUI/assets/skip-back.svg"
                        toolTipText: "Go to In"
                        enabled: previewRoot.hasClip
                        onClicked: previewRoot.seekTo(0)
                    }
                    TransportButton {
                        iconSource: "qrc:/VideoStudioUI/assets/step-back.svg"
                        toolTipText: "Step Back 1 Frame"
                        enabled: previewRoot.hasClip
                        onClicked: previewRoot.stepFrames(-1)
                    }
                    TransportButton {
                        iconSource: previewRoot.isPlaying ? "qrc:/VideoStudioUI/assets/pause.svg" : "qrc:/VideoStudioUI/assets/play.svg"
                        toolTipText: previewRoot.isPlaying ? "Pause" : "Play"
                        primary: true
                        enabled: previewRoot.hasClip && (previewRoot.hasVideo || previewRoot.hasAudio)
                        onClicked: playback.togglePlayback()
                    }
                    TransportButton {
                        iconSource: "qrc:/VideoStudioUI/assets/step-forward.svg"
                        toolTipText: "Step Forward 1 Frame"
                        enabled: previewRoot.hasClip
                        onClicked: previewRoot.stepFrames(1)
                    }
                    TransportButton {
                        iconSource: "qrc:/VideoStudioUI/assets/skip-forward.svg"
                        toolTipText: "Go to Out"
                        enabled: previewRoot.hasClip
                        onClicked: previewRoot.seekTo(previewRoot.duration)
                    }
                }

                Item {
                    id: scrubBar
                    Layout.fillWidth: true
                    Layout.preferredHeight: 20

                    property real value: previewRoot.duration > 0 ? previewRoot.previewTime / previewRoot.duration : 0.0

                    function seekFromMouse(mouseX) {
                        if (previewRoot.duration <= 0)
                            return

                        previewRoot.seekTo((Math.max(0, Math.min(mouseX, width)) / width) * previewRoot.duration)
                    }

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        height: 4
                        radius: 2
                        color: "#26343b"

                        Rectangle {
                            width: scrubBar.value * parent.width
                            height: parent.height
                            radius: 2
                            color: previewRoot.accent
                        }
                    }

                    Rectangle {
                        x: scrubBar.value * (scrubBar.width - width)
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10
                        height: 10
                        radius: 5
                        color: "#bac7cc"
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: previewRoot.hasClip && previewRoot.duration > 0
                        onClicked: function(mouse) { scrubBar.seekFromMouse(mouse.x) }
                        onPositionChanged: function(mouse) {
                            if (pressed)
                                scrubBar.seekFromMouse(mouse.x)
                        }
                    }
                }

                Text {
                    text: previewRoot.formatTime(previewRoot.previewTime)
                    color: previewRoot.textPrimary
                    font.pixelSize: 13
                    font.family: "Consolas"
                    horizontalAlignment: Text.AlignRight
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
