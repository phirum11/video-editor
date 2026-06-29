// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

RowLayout {
    id: comboRowRoot
    property string label: ""
    property var model: []
    property alias currentIndex: comboBox.currentIndex

    width: parent.width
    height: 36
    spacing: 24

    Text {
        Layout.preferredWidth: 220
        text: comboRowRoot.label
        color: Theme.textMuted
        font.pixelSize: 14
        elide: Text.ElideRight
    }

    ComboBox {
        id: comboBox
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        model: comboRowRoot.model
        currentIndex: comboRowRoot.currentIndex

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        leftPadding: 12
        rightPadding: 32

        background: Rectangle {
            color: Theme.surfaceHover
            border.color: Theme.divider
            border.width: 1
            radius: 6

            Rectangle {
                anchors.fill: parent
                color: Theme.text
                opacity: comboBox.hovered ? 0.04 : 0.0
                radius: 6
                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
        }

        contentItem: Text {
            text: comboBox.displayText
            color: Theme.text
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        indicator: Image {
            x: comboBox.width - width - 12
            y: comboBox.topPadding + (comboBox.availableHeight - height) / 2
            source: "qrc:/VideoStudioUI/assets/chevron-down.svg"
            width: 16
            height: 16
            sourceSize: Qt.size(16, 16)
            opacity: 0.7
        }

        popup: Popup {
            y: parent.height - 1
            width: parent.width
            implicitHeight: contentItem.implicitHeight
            padding: 4

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: comboBox.popup.visible ? comboBox.delegateModel : null
                currentIndex: comboBox.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: Theme.surfaceRaised
                border.color: Theme.divider
                radius: 6
            }
        }

        delegate: ItemDelegate {
            width: parent.width
            height: 32
            highlighted: comboBox.highlightedIndex === index
            
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            
            background: Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: 4
                color: highlighted ? Theme.surfaceHover : "transparent"
            }

            contentItem: Text {
                text: modelData
                color: highlighted ? Theme.text : Theme.textMuted
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
