// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import VideoStudioUI

AbstractButton {
    id: sidebarItem
    property string label: ""
    property url iconSource: ""
    property int sectionIndex: 0
    property bool active: false

    Layout.fillWidth: true
    Layout.leftMargin: 12
    Layout.rightMargin: 12
    implicitHeight: 38
    hoverEnabled: true

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    onClicked: settingsRoot.activeSection = sidebarItem.sectionIndex

    background: Rectangle {
        radius: 6
        color: sidebarItem.active ? "#3358a8d8" : (sidebarItem.hovered ? Theme.surfaceHover : "transparent")
        border.color: sidebarItem.active ? settingsRoot.accent : "transparent"
        border.width: 1
        Behavior on color { ColorAnimation { duration: 150 } }
    }

    contentItem: Item {
        Image {
            id: iconImg
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            source: sidebarItem.iconSource
            width: 16
            height: 16
            sourceSize: Qt.size(16, 16)
            fillMode: Image.PreserveAspectFit
            layer.enabled: true
            layer.effect: MultiEffect {
                brightness: 1.0
                colorization: 1.0
                colorizationColor: sidebarItem.active ? Theme.text : (sidebarItem.hovered ? settingsRoot.textPrimary : settingsRoot.textMuted)
            }
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 44
            anchors.verticalCenter: parent.verticalCenter
            text: sidebarItem.label
            color: sidebarItem.active ? settingsRoot.accent : (sidebarItem.hovered ? settingsRoot.textPrimary : settingsRoot.textMuted)
            font.pixelSize: 14
            font.weight: sidebarItem.active ? Font.DemiBold : Font.Medium
        }
    }
}
