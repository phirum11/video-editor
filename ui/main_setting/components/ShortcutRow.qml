// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

RowLayout {
    id: shortcutRowRoot
    property string action: ""
    property string shortcut: ""

    width: parent.width
    height: 32
    spacing: 24

    Text {
        Layout.fillWidth: true
        text: shortcutRowRoot.action
        color: settingsRoot.textMuted
        font.pixelSize: 14
    }

    Rectangle {
        Layout.preferredWidth: shortcutLabel.implicitWidth + 24
        Layout.preferredHeight: 26
        radius: 6
        color: Theme.surfaceInset
        border.color: settingsRoot.cardBorder
        border.width: 1

        Text {
            id: shortcutLabel
            anchors.centerIn: parent
            text: shortcutRowRoot.shortcut
            color: settingsRoot.accent
            font.pixelSize: 12
            font.family: "Consolas"
            font.weight: Font.DemiBold
        }
    }
}
