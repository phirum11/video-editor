pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: root
    property string title: "Text Color"
    property string selectedColor: "#ffffff"
    property var colorList: [
        "#ffffff", // White
        "#25e5cf", // Cyan
        "#ff3860", // Pink
        "#ffdd57", // Yellow
        "#38bdf8", // Vivid Blue
        "#10b981", // Emerald Green
        "#8b5cf6", // Electric Purple
        "#f97316"  // Sunset Orange
    ]
    signal colorSelected(string color)

    Layout.fillWidth: true
    Layout.preferredHeight: 84
    color: "#202026"
    radius: 10
    border.color: "#303038"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Text {
            text: root.title
            color: Theme.text
            font.pixelSize: 14
            font.weight: Font.Medium
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Repeater {
                model: root.colorList

                Rectangle {
                    required property string modelData
                    width: 28
                    height: 28
                    radius: 14
                    color: modelData
                    border.color: root.selectedColor === modelData ? "#ffffff" : "#303038"
                    border.width: root.selectedColor === modelData ? 2 : 1

                    // Add a subtle outer ring when selected
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: -4
                        color: "transparent"
                        border.color: root.selectedColor === modelData ? modelData : "transparent"
                        border.width: 1
                        radius: 18
                        visible: root.selectedColor === modelData
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.selectedColor = modelData
                            root.colorSelected(modelData)
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
