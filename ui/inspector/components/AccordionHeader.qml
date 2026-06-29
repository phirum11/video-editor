// qmllint disable
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI

Rectangle {
    id: accordionHeader
    property string title: ""
    property bool expanded: true
    property bool hasCheckbox: true
    property bool effectEnabled: true
    property url iconSource: ""
    property string tooltipText: ""
    signal resetClicked()
    signal effectEnabledToggled(bool enabled)

    Layout.fillWidth: true
    Layout.preferredHeight: 28
    color: Theme.surfaceRaised
    border.color: Theme.divider
    border.width: 1

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: accordionHeader.expanded = !accordionHeader.expanded
        ToolTip.visible: containsMouse && accordionHeader.tooltipText !== ""
        ToolTip.text: accordionHeader.tooltipText
        ToolTip.delay: 500
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 8

        Image {
            source: accordionHeader.expanded ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            opacity: 0.8
        }

        Rectangle {
            visible: accordionHeader.hasCheckbox
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            color: accordionHeader.effectEnabled ? "#2792c3" : "transparent"
            border.color: accordionHeader.effectEnabled ? "#2792c3" : "#5a6e7a"
            border.width: 1
            radius: 2
            Text {
                anchors.centerIn: parent
                text: "\u2713"
                color: '#ffffff'
                font.pixelSize: 11
                font.bold: true
                visible: accordionHeader.effectEnabled
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    accordionHeader.effectEnabled = !accordionHeader.effectEnabled
                    accordionHeader.effectEnabledToggled(accordionHeader.effectEnabled)
                }
            }
        }
        Image {
            visible: accordionHeader.iconSource.toString().length > 0
            source: accordionHeader.iconSource
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            opacity: 0.8
        }

        Text {
            Layout.fillWidth: true
            text: accordionHeader.title
            color: "#d0d0d0"
            font.pixelSize: 12
        }
        
        Image {
            visible: accordionHeader.hasCheckbox
            source: "qrc:/VideoStudioUI/assets/undo.svg"
            Layout.preferredWidth: 12
            Layout.preferredHeight: 12
            opacity: undoMouseArea.containsMouse ? 1.0 : 0.6
            
            MouseArea {
                id: undoMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: accordionHeader.resetClicked()
            }
            ToolTip.visible: undoMouseArea.containsMouse
            ToolTip.text: "Reset Effect"
        }
    }
}
