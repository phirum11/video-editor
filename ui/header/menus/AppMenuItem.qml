import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

MenuItem {
    id: rootMenuItem
    property string actionId
    property string shortcutText: ""
    
    implicitWidth: 220
    implicitHeight: 28
    
    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
    
    contentItem: Item {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 12
            spacing: 8
            
            Text {
                text: rootMenuItem.text
                color: rootMenuItem.highlighted ? Theme.text : "#dce4e7"
                font.pixelSize: 13
                Layout.fillWidth: true
            }
            Text {
                text: rootMenuItem.shortcutText
                color: rootMenuItem.highlighted ? Theme.text : "#6a7b83"
                font.pixelSize: 12
                visible: rootMenuItem.shortcutText.length > 0
            }
            Text {
                text: ">"
                color: rootMenuItem.highlighted ? Theme.text : "#6a7b83"
                font.pixelSize: 12
                visible: rootMenuItem.subMenu !== null
            }
        }
    }
    
    background: Rectangle {
        implicitWidth: 220
        implicitHeight: 28
        x: 2; y: 1
        width: parent.width - 4
        height: parent.height - 2
        color: rootMenuItem.highlighted ? "#2792c3" : "transparent"
        radius: 3
    }
}
