import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

RowLayout {
    id: colorRowRoot
    property string label: ""
    property color colorValue: "#ffffff"

    width: parent.width
    height: 36
    spacing: 24

    Text {
        Layout.preferredWidth: 220
        text: colorRowRoot.label
        color: Theme.textMuted
        font.pixelSize: 14
        elide: Text.ElideRight
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        color: Theme.surfaceHover
        border.color: Theme.divider
        border.width: 1
        radius: 6
        
        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
        }

        Rectangle {
            anchors.fill: parent
            color: "#ffffff"
            opacity: hoverArea.containsMouse ? 0.04 : 0.0
            radius: 6
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 6
            anchors.leftMargin: 8
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                radius: 4
                color: colorRowRoot.colorValue
                border.color: "#33ffffff"
                border.width: 1
            }

            Text {
                Layout.fillWidth: true
                text: colorRowRoot.colorValue.toString().toUpperCase()
                color: Theme.text
                font.pixelSize: 13
                font.family: "Consolas"
            }
        }
    }
}
