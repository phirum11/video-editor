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
    property var effectController: null
    clip: true
    contentWidth: availableWidth
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        width: 12
    }
    
    property real lastNonZeroRadius: 25

    ColumnLayout {
        width: rootScrollView.availableWidth
        spacing: 2
        
        // v Gaussian Blur
        AccordionHeader { 
            id: blurHeader
            title: "Gaussian Blur"
            expanded: true
            effectEnabled: effectController ? effectController.blur.radius > 0 : false
            tooltipText: "Applies a smooth blur to the selected region"
            onEffectEnabledToggled: function(enabled) {
                if (!effectController)
                    return
                effectController.blur.radius = enabled ? Math.max(1, rootScrollView.lastNonZeroRadius) : 0
            }
            onResetClicked: {
                blurSlider.value = blurSlider.defaultValue
                if (effectController) {
                    effectController.blur.updateRegion(0.25, 0.25, 0.5, 0.5)
                }
            }
        }
        ColumnLayout {
            visible: blurHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 12
            
            EffectSlider { 
                id: blurSlider; label: "Blurriness"; defaultValue: 0; max: 100; tooltipText: "Intensity of the blur effect"
                value: effectController ? effectController.blur.radius : 0
                onValueChanged: {
                    if (value > 0)
                        rootScrollView.lastNonZeroRadius = value
                    if(effectController && effectController.blur.radius !== value)
                        effectController.blur.radius = value
                }
            }
        }
        
        Connections {
            target: effectController ? effectController.blur : null
            function onRadiusChanged() {
                blurSlider.value = effectController.blur.radius
                blurHeader.effectEnabled = effectController.blur.radius > 0
                if (effectController.blur.radius > 0)
                    rootScrollView.lastNonZeroRadius = effectController.blur.radius
            }
        }
        
        Item { Layout.fillHeight: true }
    }
}

