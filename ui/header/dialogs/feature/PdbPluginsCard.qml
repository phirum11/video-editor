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

    property string selectedCategory: "Brush"
    property string selectedProcedure: "gimp-brushes-get-list"
    property var procedureDetails: (typeof EditorBackend !== "undefined" && EditorBackend.pdbProcedureManager) ? EditorBackend.pdbProcedureManager.getProcedureDetails(selectedProcedure) : {}
    property string lastIpcStatus: (typeof EditorBackend !== "undefined" && EditorBackend.pluginIpc && EditorBackend.pluginIpc.isIpcConnected) ? "Connected" : "Disconnected"

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "PDB Procedures & Plug-in IPC"
                color: Theme.text
                font.pixelSize: 16
                font.weight: Font.Bold
                Layout.fillWidth: true
            }
            Rectangle {
                width: ipcText.width + 20
                height: 26
                radius: 13
                color: "#18181c"
                border.color: root.lastIpcStatus === "Connected" ? "#10b981" : "#454550"
                border.width: 1
                Text {
                    id: ipcText
                    anchors.centerIn: parent
                    text: "IPC: " + root.lastIpcStatus
                    color: root.lastIpcStatus === "Connected" ? "#10b981" : Theme.textMuted
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }
            }
        }

        Divider { Layout.fillWidth: true }

        Text {
            text: "PDB Categories"
            color: Theme.textMuted
            font.pixelSize: 13
            font.weight: Font.Medium
        }

        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: (typeof EditorBackend !== "undefined" && EditorBackend.pdbProcedureManager) ? EditorBackend.pdbProcedureManager.availableCategories : []
                Button {
                    required property string modelData
                    text: modelData
                    highlighted: root.selectedCategory === modelData
                    onClicked: {
                        root.selectedCategory = modelData
                        var procs = EditorBackend.pdbProcedureManager.getProceduresInCategory(modelData)
                        if (procs.length > 0) {
                            root.selectedProcedure = procs[0]
                            root.procedureDetails = EditorBackend.pdbProcedureManager.getProcedureDetails(root.selectedProcedure)
                        }
                    }
                }
            }
        }

        Divider { Layout.fillWidth: true }

        Text {
            text: "Procedures in " + root.selectedCategory
            color: Theme.textMuted
            font.pixelSize: 13
            font.weight: Font.Medium
        }

        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: (typeof EditorBackend !== "undefined" && EditorBackend.pdbProcedureManager) ? EditorBackend.pdbProcedureManager.getProceduresInCategory(root.selectedCategory) : []
                Button {
                    required property string modelData
                    text: modelData
                    highlighted: root.selectedProcedure === modelData
                    onClicked: {
                        root.selectedProcedure = modelData
                        root.procedureDetails = EditorBackend.pdbProcedureManager.getProcedureDetails(root.selectedProcedure)
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: detailsColumn.implicitHeight + 20
            color: "#18181c"
            radius: 8
            border.color: "#303038"
            border.width: 1

            ColumnLayout {
                id: detailsColumn
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Text {
                    text: root.procedureDetails.name ? root.procedureDetails.name : "Select a procedure"
                    color: "#38bdf8"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    Layout.fillWidth: true
                }
                Text {
                    text: root.procedureDetails.blurb ? root.procedureDetails.blurb : ""
                    color: Theme.text
                    font.pixelSize: 13
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Text {
                    text: root.procedureDetails.help ? root.procedureDetails.help : ""
                    color: Theme.textMuted
                    font.pixelSize: 12
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "Run Procedure"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.pdbProcedureManager) {
                        EditorBackend.pdbProcedureManager.runProcedure(root.selectedProcedure, {})
                    }
                }
            }

            Button {
                text: root.lastIpcStatus === "Connected" ? "Disconnect IPC" : "Init Plugin IPC"
                Layout.fillWidth: true
                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.pluginIpc) {
                        if (EditorBackend.pluginIpc.isIpcConnected) {
                            EditorBackend.pluginIpc.disconnectIpc()
                        } else {
                            EditorBackend.pluginIpc.initializeSharedMemory("GimpVideoStudioIPC", 1024 * 1024)
                        }
                        root.lastIpcStatus = EditorBackend.pluginIpc.isIpcConnected ? "Connected" : "Disconnected"
                    }
                }
            }
        }
    }

    Connections {
        target: typeof EditorBackend !== "undefined" ? EditorBackend.pluginIpc : null
        function onIpcConnectionChanged() {
            root.lastIpcStatus = EditorBackend.pluginIpc.isIpcConnected ? "Connected" : "Disconnected"
        }
    }
}
