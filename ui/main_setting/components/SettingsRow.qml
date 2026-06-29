// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

RowLayout {
    id: settingsRowRoot
    property string label: ""
    property string value: ""
    property bool isPath: false

    width: parent.width
    height: 36
    spacing: 24

    Text {
        Layout.preferredWidth: 220
        text: settingsRowRoot.label
        color: settingsRoot.textMuted
        font.pixelSize: 14
        elide: Text.ElideRight
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        color: Theme.surfaceInset
        border.color: settingsRoot.cardBorder
        border.width: 1
        radius: 6
        
        // Hover effect for the value box
        HoverHandler {
            id: hoverHandler
            cursorShape: Qt.IBeamCursor
        }

        Rectangle {
            anchors.fill: parent
            color: Theme.text
            opacity: hoverHandler.hovered ? 0.03 : 0.0
            radius: 6
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        TextInput {
            id: inputField
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: settingsRowRoot.value
            color: settingsRoot.textPrimary
            font.pixelSize: 13
            font.family: settingsRowRoot.isPath ? "Consolas" : Qt.application.font.family
            clip: true
            selectByMouse: true
            onTextEdited: settingsRowRoot.value = text
            onEditingFinished: settingsRowRoot.value = text
        }
    }
}
