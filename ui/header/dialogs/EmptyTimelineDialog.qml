// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Dialog {
    id: emptyTimelineDialog

    parent: Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: 320
    implicitHeight: contentLayout.implicitHeight + topPadding + bottomPadding
    topPadding: 32
    bottomPadding: 24
    leftPadding: 24
    rightPadding: 24
    modal: true

    background: Rectangle {
        color: Theme.surfaceRaised
        radius: 12
        border.color: Theme.divider
        border.width: 1
        
        // Add a subtle drop shadow
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            z: -1
            color: "transparent"
            border.color: "#000000"
            border.width: 1
            radius: 13
            opacity: 0.5
        }
    }

    contentItem: Item {
        implicitHeight: contentLayout.implicitHeight
        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            spacing: 20

            // Warning Icon
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64
                radius: 32
                color: "#1a1a1a" // Darker inset
                border.color: "#333333"
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: "!"
                    font.pixelSize: 32
                    font.weight: Font.Black
                    color: "#f2c94c"
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "Timeline is empty"
                color: Theme.text
                font.pixelSize: 18
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                text: "Couldn't export because there are no clips on the timeline. Add some clips and try again."
                color: Theme.textMuted ? Theme.textMuted : "#a0a0a0"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                lineHeight: 1.4
            }

            Item {
                Layout.preferredHeight: 8 // small spacer
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 120
                Layout.preferredHeight: 36
                text: "OK"

                contentItem: Text {
                    text: parent.text
                    color: parent.hovered ? Theme.text : "#000000"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.hovered ? "#38ffe8" : "#25e5cf"
                    radius: 18
                    
                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }

                onClicked: emptyTimelineDialog.accept()
            }
        }
    }
}
