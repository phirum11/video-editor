import QtQuick
import QtQuick.Layouts
import VideoStudioUI
import "components"

Rectangle {
    id: timelineRoot
    color: Theme.background
    property real playbackPosition: 0
    property real sequenceDuration: 0
    property real audioLevelLeft: 0
    property real audioLevelRight: 0
    property string activeTool: "selection"
    property bool snapEnabled: true
    property bool linkedSelection: true
    property alias timelineController: tracks.timelineController

    signal previewRequested(string name, string filePath, real duration, bool hasVideo, real startOffset, real sourceInPoint)
    signal previewCleared()
    signal seekRequested(real seconds)
    signal subtitleDropped(string filePath, real startSeconds, int trackIndex)
    signal generateAudioRequested(string language)

    function addMediaClip(name, filePath, duration, hasVideo, hasAudio, startSeconds, trackIndex) {
        const insertAt = Number.isFinite(startSeconds) && startSeconds >= 0
            ? startSeconds
            : Math.max(0, playbackPosition)
        const insertTrack = Number.isFinite(trackIndex) ? trackIndex : -1
        return tracks.addMediaClip(name, filePath, duration, hasVideo, hasAudio, insertAt, insertTrack)
    }

    function deleteSelectedClips() {
        tracks.deleteSelectedClips()
    }

    function removeMediaClips(filePath) {
        return tracks.removeMediaClips(filePath)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        TimelineTracks {
            id: tracks
            Layout.fillWidth: true
            Layout.fillHeight: true
            activeTool: timelineRoot.activeTool
            snapEnabled: timelineRoot.snapEnabled
            linkedSelection: timelineRoot.linkedSelection
            playbackPosition: timelineRoot.playbackPosition
            sequenceDuration: timelineRoot.sequenceDuration
            onToolSelected: function(tool) { timelineRoot.activeTool = tool }
            onSnapToggleRequested: function(enabled) { timelineRoot.snapEnabled = enabled }
            onLinkedSelectionToggleRequested: function(enabled) { timelineRoot.linkedSelection = enabled }
            onPreviewRequested: function(name, filePath, duration, hasVideo, startOffset, sourceInPoint) {
                timelineRoot.previewRequested(name, filePath, duration, hasVideo, startOffset, sourceInPoint)
            }
            onPreviewCleared: timelineRoot.previewCleared()
            onSeekRequested: function(seconds) {
                timelineRoot.seekRequested(seconds)
            }
            onSubtitleDropped: function(filePath, startSeconds, trackIndex) {
                timelineRoot.subtitleDropped(filePath, startSeconds, trackIndex)
            }
            onGenerateAudioRequested: function(language) {
                timelineRoot.generateAudioRequested(language)
            }
        }

        AudioMeter {
            Layout.fillHeight: true
            leftLevel: timelineRoot.audioLevelLeft
            rightLevel: timelineRoot.audioLevelRight
        }
    }
}

