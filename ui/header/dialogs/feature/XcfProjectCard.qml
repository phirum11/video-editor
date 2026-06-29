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

    property string projectTitle: (typeof EditorBackend !== "undefined" && EditorBackend.imageModel) ? EditorBackend.imageModel.title : "Untitled Project"
    property int projectWidth: (typeof EditorBackend !== "undefined" && EditorBackend.imageModel) ? EditorBackend.imageModel.width : 1920
    property int projectHeight: (typeof EditorBackend !== "undefined" && EditorBackend.imageModel) ? EditorBackend.imageModel.height : 1080
    property string activeStatus: "Ready"

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "XCF Project & Undo Stack"
                color: Theme.text
                font.pixelSize: 16
                font.weight: Font.Bold
                Layout.fillWidth: true
            }
            Text {
                text: root.activeStatus
                color: "#38bdf8"
                font.pixelSize: 13
                font.weight: Font.Medium
            }
        }

        Divider { Layout.fillWidth: true }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: infoLayout.implicitHeight + 20
            color: "#18181c"
            radius: 8
            border.color: "#303038"
            border.width: 1

            ColumnLayout {
                id: infoLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Text {
                    text: "Active Project: " + root.projectTitle
                    color: Theme.text
                    font.pixelSize: 14
                    font.weight: Font.Bold
                }
                Text {
                    text: "Resolution: " + root.projectWidth + " x " + root.projectHeight
                    color: Theme.textMuted
                    font.pixelSize: 13
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "Parse XCF Project"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.xcfParser) {
                        root.activeStatus = "Parsing XCF..."
                        EditorBackend.xcfParser.parseXcfProject("default_studio_project.xcf")
                    }
                }
            }

            Button {
                text: "Save XCF Project"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.xcfParser) {
                        root.activeStatus = "Saving XCF..."
                        EditorBackend.xcfParser.saveXcfProject("default_studio_project.xcf", {"title": root.projectTitle}, [])
                    }
                }
            }
        }

        Divider { Layout.fillWidth: true }

        Text {
            text: "Undo Stack Actions"
            color: Theme.textMuted
            font.pixelSize: 13
            font.weight: Font.Medium
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "Undo"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.undoStack) {
                        EditorBackend.undoStack.undo()
                        root.activeStatus = "Undo Performed"
                    }
                }
            }
            Button {
                text: "Redo"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.undoStack) {
                        EditorBackend.undoStack.redo()
                        root.activeStatus = "Redo Performed"
                    }
                }
            }
            Button {
                text: "Clear History"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.undoStack) {
                        EditorBackend.undoStack.clear()
                        root.activeStatus = "History Cleared"
                    }
                }
            }
        }
    }

    Connections {
        target: typeof EditorBackend !== "undefined" ? EditorBackend.xcfParser : null
        function onXcfParsed(success, metadata, layerList) {
            root.activeStatus = success ? "XCF Parsed Successfully" : "XCF Parse Failed"
            if (success && metadata.width) {
                root.projectWidth = metadata.width
                root.projectHeight = metadata.height
            }
        }
        function onXcfSaved(success) {
            root.activeStatus = success ? "XCF Saved Successfully" : "XCF Save Failed"
        }
    }
}
