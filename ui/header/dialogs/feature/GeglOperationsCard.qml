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

    property string lastOperationStatus: "Ready"

    Column {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        RowLayout {
            width: parent.width
            Text {
                text: "GEGL Operations Engine"
                color: Theme.text
                font.pixelSize: 16
                font.weight: Font.Bold
                Layout.fillWidth: true
            }
            Rectangle {
                width: statusText.width + 20
                height: 26
                radius: 13
                color: "#18181c"
                border.color: "#38bdf8"
                border.width: 1
                Text {
                    id: statusText
                    anchors.centerIn: parent
                    text: root.lastOperationStatus
                    color: "#38bdf8"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }
            }
        }

        Divider { width: parent.width }

        FeatureSlider {
            width: parent.width
            title: "Brightness"
            from: -1.0
            to: 1.0
            stepSize: 0.05
            value: 0.0
            valueSuffix: ""
            onValueModified: (val) => {
                if (typeof EditorBackend !== "undefined" && EditorBackend.operationEngine) {
                    root.lastOperationStatus = "Running Brightness..."
                    // Pass empty dummy image for QML binding demonstration
                    EditorBackend.operationEngine.runAsyncOperation("gimp:brightness-contrast", EditorBackend.imageModel.activeLayer ? EditorBackend.imageModel.activeLayer.buffer : null, {"brightness": val}, "ColorAdjustment")
                }
            }
        }

        FeatureSlider {
            width: parent.width
            title: "Contrast"
            from: 0.0
            to: 2.0
            stepSize: 0.05
            value: 1.0
            valueSuffix: "x"
            onValueModified: (val) => {
                if (typeof EditorBackend !== "undefined" && EditorBackend.operationEngine) {
                    root.lastOperationStatus = "Running Contrast..."
                    EditorBackend.operationEngine.runAsyncOperation("gimp:brightness-contrast", EditorBackend.imageModel.activeLayer ? EditorBackend.imageModel.activeLayer.buffer : null, {"contrast": val}, "ColorAdjustment")
                }
            }
        }

        FeatureSlider {
            width: parent.width
            title: "Threshold Level"
            from: 0
            to: 255
            stepSize: 1
            value: 128
            valueSuffix: ""
            onValueModified: (val) => {
                if (typeof EditorBackend !== "undefined" && EditorBackend.operationEngine) {
                    root.lastOperationStatus = "Running Threshold..."
                    EditorBackend.operationEngine.runAsyncOperation("gimp:threshold", EditorBackend.imageModel.activeLayer ? EditorBackend.imageModel.activeLayer.buffer : null, {"threshold": val}, "ColorAdjustment")
                }
            }
        }

        Flow {
            width: parent.width
            spacing: 12

            Button {
                text: "Flood Fill"
                width: (parent.width - 12) / 2
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitHeight: 28
                    color: parent.down ? Theme.accent : "#18181c"
                    radius: 4
                    border.color: parent.down ? Theme.accent : "#303038"
                }

                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.operationEngine) {
                        root.lastOperationStatus = "Flood Filling..."
                        EditorBackend.operationEngine.runAsyncOperation("gimp:flood-fill", EditorBackend.imageModel.activeLayer ? EditorBackend.imageModel.activeLayer.buffer : null, {"tolerance": 15}, "GenerativeFill")
                    }
                }
            }

            Button {
                text: "Color Balance"
                width: (parent.width - 12) / 2
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: parent.down ? "#ffffff" : Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitHeight: 28
                    color: parent.down ? Theme.accent : "#18181c"
                    radius: 4
                    border.color: parent.down ? Theme.accent : "#303038"
                }

                onClicked: {
                    if (typeof EditorBackend !== "undefined" && EditorBackend.operationEngine) {
                        root.lastOperationStatus = "Balancing Colors..."
                        EditorBackend.operationEngine.runAsyncOperation("gimp:color-balance", EditorBackend.imageModel.activeLayer ? EditorBackend.imageModel.activeLayer.buffer : null, {}, "ColorAdjustment")
                    }
                }
            }
        }
    }

    Connections {
        target: typeof EditorBackend !== "undefined" ? EditorBackend.operationEngine : null
        function onOperationCompleted(operationName, resultImage, success) {
            root.lastOperationStatus = success ? (operationName + " Done") : "Failed"
        }
    }
}
