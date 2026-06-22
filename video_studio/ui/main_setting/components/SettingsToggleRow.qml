import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

RowLayout {
    id: settingsToggleRowRoot
    property string label: ""
    property bool checked: false

    width: parent.width
    height: 36
    spacing: 24

    Text {
        Layout.fillWidth: true
        text: settingsToggleRowRoot.label
        color: settingsRoot.textMuted
        font.pixelSize: 14
        elide: Text.ElideRight
    }

    Rectangle {
        Layout.preferredWidth: 44
        Layout.preferredHeight: 22
        radius: 11
        color: settingsToggleRowRoot.checked ? settingsRoot.accent : "#2a3640"
        border.color: settingsToggleRowRoot.checked ? settingsRoot.accent : settingsRoot.cardBorder
        border.width: 1
        
        Behavior on color { ColorAnimation { duration: 200 } }

        Rectangle {
            x: settingsToggleRowRoot.checked ? parent.width - width - 2 : 2
            anchors.verticalCenter: parent.verticalCenter
            width: 18
            height: 18
            radius: 9
            color: Theme.text

            Behavior on x {
                NumberAnimation { duration: 200; easing.type: Easing.OutBack }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: settingsToggleRowRoot.checked = !settingsToggleRowRoot.checked
        }
    }
}
