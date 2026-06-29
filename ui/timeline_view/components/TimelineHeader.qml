// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: headerRoot

    property color panelLine: Theme.divider
    property color textPrimary: Theme.text
    property color textMuted: Theme.textMuted
    property string activeTool: "selection"
    property bool snapEnabled: true
    property real zoomValue: 0
    property var timelineController: null

    signal closeRequested
    signal menuRequested
    signal toolSelected(string tool)
    signal snapToggled(bool enabled)
    signal markerRequested
    signal zoomInRequested
    signal zoomOutRequested
    signal zoomValueRequested(real value)
    signal generateAudioRequested(string language)
    signal autoEditRequested

    signal undoRequested
    signal redoRequested
    signal splitRequested
    signal deleteLeftRequested
    signal deleteRightRequested
    signal deleteRequested

    property bool hasSelection: false
    property bool playheadOverSelection: false

    height: 32
    color: Theme.background
    border.color: panelLine
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 6

        Text {
            text: "x"
            color: headerRoot.textMuted
            font.pixelSize: 12

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: headerRoot.closeRequested()
            }
        }

        Text {
            text: qsTr("Timeline - Sequence 1")
            color: headerRoot.textPrimary
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 18
            color: Theme.divider
            opacity: 0.85
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/undo.svg"
            toolTipText: qsTr("Undo")
            onClicked: headerRoot.undoRequested()
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/redo.svg"
            toolTipText: qsTr("Redo")
            onClicked: headerRoot.redoRequested()
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/split.svg"
            toolTipText: qsTr("Split(Ctrl+B)")
            enabled: headerRoot.playheadOverSelection
            onClicked: headerRoot.splitRequested()
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/delete-left.svg"
            toolTipText: qsTr("Delete left(Q)")
            enabled: headerRoot.playheadOverSelection
            onClicked: headerRoot.deleteLeftRequested()
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/delete-right.svg"
            toolTipText: qsTr("Delete right(W)")
            enabled: headerRoot.playheadOverSelection
            onClicked: headerRoot.deleteRightRequested()
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/trash.svg"
            toolTipText: qsTr("Delete")
            enabled: headerRoot.hasSelection
            onClicked: headerRoot.deleteRequested()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 18
            color: Theme.divider
            opacity: 0.85
            Layout.leftMargin: 8
            Layout.rightMargin: 8
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-pointer.svg"
            active: headerRoot.activeTool === "selection"
            toolTipText: qsTr("Selection Tool")
            onClicked: headerRoot.toolSelected("selection")
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-razor.svg"
            active: headerRoot.activeTool === "razor"
            toolTipText: qsTr("Razor Tool")
            onClicked: headerRoot.toolSelected("razor")
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-ripple.svg"
            active: headerRoot.activeTool === "ripple"
            toolTipText: qsTr("Ripple Edit Tool")
            onClicked: headerRoot.toolSelected("ripple")
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-slip.svg"
            active: headerRoot.activeTool === "slip"
            toolTipText: qsTr("Slip Tool")
            onClicked: headerRoot.toolSelected("slip")
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-magnet.svg"
            active: headerRoot.snapEnabled
            toolTipText: qsTr("Snap")
            onClicked: headerRoot.snapToggled(!headerRoot.snapEnabled)
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-hand.svg"
            active: headerRoot.activeTool === "hand"
            toolTipText: qsTr("Hand Tool")
            onClicked: headerRoot.toolSelected("hand")
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/tool-zoom.svg"
            active: headerRoot.activeTool === "zoom"
            toolTipText: qsTr("Zoom Tool")
            onClicked: headerRoot.toolSelected("zoom")
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/marker-pin.svg"
            toolTipText: qsTr("Add Marker")
            onClicked: headerRoot.markerRequested()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 18
            color: Theme.divider
            opacity: 0.85
            Layout.leftMargin: 8
            Layout.rightMargin: 8
        }

        ComboBox {
            id: aiLangCombo
            Layout.preferredHeight: 24
            Layout.preferredWidth: 130
            model: [qsTr("Khmer"), qsTr("English (Male)"), qsTr("English (Female)")]

            background: Rectangle {
                color: aiLangCombo.pressed ? Theme.surfacePressed : (aiLangCombo.hovered ? Theme.surfaceHover : "transparent")
                border.color: Theme.divider
                border.width: 1
                radius: 4
            }

            contentItem: Text {
                text: aiLangCombo.displayText
                color: headerRoot.textPrimary
                verticalAlignment: Text.AlignVCenter
                leftPadding: 8
                font.pixelSize: 12
            }

            indicator: Image {
                x: aiLangCombo.width - width - 8
                y: aiLangCombo.topPadding + (aiLangCombo.availableHeight - height) / 2
                width: 14
                height: 14
                source: aiLangCombo.popup.visible ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
                fillMode: Image.PreserveAspectFit
                opacity: aiLangCombo.enabled ? 0.8 : 0.3
            }

            delegate: ItemDelegate {
                id: itemDlg
                required property int index
                required property string modelData

                width: aiLangCombo.width - 8
                height: 28
                highlighted: aiLangCombo.highlightedIndex === itemDlg.index

                background: Rectangle {
                    color: itemDlg.highlighted ? Theme.surfaceHover : "transparent"
                    radius: 4
                }

                contentItem: Text {
                    text: itemDlg.modelData
                    color: itemDlg.highlighted ? Theme.accent : Theme.text
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 8
                }
            }

            popup: Popup {
                y: aiLangCombo.height + 4
                width: aiLangCombo.width
                implicitHeight: contentItem.implicitHeight + 8
                padding: 4

                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: aiLangCombo.popup.visible ? aiLangCombo.delegateModel : null
                    currentIndex: aiLangCombo.highlightedIndex
                    interactive: false
                }

                background: Rectangle {
                    color: Theme.surfaceRaised
                    border.color: Theme.divider
                    border.width: 1
                    radius: 6
                    layer.enabled: true
                }
            }
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/ai-voice.svg"
            toolTipText: qsTr("Generate AI Voice from SRT")
            enabled: headerRoot.timelineController && !headerRoot.timelineController.isGeneratingAIVoice && headerRoot.timelineController.hasSubtitleTrack
            opacity: enabled ? 1.0 : 0.4
            onClicked: headerRoot.generateAudioRequested(aiLangCombo.currentText)
        }

        HeaderIconButton {
            iconSource: "qrc:/VideoStudioUI/assets/sync-lock.svg"
            toolTipText: qsTr("Auto Edit (Sync Video/Images to Voice)")
            enabled: headerRoot.timelineController
            onClicked: headerRoot.autoEditRequested()
        }

        Item {
            Layout.fillWidth: true
        }

        AbstractButton {
            id: zoomOutBtn
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            hoverEnabled: true

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            enabled: headerRoot.zoomValue > 0
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Zoom Out")
            onClicked: headerRoot.zoomOutRequested()

            background: Rectangle {
                radius: 4
                color: zoomOutBtn.pressed ? Theme.surfacePressed : (zoomOutBtn.hovered ? Theme.surfaceHover : "transparent")
            }

            contentItem: Item {
                opacity: zoomOutBtn.enabled ? (zoomOutBtn.hovered ? 1.0 : 0.7) : 0.3
                Rectangle {
                    anchors.centerIn: parent
                    width: 12
                    height: 2
                    color: headerRoot.textPrimary
                    radius: 1
                }
            }
        }

        Slider {
            id: zoomSlider
            Layout.preferredWidth: 140
            Layout.preferredHeight: 24
            from: 0
            to: 1
            value: headerRoot.zoomValue
            live: true
            onMoved: headerRoot.zoomValueRequested(value)

            background: Rectangle {
                x: zoomSlider.leftPadding
                y: zoomSlider.topPadding + zoomSlider.availableHeight / 2 - height / 2
                width: zoomSlider.availableWidth
                height: 4
                radius: 2
                color: Theme.dividerSoft

                Rectangle {
                    width: zoomSlider.visualPosition * parent.width
                    height: parent.height
                    radius: parent.radius
                    color: "#58a8d8"
                }
            }

            handle: Item {
                x: zoomSlider.leftPadding + zoomSlider.visualPosition * (zoomSlider.availableWidth - width)
                y: zoomSlider.topPadding + zoomSlider.availableHeight / 2 - height / 2
                width: 14
                height: 14

                Rectangle {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    radius: 7
                    color: "#40000000"
                    y: 1
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 12
                    height: 12
                    radius: 6
                    color: zoomSlider.pressed ? "#e0e8eb" : Theme.text
                    border.color: "#58a8d8"
                    border.width: zoomSlider.hovered ? 2 : 1
                }
            }
        }

        AbstractButton {
            id: zoomInBtn
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            hoverEnabled: true

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            enabled: headerRoot.zoomValue < 1
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Zoom In")
            onClicked: headerRoot.zoomInRequested()

            background: Rectangle {
                radius: 4
                color: zoomInBtn.pressed ? Theme.surfacePressed : (zoomInBtn.hovered ? Theme.surfaceHover : "transparent")
            }

            contentItem: Item {
                opacity: zoomInBtn.enabled ? (zoomInBtn.hovered ? 1.0 : 0.7) : 0.3
                Rectangle {
                    anchors.centerIn: parent
                    width: 12
                    height: 2
                    color: headerRoot.textPrimary
                    radius: 1
                }
                Rectangle {
                    anchors.centerIn: parent
                    width: 2
                    height: 12
                    color: headerRoot.textPrimary
                    radius: 1
                }
            }
        }

        Text {
            text: "\u2261"
            color: headerRoot.textMuted
            font.pixelSize: 14

            HoverHandler {
                id: menuHover
            }
            ToolTip.visible: menuHover.hovered
            ToolTip.text: qsTr("Timeline Menu")

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: headerRoot.menuRequested()
            }
        }
    }

    component HeaderIconButton: AbstractButton {
        id: buttonRoot

        property url iconSource: ""
        property bool active: false
        property string toolTipText: ""

        Layout.preferredWidth: 24
        Layout.preferredHeight: 24
        hoverEnabled: true
        opacity: enabled ? 1.0 : 0.38

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        ToolTip.visible: hovered && toolTipText !== ""
        ToolTip.text: toolTipText

        background: Rectangle {
            radius: 4
            color: buttonRoot.active ? "#2d5264" : (buttonRoot.pressed ? Theme.surfacePressed : (buttonRoot.hovered ? Theme.surfaceHover : "transparent"))
            border.color: buttonRoot.active ? "#66aacf" : (buttonRoot.hovered ? Theme.divider : "transparent")
            border.width: 1
        }

        contentItem: Item {
            Image {
                anchors.centerIn: parent
                width: 15
                height: 15
                visible: buttonRoot.iconSource.toString().length > 0
                source: buttonRoot.iconSource
                fillMode: Image.PreserveAspectFit
                opacity: buttonRoot.active ? 1.0 : (buttonRoot.hovered ? 0.95 : 0.72)
            }

            Text {
                anchors.centerIn: parent
                visible: buttonRoot.iconSource.toString().length === 0
                text: buttonRoot.text
                color: headerRoot.textMuted
                font.pixelSize: 15
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
