import re

path = r'c:\we_hunting\video_studio\ui\timeline_view\TimelineTracks.qml'
with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace(
    'readonly property color panelBody: Theme.background\n    readonly property color panelHeader: Theme.surface\n    readonly property color panelLine: Theme.divider\n    readonly property color textPrimary: "#d6e0e4"\n    readonly property color textMuted: "#7f939c"\n    readonly property color accent: "#58a8d8"',
    'property color panelBody: Theme.background\n    property color panelHeader: Theme.surface\n    property color panelLine: Theme.divider\n    property color textPrimary: Theme.text\n    property color textMuted: Theme.textMuted\n    property color accent: Theme.accent'
)

content = content.replace(
    'signal subtitleDropped(string filePath, real startSeconds, int trackIndex)',
    'signal subtitleDropped(string filePath, real startSeconds, int trackIndex)\n    signal generateAudioRequested(string language)'
)

content = content.replace(
    'onZoomValueRequested: function(value) { tracksRoot.setZoom(value, 0) }',
    'onZoomValueRequested: function(value) { tracksRoot.setZoom(value, 0) }\n            onGenerateAudioRequested: function(language) { tracksRoot.generateAudioRequested(language) }'
)

content = content.replace(
    'Rectangle {\n                        Layout.preferredWidth: 15\n                        Layout.fillHeight: true\n                        color: Theme.surfaceInset',
    'Rectangle {\n                        visible: true\n                        Layout.preferredWidth: 17\n                        Layout.fillHeight: true\n                        color: Theme.surfaceInset'
)

if 'MouseArea {' not in content[-500:]:
    content = content.replace(
        'Connections {\n        target: tracksRoot.timelineController',
        'MouseArea {\n        anchors.fill: parent\n        acceptedButtons: Qt.NoButton\n        onWheel: function(wheel) {\n            if (wheel.modifiers & Qt.ControlModifier) {\n                tracksRoot.setZoom(tracksRoot.zoomValue + (wheel.angleDelta.y > 0 ? 0.05 : -0.05), wheel.x - tracksRoot.trackHeaderWidth)\n            } else if (wheel.modifiers & Qt.ShiftModifier) {\n                tracksRoot.setScrollOffset(tracksRoot.scrollOffset - wheel.angleDelta.x - wheel.angleDelta.y)\n            } else {\n                if (Math.abs(wheel.angleDelta.x) > Math.abs(wheel.angleDelta.y)) {\n                    tracksRoot.setScrollOffset(tracksRoot.scrollOffset - wheel.angleDelta.x)\n                } else {\n                    tracksRoot.setVScrollOffset(tracksRoot.vScrollOffset - wheel.angleDelta.y)\n                }\n            }\n            wheel.accepted = true\n        }\n    }\n\n    Connections {\n        target: tracksRoot.timelineController'
    )

with open(path, 'w', encoding='utf-8') as f:
    f.write(content)
print('Done')
