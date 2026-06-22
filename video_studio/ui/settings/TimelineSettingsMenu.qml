import QtQuick
import QtQuick.Controls
import VideoStudioUI

Menu {
    id: settingsMenu
    
    width: 220

    background: Rectangle {
        color: Theme.surfaceInset
        border.color: Theme.divider
        border.width: 1
        radius: 4
    }

    component SettingsMenuItem: MenuItem {
        id: menuItem
        property bool isCheckable: false
        property bool isChecked: false
        
        implicitHeight: 32
        
        background: Rectangle {
            implicitHeight: 32
            color: menuItem.highlighted ? Theme.surfaceHover : "transparent"
            radius: 2
        }
        
        contentItem: Item {
            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 12
                spacing: 10
                
                // Checkbox simulation
                Rectangle {
                    visible: menuItem.isCheckable
                    width: 14
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter
                    color: "transparent"
                    border.color: "#58a8d8"
                    border.width: 1
                    radius: 2
                    
                    Rectangle {
                        anchors.centerIn: parent
                        width: 8
                        height: 8
                        color: "#58a8d8"
                        visible: menuItem.isChecked
                        radius: 1
                    }
                }
                
                Text {
                    text: menuItem.text
                    color: Theme.text
                    font.pixelSize: 13
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
        
        onClicked: {
            if (isCheckable) {
                isChecked = !isChecked
            }
        }
    }

    SettingsMenuItem {
        text: "Show Video Thumbnails"
        isCheckable: true
        isChecked: true
    }
    
    SettingsMenuItem {
        text: "Show Audio Waveforms"
        isCheckable: true
        isChecked: true
    }

    SettingsMenuItem {
        text: "Show Keyframe Controls"
        isCheckable: true
        isChecked: false
    }

    MenuSeparator {
        contentItem: Rectangle {
            implicitWidth: 200
            implicitHeight: 1
            color: Theme.divider
        }
    }

    SettingsMenuItem {
        text: "Minimize All Tracks"
    }

    SettingsMenuItem {
        text: "Expand All Tracks"
    }
}
