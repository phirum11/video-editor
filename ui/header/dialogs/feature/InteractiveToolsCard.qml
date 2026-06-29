pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI
import VideoStudio.Core

Rectangle {
    id: root
    Layout.fillWidth: true
    implicitHeight: mainLayout.implicitHeight + 28
    color: "#202026"
    radius: 12
    border.color: "#303038"
    border.width: 1

    property var selectionModes: ["Replace", "Add", "Subtract", "Intersect"]

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        Text {
            text: "Interactive Tools & Utilities"
            color: Theme.text
            font.pixelSize: 16
            font.weight: Font.Bold
            Layout.fillWidth: true
        }

        Divider { Layout.fillWidth: true }

        Text {
            text: "Selection Mode"
            color: Theme.textMuted
            font.pixelSize: 13
            font.weight: Font.Medium
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8
            onWidthChanged: { height = implicitHeight }
            Layout.preferredHeight: implicitHeight

            Repeater {
                model: root.selectionModes
                Button {
                    required property string modelData
                    text: modelData
                    
                    highlighted: (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) ? (EditorBackend.selectionTool.selectionMode === modelData) : false
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        color: parent.highlighted ? "#ffffff" : Theme.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: parent.contentItem.implicitWidth + 20
                        implicitHeight: 28
                        color: parent.highlighted ? Theme.accent : "#18181c"
                        radius: 4
                        border.color: parent.highlighted ? Theme.accent : "#303038"
                    }
                    
                    onClicked: {
                        if (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) {
                            EditorBackend.selectionTool.selectionMode = modelData
                        }
                    }
                }
            }
        }

        FeatureSwitch {
            text: "Feather Selection"
            checked: typeof EditorBackend !== "undefined" && EditorBackend.selectionTool ? EditorBackend.selectionTool.feather : false
            onToggled: (checked) => {
                if (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) {
                    EditorBackend.selectionTool.feather = checked
                }
            }
        }

        FeatureSlider {
            title: "Feather Radius"
            from: 1.0
            to: 50.0
            stepSize: 1.0
            value: typeof EditorBackend !== "undefined" && EditorBackend.selectionTool ? EditorBackend.selectionTool.featherRadius : 10.0
            valueSuffix: "px"
            onValueModified: (val) => {
                if (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) {
                    EditorBackend.selectionTool.featherRadius = val
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8
            onWidthChanged: { height = implicitHeight }
            Layout.preferredHeight: implicitHeight

            Button {
                text: "Select All"
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.weight: Font.Medium; color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: parent.contentItem.implicitWidth + 20; implicitHeight: 28; radius: 4
                    color: parent.down ? Theme.accent : "#18181c"; border.color: parent.down ? Theme.accent : "#303038"
                }
                onClicked: { if (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) EditorBackend.selectionTool.selectAll() }
            }
            Button {
                text: "Select None"
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.weight: Font.Medium; color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: parent.contentItem.implicitWidth + 20; implicitHeight: 28; radius: 4
                    color: parent.down ? Theme.accent : "#18181c"; border.color: parent.down ? Theme.accent : "#303038"
                }
                onClicked: { if (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) EditorBackend.selectionTool.selectNone() }
            }
            Button {
                text: "Invert Selection"
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.weight: Font.Medium; color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: parent.contentItem.implicitWidth + 20; implicitHeight: 28; radius: 4
                    color: parent.down ? Theme.accent : "#18181c"; border.color: parent.down ? Theme.accent : "#303038"
                }
                onClicked: { if (typeof EditorBackend !== "undefined" && EditorBackend.selectionTool) EditorBackend.selectionTool.selectInvert() }
            }
        }

        Divider { Layout.fillWidth: true }

        Flow {
            Layout.fillWidth: true
            spacing: 8
            onWidthChanged: { height = implicitHeight }
            Layout.preferredHeight: implicitHeight

            Button {
                text: "Add Horizontal Guide"
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.weight: Font.Medium; color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: parent.contentItem.implicitWidth + 20; implicitHeight: 28; radius: 4
                    color: parent.down ? Theme.accent : "#18181c"; border.color: parent.down ? Theme.accent : "#303038"
                }
                onClicked: { if (typeof EditorBackend !== "undefined" && EditorBackend.utilityTool) EditorBackend.utilityTool.addGuide("Horizontal", 540.0) }
            }
            Button {
                text: "Add Sample Point"
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.weight: Font.Medium; color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: parent.contentItem.implicitWidth + 20; implicitHeight: 28; radius: 4
                    color: parent.down ? Theme.accent : "#18181c"; border.color: parent.down ? Theme.accent : "#303038"
                }
                onClicked: { if (typeof EditorBackend !== "undefined" && EditorBackend.utilityTool) EditorBackend.utilityTool.addSamplePoint(Qt.point(960, 540)) }
            }
            Button {
                text: "Reset Transform"
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.weight: Font.Medium; color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: parent.contentItem.implicitWidth + 20; implicitHeight: 28; radius: 4
                    color: parent.down ? Theme.accent : "#18181c"; border.color: parent.down ? Theme.accent : "#303038"
                }
                onClicked: { if (typeof EditorBackend !== "undefined" && EditorBackend.transformTool) EditorBackend.transformTool.resetTransform() }
            }
        }
    }
}
