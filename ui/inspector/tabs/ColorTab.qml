// qmllint disable unqualified
// qmllint disable missing-property
// qmllint disable
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI
import "../components"
pragma ComponentBehavior: Bound

ScrollView {
    id: rootScrollView
    property var effectController: null
    clip: true
    contentWidth: availableWidth
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        width: 12
    }
    ColumnLayout {
        width: rootScrollView.availableWidth
        spacing: 2
        
        // v Lumetri Color
        AccordionHeader { 
            id: lumetriHeader; title: "Lumetri Color"; expanded: true; tooltipText: "Advanced color correction and grading" 
            onResetClicked: {
                shadowsWheel.reset()
                midtonesWheel.reset()
                highlightsWheel.reset()
                if (effectController && effectController.color) {
                    effectController.color.shadowsColor = "#808080"
                    effectController.color.midtonesColor = "#808080"
                    effectController.color.highlightsColor = "#808080"
                }
            }
        }
        Rectangle {
            visible: lumetriHeader.expanded
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: "transparent"
            RowLayout {
                anchors.centerIn: parent
                spacing: 20
                ColorWheel {
                    id: shadowsWheel
                    onSelectedColorChanged: { if(effectController && effectController.color && effectController.color.shadowsColor !== selectedColor.toString()) effectController.color.shadowsColor = selectedColor.toString() }
                }
                ColorWheel {
                    id: midtonesWheel
                    onSelectedColorChanged: { if(effectController && effectController.color && effectController.color.midtonesColor !== selectedColor.toString()) effectController.color.midtonesColor = selectedColor.toString() }
                }
                ColorWheel {
                    id: highlightsWheel
                    onSelectedColorChanged: { if(effectController && effectController.color && effectController.color.highlightsColor !== selectedColor.toString()) effectController.color.highlightsColor = selectedColor.toString() }
                }
            }
        }
        
        // v Vignette
        AccordionHeader { 
            id: vignetteHeader; title: "Vignette"; expanded: false; tooltipText: "Darkens or lightens the edges of the image to draw attention to the center" 
            onResetClicked: {
                vignetteAmountSlider.value = vignetteAmountSlider.defaultValue
                vignetteFeatherSlider.value = vignetteFeatherSlider.defaultValue
                if (effectController && effectController.color) {
                    effectController.color.vignetteAmount = 0
                    effectController.color.vignetteFeather = 50
                }
            }
        }
        ColumnLayout {
            visible: vignetteHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { 
                id: vignetteAmountSlider; label: "Amount"; value: effectController ? effectController.color.vignetteAmount : 0; max: 100; defaultValue: 0; tooltipText: "Strength of the vignette effect" 
                onValueChanged: { if(effectController && effectController.color.vignetteAmount !== value) effectController.color.vignetteAmount = value }
            }
            EffectSlider { 
                id: vignetteFeatherSlider; label: "Feather"; value: effectController ? effectController.color.vignetteFeather : 50; max: 100; defaultValue: 50; tooltipText: "Softness of the vignette edges" 
                onValueChanged: { if(effectController && effectController.color.vignetteFeather !== value) effectController.color.vignetteFeather = value }
            }
        }
        
        // v Chroma Key
        AccordionHeader { 
            id: chromaKeyHeader; title: "Chroma Key (Ultra Key)"; expanded: false; effectEnabled: effectController ? effectController.chromaKey.enabled : false; tooltipText: "Removes a specific color from the background (e.g., green screen)" 
            onEffectEnabledToggled: (enabled) => { if(effectController && effectController.chromaKey && effectController.chromaKey.enabled !== enabled) effectController.chromaKey.enabled = enabled }
            onResetClicked: {
                toleranceSlider.value = toleranceSlider.defaultValue
                featherSlider.value = featherSlider.defaultValue
                if (effectController && effectController.chromaKey) {
                    effectController.chromaKey.enabled = false
                    effectController.chromaKey.color = "#00ff00"
                    effectController.chromaKey.variance = 0.45
                    effectController.chromaKey.softness = 0.10
                }
            }
        }
        ColumnLayout {
            visible: chromaKeyHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                
                HoverHandler { id: keyColorHover }
                ToolTip.visible: keyColorHover.hovered
                ToolTip.text: "Select the color to remove from the footage"
                ToolTip.delay: 500

                Text { text: "Key Color"; color: "#8a9ba3"; font.pixelSize: 11; Layout.preferredWidth: 60 }
                Rectangle {
                    id: keyColorRect
                    width: 40; height: 24
                    color: effectController ? effectController.chromaKey.color : "#00ff00" // Default green screen
                    border.color: Theme.divider
                    radius: 2
                }
                Rectangle {
                    width: 24; height: 24
                    color: Theme.surfaceRaised
                    border.color: Theme.divider
                    radius: 2
                    Text { anchors.centerIn: parent; text: "🖌"; color: Theme.text; font.pixelSize: 12 }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { if(effectController && effectController.chromaKey) effectController.chromaKey.color = (effectController.chromaKey.color === "#00ff00" ? "#0000ff" : "#00ff00") }
                    }
                }
                Item { Layout.fillWidth: true }
            }
            
            EffectSlider { 
                id: toleranceSlider; label: "Tolerance"; value: effectController ? effectController.chromaKey.variance * 100 : 45; max: 100; defaultValue: 45; tooltipText: "How closely a color must match the Key Color to be removed" 
                onValueChanged: { if(effectController && effectController.chromaKey && Math.abs(effectController.chromaKey.variance * 100 - value) > 0.1) effectController.chromaKey.variance = value / 100.0 }
            }
            EffectSlider { 
                id: featherSlider; label: "Edge Feather"; value: effectController ? effectController.chromaKey.softness * 100 : 10; max: 100; defaultValue: 10; tooltipText: "Softens the edges of the keyed area" 
                onValueChanged: { if(effectController && effectController.chromaKey && Math.abs(effectController.chromaKey.softness * 100 - value) > 0.1) effectController.chromaKey.softness = value / 100.0 }
            }
            
            Connections {
                target: effectController ? effectController.color : null
                function onVignetteAmountChanged() { vignetteAmountSlider.value = effectController.color.vignetteAmount }
                function onVignetteFeatherChanged() { vignetteFeatherSlider.value = effectController.color.vignetteFeather }
                function onShadowsColorChanged() { if (effectController && effectController.color && effectController.color.shadowsColor === "#808080") shadowsWheel.reset() }
                function onMidtonesColorChanged() { if (effectController && effectController.color && effectController.color.midtonesColor === "#808080") midtonesWheel.reset() }
                function onHighlightsColorChanged() { if (effectController && effectController.color && effectController.color.highlightsColor === "#808080") highlightsWheel.reset() }
            }
            Connections {
                target: effectController ? effectController.chromaKey : null
                function onVarianceChanged() { toleranceSlider.value = effectController.chromaKey.variance * 100 }
                function onSoftnessChanged() { featherSlider.value = effectController.chromaKey.softness * 100 }
                function onColorChanged() { keyColorRect.color = effectController.chromaKey.color }
                function onEnabledChanged() { chromaKeyHeader.effectEnabled = effectController.chromaKey.enabled }
            }
        }
        
        Item { Layout.fillHeight: true }
    }
}
