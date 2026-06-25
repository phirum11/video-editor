import QtQuick
import QtQuick.Controls
import VideoStudioUI

Rectangle {
    id: toolbarRoot

    property string activeTool: "selection"
    property bool snapEnabled: true

    signal toolSelected(string tool)
    signal snapToggled(bool enabled)
    signal zoomInRequested()
    signal zoomOutRequested()
    signal markerRequested()

    width: 36
    color: Theme.background
    border.color: Theme.divider
    border.width: 1

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOff
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            topPadding: 20
            bottomPadding: 20
            spacing: 15

            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-pointer.svg"
                active: toolbarRoot.activeTool === "selection"
                toolTipText: qsTr("Selection Tool")
                onClicked: toolbarRoot.toolSelected("selection")
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-razor.svg"
                active: toolbarRoot.activeTool === "razor"
                toolTipText: qsTr("Razor Tool")
                onClicked: toolbarRoot.toolSelected("razor")
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-ripple.svg"
                active: toolbarRoot.activeTool === "ripple"
                toolTipText: qsTr("Ripple Edit Tool")
                onClicked: toolbarRoot.toolSelected("ripple")
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-slip.svg"
                active: toolbarRoot.activeTool === "slip"
                toolTipText: qsTr("Slip Tool")
                onClicked: toolbarRoot.toolSelected("slip")
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-magnet.svg"
                active: toolbarRoot.snapEnabled
                toolTipText: qsTr("Snap")
                onClicked: toolbarRoot.snapToggled(!toolbarRoot.snapEnabled)
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-hand.svg"
                active: toolbarRoot.activeTool === "hand"
                toolTipText: qsTr("Hand Tool")
                onClicked: toolbarRoot.toolSelected("hand")
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/tool-zoom.svg"
                active: toolbarRoot.activeTool === "zoom"
                toolTipText: qsTr("Zoom Tool")
                onClicked: toolbarRoot.toolSelected("zoom")
                onDoubleClicked: toolbarRoot.zoomInRequested()
            }
            TimelineToolButton {
                iconSource: "qrc:/VideoStudioUI/assets/marker-pin.svg"
                toolTipText: qsTr("Add Marker")
                onClicked: toolbarRoot.markerRequested()
            }
        }
    }

    component TimelineToolButton: AbstractButton {
        id: btn
        property url iconSource: ""
        property bool active: false
        property string toolTipText: ""

        implicitWidth: 30
        implicitHeight: 30
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        ToolTip.visible: hovered && toolTipText !== ""
        ToolTip.text: toolTipText

        background: Rectangle {
            radius: 4
            color: btn.active ? "#2e4a58" : (btn.hovered ? Theme.surfaceHover : "transparent")
            border.color: btn.active ? "#66aacf" : "transparent"
            border.width: 1
        }

        contentItem: Image {
            anchors.centerIn: parent
            width: 16
            height: 16
            source: btn.iconSource
            fillMode: Image.PreserveAspectFit
            opacity: btn.active ? 1.0 : (btn.hovered ? 0.9 : 0.6)
            antialiasing: true
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onClicked: btn.clicked()
            onDoubleClicked: btn.doubleClicked()
        }
    }
}
