pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: root
    property string text: ""
    property bool checked: false
    signal toggled(bool checked)

    Layout.fillWidth: true
    Layout.preferredHeight: 54
    color: "#202026"
    radius: 10
    border.color: "#303038"
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16

        Text {
            text: root.text
            color: Theme.text
            font.pixelSize: 14
            font.weight: Font.Medium
            Layout.fillWidth: true
        }

        Switch {
            id: control
            checked: root.checked
            onCheckedChanged: {
                if (root.checked !== checked) {
                    root.checked = checked
                    root.toggled(checked)
                }
            }

            indicator: Rectangle {
                implicitWidth: 46
                implicitHeight: 26
                x: control.leftPadding
                y: parent.height / 2 - height / 2
                radius: 13
                color: control.checked ? "#38bdf8" : "#303038" // Vibrant blue/cyan toggle track
                border.color: control.checked ? "#38bdf8" : "#454550"
                border.width: 1

                Rectangle {
                    x: control.checked ? parent.width - width - 3 : 3
                    y: 3
                    width: 20
                    height: 20
                    radius: 10
                    color: "#ffffff"
                    
                    Behavior on x {
                        NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                    }
                }
            }
        }
    }
}
