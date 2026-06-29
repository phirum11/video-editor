pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: root
    property string title: "Scale"
    property real from: 0.5
    property real to: 2.0
    property real stepSize: 0.1
    property real value: 1.0
    property string valueSuffix: "x"
    signal valueModified(real value)

    implicitWidth: 200
    implicitHeight: 74
    Layout.fillWidth: true
    Layout.preferredHeight: 74
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: root.title
                color: Theme.text
                font.pixelSize: 14
                font.weight: Font.Medium
                Layout.fillWidth: true
            }

            Text {
                text: (Math.round(root.value * 100) / 100) + root.valueSuffix
                color: "#38bdf8"
                font.pixelSize: 13
                font.weight: Font.Bold
            }
        }

        Slider {
            id: control
            Layout.fillWidth: true
            from: root.from
            to: root.to
            stepSize: root.stepSize
            value: root.value
            onValueChanged: {
                if (root.value !== value) {
                    root.value = value
                    root.valueModified(value)
                }
            }

            background: Rectangle {
                x: control.leftPadding
                y: control.topPadding + control.availableHeight / 2 - height / 2
                implicitWidth: 200
                implicitHeight: 6
                width: control.availableWidth
                height: 6
                radius: 3
                color: "#303038"

                Rectangle {
                    width: control.visualPosition * parent.width
                    height: parent.height
                    color: "#38bdf8"
                    radius: 3
                }
            }

            handle: Rectangle {
                x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
                y: control.topPadding + control.availableHeight / 2 - height / 2
                width: 18
                height: 18
                radius: 9
                color: "#ffffff"
                border.color: "#38bdf8"
                border.width: 2
            }
        }
    }
}
