// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import VideoStudioUI

Item {
    id: root
    
    property var timelineCtrl: null
    property var subtitleCtrl: null
    property real currentTime: 0
    property string currentText: ""
    
    property color fontColor: Theme.text
    property color bgColor: "#99000000"
    
    // We bind to the global font and color, with a fallback if subtitleCtrl isn't ready
    property font currentFont: subtitleCtrl ? subtitleCtrl.font : Qt.font({pixelSize: 12, weight: Font.Bold})
    property color currentColor: subtitleCtrl ? subtitleCtrl.color : Theme.text
    
    clip: true
    
    onCurrentTimeChanged: {
        if (timelineCtrl && subtitleCtrl) {
            currentText = subtitleCtrl.getSubtitleAtTime(timelineCtrl, currentTime)
        }
    }
    
    FontDialog {
        id: fontDialog
        title: "Choose Subtitle Font"
        onAccepted: {
            if (root.subtitleCtrl) {
                root.subtitleCtrl.font = selectedFont
            }
        }
    }
    
    ColorDialog {
        id: textColorDialog
        title: "Choose Text Color"
        onAccepted: {
            if (root.subtitleCtrl) {
                root.subtitleCtrl.color = selectedColor
            }
        }
    }
    
    ColorDialog {
        id: bgColorDialog
        title: "Choose Background Color"
        onAccepted: {
            root.bgColor = selectedColor
        }
    }
    
    property bool noBackground: false
    property bool isHidden: false
    
    Menu {
        id: contextMenu
        
        background: Rectangle {
            implicitWidth: 220
            color: "#1a1a24"
            border.color: "#2a2a3a"
            radius: 6
        }
        
        MenuItem {
            id: fontItem
            text: qsTr("Change Font...")
            contentItem: Text {
                text: fontItem.text
                color: Theme.text
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 34
                color: fontItem.highlighted ? "#3a3a4a" : "transparent"
            }
            onTriggered: {
                fontDialog.selectedFont = root.currentFont
                fontDialog.open()
            }
        }
        MenuItem {
            id: textColorItem
            text: qsTr("Change Text Color...")
            contentItem: Text {
                text: textColorItem.text
                color: Theme.text
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 34
                color: textColorItem.highlighted ? "#3a3a4a" : "transparent"
            }
            onTriggered: {
                textColorDialog.selectedColor = root.currentColor
                textColorDialog.open()
            }
        }
        MenuItem {
            id: bgColorItem
            text: qsTr("Change Background Color...")
            contentItem: Text {
                text: bgColorItem.text
                color: Theme.text
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 34
                color: bgColorItem.highlighted ? "#3a3a4a" : "transparent"
            }
            onTriggered: {
                bgColorDialog.selectedColor = root.bgColor
                bgColorDialog.open()
            }
        }
        MenuSeparator {
            contentItem: Rectangle {
                implicitWidth: 220
                implicitHeight: 1
                color: "#2a2a3a"
            }
        }
        MenuItem {
            id: noBgItem
            text: qsTr("No Background")
            checkable: true
            checked: root.noBackground
            contentItem: Text {
                text: noBgItem.checked ? "✓ " + noBgItem.text : "   " + noBgItem.text
                color: Theme.text
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 34
                color: noBgItem.highlighted ? "#3a3a4a" : "transparent"
            }
            onTriggered: {
                root.noBackground = !root.noBackground
            }
        }
        MenuItem {
            id: hideItem
            text: qsTr("Hide Subtitle")
            checkable: true
            checked: root.isHidden
            contentItem: Text {
                text: hideItem.checked ? "✓ " + hideItem.text : "   " + hideItem.text
                color: Theme.text
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }
            background: Rectangle {
                implicitWidth: 220
                implicitHeight: 34
                color: hideItem.highlighted ? "#3a3a4a" : "transparent"
            }
            onTriggered: {
                root.isHidden = !root.isHidden
            }
        }
    }

    Rectangle {
        id: subtitleContainer
        
        visible: root.currentText.length > 0 && !root.isHidden
        
        // Dynamic sizing based on text content
        width: subtitleText.implicitWidth + 32
        height: subtitleText.implicitHeight + 16
        
        anchors.horizontalCenter: parent.horizontalCenter
        
        onYChanged: {
            if (root.height > 0 && root.subtitleCtrl && dragHandler.active) {
                root.subtitleCtrl.verticalPosition = y / root.height
            }
        }
        
        property bool isCentered: false
        
        Connections {
            target: root
            function onHeightChanged() {
                if (!subtitleContainer.isCentered && root.height > 0) {
                    subtitleContainer.y = root.height - subtitleContainer.height - 30
                    subtitleContainer.isCentered = true
                }
            }
        }
        
        Component.onCompleted: {
            if (root.height > 0) {
                if (root.subtitleCtrl && root.subtitleCtrl.verticalPosition >= 0.0) {
                    y = root.height * root.subtitleCtrl.verticalPosition
                } else {
                    y = root.height - height - 30
                }
                isCentered = true
            }
        }
        
        color: root.noBackground ? "transparent" : root.bgColor
        radius: 8
        
        Text {
            id: subtitleText
            anchors.centerIn: parent
            text: root.currentText
            color: root.currentColor
            font: root.currentFont
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        
        DragHandler {
            id: dragHandler
            target: subtitleContainer
            cursorShape: Qt.OpenHandCursor
            xAxis.enabled: false
            
            onActiveChanged: {
                if (active) {
                    cursorShape = Qt.ClosedHandCursor;
                } else {
                    cursorShape = Qt.OpenHandCursor;
                }
            }
        }
        
        // Mouse area strictly for the right-click menu
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            
            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    contextMenu.popup()
                }
            }
        }
        
        // Give some visual feedback when hovering
        HoverHandler {
            id: hoverHandler
            cursorShape: dragHandler.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        }
        // Subtle border when hovering to indicate it's interactive
        border.color: hoverHandler.hovered || contextMenu.opened ? "#5ec4e8" : "transparent"
        border.width: hoverHandler.hovered || contextMenu.opened ? 2 : 0
    }
}
