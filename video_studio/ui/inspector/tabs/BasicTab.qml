// qmllint disable unqualified
// qmllint disable missing-property
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI
import "../components"
pragma ComponentBehavior: Bound

ScrollView {
    id: rootScrollView
    clip: true
    contentWidth: availableWidth
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        width: 12
    }
    
    ColumnLayout {
        width: rootScrollView.availableWidth
        spacing: 2
        
        // v Transform
        AccordionHeader { 
            id: transformHeader
            title: "Transform"
            tooltipText: "Adjust the size, position, and rotation of the clip"
            expanded: true
            onResetClicked: {
                scaleSlider.value = scaleSlider.defaultValue
                posXSlider.value = posXSlider.defaultValue
                posYSlider.value = posYSlider.defaultValue
                rotSlider.value = rotSlider.defaultValue
                anchorXSlider.value = anchorXSlider.defaultValue
                anchorYSlider.value = anchorYSlider.defaultValue
            }
        }
        
        ColumnLayout {
            visible: transformHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            
            EffectSlider { 
                id: scaleSlider; label: "Scale"; defaultValue: 100; max: 200; suffix: "%"; tooltipText: "Scales the clip proportionally"
                value: effectController ? effectController.transform.scale : 100
                onValueChanged: { if(effectController && effectController.transform.scale !== value) effectController.transform.scale = value }
            }
            
            EffectSlider { 
                id: posXSlider; label: "Position X"; defaultValue: 960; max: 3840; tooltipText: "Horizontal position of the clip"
                value: effectController ? effectController.transform.posX : 960
                onValueChanged: { if(effectController && effectController.transform.posX !== value) effectController.transform.posX = value }
            }
            EffectSlider { 
                id: posYSlider; label: "Position Y"; defaultValue: 540; max: 2160; tooltipText: "Vertical position of the clip"
                value: effectController ? effectController.transform.posY : 540
                onValueChanged: { if(effectController && effectController.transform.posY !== value) effectController.transform.posY = value }
            }
            
            EffectSlider { 
                id: rotSlider; label: "Rotation"; defaultValue: 0; max: 360; suffix: "°"; tooltipText: "Rotates the clip"
                value: effectController ? effectController.transform.rotation : 0
                onValueChanged: { if(effectController && effectController.transform.rotation !== value) effectController.transform.rotation = value }
            }
            
            EffectSlider { 
                id: anchorXSlider; label: "Anchor X"; defaultValue: 960; max: 3840; tooltipText: "Horizontal anchor point around which transformations occur"
                value: effectController ? effectController.transform.anchorX : 960
                onValueChanged: { if(effectController && effectController.transform.anchorX !== value) effectController.transform.anchorX = value }
            }
            EffectSlider { 
                id: anchorYSlider; label: "Anchor Y"; defaultValue: 540; max: 2160; tooltipText: "Vertical anchor point around which transformations occur"
                value: effectController ? effectController.transform.anchorY : 540
                onValueChanged: { if(effectController && effectController.transform.anchorY !== value) effectController.transform.anchorY = value }
            }
        }
        
        // v Opacity
        AccordionHeader { 
            id: opacityHeader
            title: "Opacity"
            tooltipText: "Control the transparency and blend mode of the clip"
            expanded: true
            onResetClicked: {
                opacitySlider.value = opacitySlider.defaultValue
                blendModeCombo.currentIndex = 0
            }
        }
        
        ColumnLayout {
            visible: opacityHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { 
                id: opacitySlider; label: "Opacity"; defaultValue: 100; max: 100; suffix: "%"; tooltipText: "Controls how opaque the clip is"
                value: effectController ? effectController.transform.opacity : 100
                onValueChanged: { if(effectController && effectController.transform.opacity !== value) effectController.transform.opacity = value }
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                
                HoverHandler { id: blendHover }
                ToolTip.visible: blendHover.hovered
                ToolTip.text: "Determines how the clip blends with the layers beneath it"
                ToolTip.delay: 500

                Text { text: "Blend Mode"; color: "#d0d0d0"; font.pixelSize: 12; Layout.preferredWidth: 80 }
                ComboBox {
                    id: blendModeCombo
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                    model: ["Normal", "Dissolve", "Darken", "Multiply", "Color Burn", "Linear Burn", "Screen", "Color Dodge", "Overlay", "Soft Light", "Hard Light"]
                    
                    background: Rectangle {
                        color: Theme.surfaceRaised
                        border.color: Theme.divider
                        radius: 2
                    }
                    contentItem: Text {
                        text: blendModeCombo.currentText
                        color: Theme.text
                        font.pixelSize: 11
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 8
                    }

                    delegate: ItemDelegate {
                        id: delegateItem
                        required property string modelData
                        width: blendModeCombo.width
                        height: 24
                        contentItem: Text {
                            text: delegateItem.modelData
                            color: delegateItem.highlighted ? Theme.text : "#dce4e7"
                            font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 4
                        }
                        background: Rectangle {
                            color: delegateItem.highlighted ? "#2792c3" : Theme.background
                        }
                    }

                    popup: Popup {
                        y: blendModeCombo.height - 1
                        width: blendModeCombo.width
                        implicitHeight: Math.min(contentItem.implicitHeight, 150)
                        padding: 1

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: blendModeCombo.popup.visible ? blendModeCombo.delegateModel : null
                            currentIndex: blendModeCombo.highlightedIndex
                            
                            ScrollBar.vertical: ScrollBar {
                                width: 8
                                policy: ScrollBar.AlwaysOn
                            }
                        }

                        background: Rectangle {
                            color: Theme.background
                            border.color: Theme.divider
                            border.width: 1
                            radius: 2
                        }
                    }
                }
            }
            
            Connections {
                target: effectController ? effectController.transform : null
                function onScaleChanged() { scaleSlider.value = effectController.transform.scale }
                function onPosXChanged() { posXSlider.value = effectController.transform.posX }
                function onPosYChanged() { posYSlider.value = effectController.transform.posY }
                function onRotationChanged() { rotSlider.value = effectController.transform.rotation }
                function onAnchorXChanged() { anchorXSlider.value = effectController.transform.anchorX }
                function onAnchorYChanged() { anchorYSlider.value = effectController.transform.anchorY }
                function onOpacityChanged() { opacitySlider.value = effectController.transform.opacity }
            }
        }
        Item { Layout.fillHeight: true }
    }
}

