// qmllint disable
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import VideoStudioUI
import "../components"

Rectangle {
    id: chromaTabRoot
    color: Theme.background
    
    property var effectController: null
    property var chromaKeyData: effectController ? effectController.chromaKey : null
    
    ScrollView {
        id: rootScrollView
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        
        ColumnLayout {
            width: rootScrollView.width
            spacing: 0
            
            // --- Basic Chroma Key ---
            AccordionHeader {
                id: chromaHeader
                Layout.fillWidth: true
                title: "Chroma Key (Advanced)"
                expanded: true
                effectEnabled: chromaTabRoot.chromaKeyData ? chromaTabRoot.chromaKeyData.enabled : false
                
                onEffectEnabledToggled: (enabled) => {
                    if (chromaTabRoot.chromaKeyData)
                        chromaTabRoot.chromaKeyData.enabled = enabled
                }
                
                onResetClicked: {
                    varianceSlider.value = varianceSlider.defaultValue
                    softnessSlider.value = softnessSlider.defaultValue
                    if (chromaTabRoot.chromaKeyData) {
                        chromaTabRoot.chromaKeyData.enabled = false
                        chromaTabRoot.chromaKeyData.color = "#00FF00"
                        chromaTabRoot.chromaKeyData.variance = 0.25
                        chromaTabRoot.chromaKeyData.softness = 0.1
                        chromaTabRoot.chromaKeyData.spillSuppress = true
                    }
                }
            }
            
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: chromaContentLayout.implicitHeight + 16
                color: Theme.background
                visible: chromaHeader.expanded
                
                ColumnLayout {
                    id: chromaContentLayout
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Key Color"
                            color: Theme.text
                            font.pixelSize: 11
                            Layout.preferredWidth: 80
                        }
                        
                        Rectangle {
                            id: colorPreview
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 24
                            color: chromaTabRoot.chromaKeyData ? chromaTabRoot.chromaKeyData.color : "#00FF00"
                            border.color: Theme.divider
                            border.width: 1
                            radius: 4
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: colorDialog.open()
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                    
                    ColorDialog {
                        id: colorDialog
                        title: "Select Key Color"
                        selectedColor: chromaTabRoot.chromaKeyData ? chromaTabRoot.chromaKeyData.color : "#00FF00"
                        onAccepted: {
                            if (chromaTabRoot.chromaKeyData) chromaTabRoot.chromaKeyData.color = selectedColor
                        }
                    }
                    
                    EffectSlider {
                        id: varianceSlider
                        Layout.fillWidth: true
                        label: "Variance"
                        min: 0.0
                        max: 1.0
                        defaultValue: 0.25
                        value: chromaTabRoot.chromaKeyData ? chromaTabRoot.chromaKeyData.variance : 0.25
                        onValueChanged: { if(chromaTabRoot.chromaKeyData && chromaTabRoot.chromaKeyData.variance !== value) chromaTabRoot.chromaKeyData.variance = value }
                    }
                    
                    EffectSlider {
                        id: softnessSlider
                        Layout.fillWidth: true
                        label: "Softness"
                        min: 0.0
                        max: 1.0
                        defaultValue: 0.1
                        value: chromaTabRoot.chromaKeyData ? chromaTabRoot.chromaKeyData.softness : 0.1
                        onValueChanged: { if(chromaTabRoot.chromaKeyData && chromaTabRoot.chromaKeyData.softness !== value) chromaTabRoot.chromaKeyData.softness = value }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Spill Suppress"
                            color: Theme.text
                            font.pixelSize: 11
                            Layout.preferredWidth: 80
                        }
                        
                        Switch {
                            checked: chromaTabRoot.chromaKeyData ? chromaTabRoot.chromaKeyData.spillSuppress : true
                            onCheckedChanged: { if(chromaTabRoot.chromaKeyData && chromaTabRoot.chromaKeyData.spillSuppress !== checked) chromaTabRoot.chromaKeyData.spillSuppress = checked }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
        
        Connections {
            target: chromaTabRoot.chromaKeyData
            ignoreUnknownSignals: true
            function onEnabledChanged() { chromaHeader.effectEnabled = chromaTabRoot.chromaKeyData.enabled }
            function onColorChanged() { colorPreview.color = chromaTabRoot.chromaKeyData.color }
            function onVarianceChanged() { varianceSlider.value = chromaTabRoot.chromaKeyData.variance }
            function onSoftnessChanged() { softnessSlider.value = chromaTabRoot.chromaKeyData.softness }
            function onSpillSuppressChanged() { /* bindings handle this */ }
        }
    }
}
