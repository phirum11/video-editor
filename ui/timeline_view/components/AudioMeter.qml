// qmllint disable
import QtQuick
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: meterRoot
    color: Theme.background
    implicitWidth: 48
    property real leftLevel: 0.0
    property real rightLevel: 0.0

    function normalizedLevel(level) {
        const clamped = Math.max(0.0001, Math.min(1.0, level))
        const db = 20.0 * Math.log10(clamped)
        return Math.max(0.0, Math.min(1.0, (db + 45.0) / 45.0))
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.background
        border.color: Theme.divider
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 2

            // Top clipping indicators
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 2
                Rectangle {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: 3
                    color: meterRoot.leftLevel > 0.98 ? "#ff4545" : "#542020"
                }
                Rectangle {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: 3
                    color: meterRoot.rightLevel > 0.98 ? "#ff4545" : "#542020"
                }
            }

            // Bars and Scale
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4

                // Bars Container
                RowLayout {
                    Layout.fillHeight: true
                    spacing: 2

                    // Left Channel
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 10
                        
                        // Full height gradient bar
                        Rectangle {
                            anchors.fill: parent
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#ff5252" }
                                GradientStop { position: 0.15; color: "#ffeb3b" }
                                GradientStop { position: 0.3; color: "#4caf50" }
                                GradientStop { position: 1.0; color: "#388e3c" }
                            }
                        }
                        
                        // Animated cover mask
                        Rectangle {
                            id: leftMask
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: parent.height * (1.0 - meterRoot.normalizedLevel(meterRoot.leftLevel))
                            color: "#1b2820" // Dark meter background

                            Behavior on height {
                                NumberAnimation { duration: 60; easing.type: Easing.OutCubic }
                            }
                        }

                        // LED gaps overlay
                        Column {
                            anchors.fill: parent
                            Repeater {
                                model: 80
                                Rectangle { width: parent.width; height: 2; color: "transparent"; border.color: "#181818"; border.width: 1; opacity: 0.8 }
                            }
                        }
                    }

                    // Right Channel
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 10
                        
                        // Full height gradient bar
                        Rectangle {
                            anchors.fill: parent
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#ff5252" }
                                GradientStop { position: 0.15; color: "#ffeb3b" }
                                GradientStop { position: 0.3; color: "#4caf50" }
                                GradientStop { position: 1.0; color: "#388e3c" }
                            }
                        }
                        
                        // Animated cover mask
                        Rectangle {
                            id: rightMask
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: parent.height * (1.0 - meterRoot.normalizedLevel(meterRoot.rightLevel))
                            color: "#1b2820"

                            Behavior on height {
                                NumberAnimation { duration: 60; easing.type: Easing.OutCubic }
                            }
                        }

                        // LED gaps overlay
                        Column {
                            anchors.fill: parent
                            Repeater {
                                model: 80
                                Rectangle { width: parent.width; height: 2; color: "transparent"; border.color: "#181818"; border.width: 1; opacity: 0.8 }
                            }
                        }
                    }
                }

                // Scale
                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 18

                    Repeater {
                        model: [
                            { val: "0", pos: 0.05 },
                            { val: "-5", pos: 0.20 },
                            { val: "-10", pos: 0.35 },
                            { val: "-15", pos: 0.50 },
                            { val: "-20", pos: 0.65 },
                            { val: "-25", pos: 0.80 },
                            { val: "-30", pos: 0.95 }
                        ]
                        Item {
                            id: scaleMark
                            required property var modelData

                            width: parent.width
                            height: 10
                            y: parent.height * scaleMark.modelData.pos - height / 2
                            
                            Rectangle {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                width: 3
                                height: 1
                                color: "#8a9ba3"
                            }
                            
                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                text: scaleMark.modelData.val
                                color: "#8a9ba3"
                                font.pixelSize: 9
                                font.weight: Font.DemiBold
                                font.family: "Segoe UI"
                            }
                        }
                    }
                    
                    // Add minor ticks
                    Repeater {
                        model: [0.125, 0.275, 0.425, 0.575, 0.725, 0.875]
                        Rectangle {
                            required property real modelData

                            x: 0
                            y: parent.height * modelData
                            width: 2
                            height: 1
                            color: "#52656e"
                        }
                    }
                }
            }

            // Bottom Text
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "1 | 2"
                color: "#8a9ba3"
                font.pixelSize: 9
                font.weight: Font.DemiBold
                font.family: "Segoe UI"
                Layout.bottomMargin: 2
            }
        }
    }
}
