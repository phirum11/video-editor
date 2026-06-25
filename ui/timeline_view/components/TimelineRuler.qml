pragma ComponentBehavior: Bound

import QtQuick
import VideoStudioUI

Rectangle {
    id: rulerRoot

    property real pixelsPerSecond: 18
    property real scrollOffset: 0
    property real contentWidth: width
    property real playheadSeconds: 0
    property bool hasTimelineClips: true
    property color panelLine: Theme.divider
    property color textMuted: Theme.textMuted
    property color accent: Theme.accent
    readonly property real playheadX: playheadSeconds * pixelsPerSecond - scrollOffset
    readonly property real visibleDurationSeconds: Math.max(1, width / Math.max(0.001, pixelsPerSecond))
    readonly property real majorStepSeconds: chooseMajorStepSeconds(visibleDurationSeconds)
    readonly property int minorDivisions: chooseMinorDivisions(majorStepSeconds)
    readonly property real minorStepSeconds: majorStepSeconds / Math.max(1, minorDivisions)

    signal seekPreview(real seconds)
    signal seekCommitted(real seconds)

    color: Theme.background
    border.color: "transparent"
    border.width: 0
    clip: true

    function chooseMajorStepSeconds(visibleSeconds) {
        const targetLabels = width >= 1200 ? 8 : 6;
        const minimumSeconds = Math.max(1, visibleSeconds / targetLabels);
        const steps = [1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 1200, 1800, 3600, 7200];
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

    function formatTime(seconds) {
        const totalSeconds = Math.max(0, Math.floor(seconds));
        const secs = totalSeconds % 60;
        const mins = Math.floor(totalSeconds / 60) % 60;
        const hours = Math.floor(totalSeconds / 3600);
        if (hours > 0) {
            return String(hours).padStart(2, "0") + ":" + String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0");
        }
        return String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0");
    }

    function secondsFromX(x) {
        return Math.max(0, (x + scrollOffset) / Math.max(0.001, pixelsPerSecond));
    }

    Repeater {
        model: Math.ceil(rulerRoot.contentWidth / Math.max(1, rulerRoot.minorStepSeconds * rulerRoot.pixelsPerSecond)) + 2

        Rectangle {
            required property int index

            readonly property bool major: index % rulerRoot.minorDivisions === 0
            x: index * rulerRoot.minorStepSeconds * rulerRoot.pixelsPerSecond - rulerRoot.scrollOffset
            y: major ? rulerRoot.height - 15 : rulerRoot.height - 7
            width: 1
            height: major ? 15 : 7
            color: major ? "#6b7a80" : Theme.dividerSoft
            opacity: major ? 0.72 : 0.45
            visible: x > -20 && x < rulerRoot.width + 20
        }
    }

    Repeater {
        model: Math.ceil(rulerRoot.contentWidth / Math.max(1, rulerRoot.majorStepSeconds * rulerRoot.pixelsPerSecond)) + 2

        Text {  
            required property int index
            
            readonly property real seconds: index * rulerRoot.majorStepSeconds
            x: seconds * rulerRoot.pixelsPerSecond - rulerRoot.scrollOffset + 18
            y: 2
            text: rulerRoot.formatTime(seconds)
            color: rulerRoot.textMuted
            font.pixelSize: 11
            font.family: "Consolas"
            opacity: 0.82
            visible: x > -120 && x < rulerRoot.width + 20
        }
    }

    Item {
        id: playheadHead
        x: rulerRoot.playheadX - width / 2 + 1
        y: 0
        width: 16
        height: rulerRoot.height
        visible: x > -width && x < rulerRoot.width && rulerRoot.hasTimelineClips
        z: 10

        Canvas {
            x: 0
            y: 0
            width: parent.width
            height: 12
            antialiasing: true
            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                ctx.beginPath();
                ctx.moveTo(0, 0);
                ctx.lineTo(width, 0);
                ctx.lineTo(width / 2, height);
                ctx.closePath();
                ctx.fillStyle = rulerRoot.accent;
                ctx.fill();
            }
        }

        Rectangle {
            x: parent.width / 2 - 1
            y: 10
            width: 2
            height: parent.height - 10
            color: rulerRoot.accent
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        cursorShape: Math.abs(mouseX - rulerRoot.playheadX) < 10 || pressed ? Qt.SizeHorCursor : Qt.ArrowCursor
        onPressed: function (mouse) {
            rulerRoot.seekPreview(rulerRoot.secondsFromX(mouse.x));
        }
        onPositionChanged: function (mouse) {
            if (pressed)
                rulerRoot.seekPreview(rulerRoot.secondsFromX(mouse.x));
        }
        onReleased: function (mouse) {
            rulerRoot.seekCommitted(rulerRoot.secondsFromX(mouse.x));
        }
    }
}
