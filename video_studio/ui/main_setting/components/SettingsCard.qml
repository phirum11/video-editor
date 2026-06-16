import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: card
    property string title: ""
    default property alias innerRows: innerLayout.data

    width: parent.width
    implicitHeight: cardLayout.implicitHeight + 40
    color: settingsRoot.cardBg
    border.color: settingsRoot.cardBorder
    border.width: 1
    radius: 10

    Column {
        id: cardLayout
        y: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 20

        Text {
            text: card.title
            color: settingsRoot.textPrimary
            font.pixelSize: 15
            font.weight: Font.DemiBold
        }

        Column {
            id: innerLayout
            width: parent.width
            spacing: 16
        }
    }
}
