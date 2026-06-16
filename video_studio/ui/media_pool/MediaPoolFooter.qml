pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: footerRoot

    property string viewMode: "grid"
    property real zoomValue: 0.5
    property color panelLine: Theme.divider
    property color textPrimary: "#dce4e7"
    property color textMuted: "#aeb9be"
    property color accent: "#66aacf"

    signal viewModeRequested(string mode)
    signal zoomValueRequested(real value)
    signal newBinRequested()
    signal panelToggled()

    color: Theme.background
    border.color: panelLine
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 8

        ToolIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/list-view.svg"
            active: footerRoot.viewMode === "list"
            toolTipText: qsTr("List View")
            onClicked: footerRoot.viewModeRequested("list")
        }

        ToolIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/grid-view.svg"
            active: footerRoot.viewMode === "grid"
            toolTipText: qsTr("Grid View")
            onClicked: footerRoot.viewModeRequested("grid")
        }

        ToolIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/folder.svg"
            toolTipText: qsTr("New Bin")
            onClicked: footerRoot.newBinRequested()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 18
            color: Theme.divider
        }

        Slider {
            id: zoomSlider
            Layout.preferredWidth: 78
            Layout.preferredHeight: 20
            from: 0
            to: 1
            value: footerRoot.zoomValue
            onMoved: footerRoot.zoomValueRequested(value)

            background: Rectangle {
                x: zoomSlider.leftPadding
                y: zoomSlider.topPadding + zoomSlider.availableHeight / 2 - height / 2
                width: zoomSlider.availableWidth
                height: 3
                color: Theme.dividerSoft
                radius: 1

                Rectangle {
                    width: zoomSlider.visualPosition * parent.width
                    height: parent.height
                    color: "#89979d"
                    radius: 1
                }
            }

            handle: Rectangle {
                x: zoomSlider.leftPadding + zoomSlider.visualPosition * (zoomSlider.availableWidth - width)
                y: zoomSlider.topPadding + zoomSlider.availableHeight / 2 - height / 2
                implicitWidth: 12
                implicitHeight: 12
                radius: 6
                color: zoomSlider.pressed ? "#ffffff" : footerRoot.textPrimary
            }
        }

        Item { Layout.fillWidth: true }

        ToolIconButton {
            id: mediaPoolSettingsBtn
            iconSource: "qrc:/VideoStudioUI/assets/settings.svg"
            toolTipText: qsTr("Settings")
            onClicked: mediaPoolSettingsPopup.open()
        }

        ToolIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/panel.svg"
            toolTipText: qsTr("Panel Options")
            onClicked: footerRoot.panelToggled()
        }
    }

    Popup {
        id: mediaPoolSettingsPopup
        x: mediaPoolSettingsBtn.x - width + mediaPoolSettingsBtn.width + 10
        y: -height - 4
        width: 220
        padding: 0

        background: Rectangle {
            color: Theme.surfaceInset
            border.color: Theme.divider
            border.width: 1
            radius: 6
        }

        contentItem: Column {
            spacing: 0
            padding: 6

            SettingsToggle { label: qsTr("Show Thumbnails"); checked: true }
            SettingsToggle { label: qsTr("Show Audio Waveforms"); checked: true }
            SettingsToggle { label: qsTr("Show Keyframes"); checked: false }

            Rectangle {
                width: parent.width - 12
                height: 1
                color: Theme.divider
                anchors.horizontalCenter: parent.horizontalCenter
            }

            SettingsAction { label: qsTr("Minimize All Tracks") }
            SettingsAction { label: qsTr("Expand All Tracks") }
        }
    }

    component ToolIconButton: AbstractButton {
        id: toolButton
        property url iconSource: ""
        property bool active: false
        property string toolTipText: ""

        implicitWidth: 24
        implicitHeight: 24
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        ToolTip.visible: hovered && toolTipText !== ""
        ToolTip.text: toolTipText

        background: Rectangle {
            radius: 3
            color: toolButton.active ? "#263e4b" : toolButton.hovered ? "#263741" : "transparent"
            border.color: toolButton.active ? footerRoot.accent : "transparent"
            border.width: toolButton.active ? 1 : 0
        }

        contentItem: Image {
            anchors.centerIn: parent
            width: 15
            height: 15
            source: toolButton.iconSource
            opacity: toolButton.active || toolButton.hovered ? 1 : 0.72
        }
    }

    component SettingsToggle: Rectangle {
        id: settingsToggle
        property string label: ""
        property bool checked: false

        width: parent ? parent.width - 12 : 200
        height: 32
        radius: 4
        color: toggleMouse.containsMouse ? "#1c3947" : "transparent"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 14
                Layout.preferredHeight: 14
                color: "transparent"
                border.color: footerRoot.accent
                border.width: 1
                radius: 2

                Rectangle {
                    anchors.centerIn: parent
                    width: 8
                    height: 8
                    color: footerRoot.accent
                    visible: settingsToggle.checked
                    radius: 1
                }
            }

            Text {
                Layout.fillWidth: true
                text: settingsToggle.label
                color: "#d6e0e4"
                font.pixelSize: 13
            }
        }

        MouseArea {
            id: toggleMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: settingsToggle.checked = !settingsToggle.checked
        }
    }

    component SettingsAction: Rectangle {
        id: settingsAction
        property string label: ""

        width: parent ? parent.width - 12 : 200
        height: 32
        radius: 4
        color: actionMouse.containsMouse ? "#1c3947" : "transparent"

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 34
            text: settingsAction.label
            color: "#d6e0e4"
            font.pixelSize: 13
        }

        MouseArea {
            id: actionMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
        }
    }
}
