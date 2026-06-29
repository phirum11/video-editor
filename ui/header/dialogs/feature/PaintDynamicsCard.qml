pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI
import VideoStudio.Core

Rectangle {
    id: root
    Layout.fillWidth: true
    implicitHeight: mainLayout.implicitHeight + 28
    color: "#202026"
    radius: 12
    border.color: "#303038"
    border.width: 1

    property string activeBrush: "Paintbrush"
    property var brushList: ["Paintbrush", "Airbrush", "Pencil", "Eraser", "Ink", "Clone"]
    property string activeBlending: "Smudge"
    property var blendingList: ["Smudge", "DodgeBurn", "Heal", "Convolve", "SourceCore"]

    Column {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 16

        RowLayout {
            width: parent.width
            spacing: 8
            Image {
                source: "qrc:/VideoStudioUI/assets/tool-zoom.svg"
                sourceSize: Qt.size(16, 16)
            }
            Text {
                text: "Paint Dynamics & Brush Engine"
                color: Theme.text
                font.pixelSize: 16
                font.weight: Font.Bold
                Layout.fillWidth: true
            }
        }

        Divider { width: parent.width }

        // Section 1: Brush Engine
        Rectangle {
            width: parent.width
            height: brushColumn.implicitHeight + 24
            color: "#1a1a20"
            radius: 8
            border.color: "#2a2a32"
            border.width: 1

            Column {
                id: brushColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Text {
                    text: "CORE BRUSH TYPE"
                    color: Theme.textMuted
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1
                }

                Flow {
                    width: parent.width
                    height: implicitHeight
                    spacing: 8

                    Repeater {
                        model: root.brushList
                        Button {
                            required property string modelData
                            text: modelData
                            highlighted: root.activeBrush === modelData
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: parent.highlighted ? "#000000" : Theme.text
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                implicitWidth: parent.contentItem.implicitWidth + 24
                                implicitHeight: 28
                                color: parent.highlighted ? "#38bdf8" : "#222228"
                                radius: 14
                                border.color: parent.highlighted ? "#38bdf8" : "#303038"
                            }
                            
                            onClicked: {
                                root.activeBrush = modelData
                                if (typeof EditorBackend !== "undefined" && EditorBackend.toolController) {
                                    EditorBackend.toolController.options.currentTool = modelData
                                }
                            }
                        }
                    }
                }
            }
        }

        // Section 2: Dynamics Settings
        Column {
            width: parent.width
            spacing: 10

            FeatureSlider {
                width: parent.width
                title: "Velocity Scaling"
                from: 0.1
                to: 3.0
                stepSize: 0.1
                value: typeof EditorBackend !== "undefined" && EditorBackend.paintDynamics ? EditorBackend.paintDynamics.velocityScaling : 1.0
                valueSuffix: "x"
                onValueModified: (val) => {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.paintDynamics) {
                        EditorBackend.paintDynamics.velocityScaling = val
                    }
                }
            }

            FeatureSlider {
                width: parent.width
                title: "Pressure Scaling"
                from: 0.1
                to: 3.0
                stepSize: 0.1
                value: typeof EditorBackend !== "undefined" && EditorBackend.paintDynamics ? EditorBackend.paintDynamics.pressureScaling : 1.0
                valueSuffix: "x"
                onValueModified: (val) => {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.paintDynamics) {
                        EditorBackend.paintDynamics.pressureScaling = val
                    }
                }
            }
        }

        Divider { width: parent.width }

        // Section 3: Blending
        Rectangle {
            width: parent.width
            height: blendColumn.implicitHeight + 24
            color: "#1a1a20"
            radius: 8
            border.color: "#2a2a32"
            border.width: 1

            Column {
                id: blendColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Text {
                    text: "ADVANCED BLENDING ACTIONS"
                    color: Theme.textMuted
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1
                }

                Flow {
                    width: parent.width
                    height: implicitHeight
                    spacing: 8
                    
                    Repeater {
                        model: root.blendingList
                        Button {
                            required property string modelData
                            text: modelData
                            highlighted: root.activeBlending === modelData
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: parent.highlighted ? "#ffffff" : Theme.text
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                implicitWidth: parent.contentItem.implicitWidth + 24
                                implicitHeight: 28
                                color: parent.highlighted ? "#bb86fc" : "#222228"
                                radius: 4
                                border.color: parent.highlighted ? "#bb86fc" : "#303038"
                            }

                            onClicked: {
                                root.activeBlending = modelData
                            }
                        }
                    }
                }
            }
        }
    }
}
