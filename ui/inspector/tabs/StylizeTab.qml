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
        
        // v Audio Visualizer
        AccordionHeader { 
            id: visualizerHeader; title: "Audio Visualizer"; expanded: false; effectEnabled: effectController ? effectController.stylize.audioVisualizerEnabled : false; tooltipText: "Generates visual elements based on the audio track frequencies" 
            onEffectEnabledToggled: (enabled) => { if(effectController && effectController.stylize && effectController.stylize.audioVisualizerEnabled !== enabled) effectController.stylize.audioVisualizerEnabled = enabled }
            onResetClicked: {
                bandsSlider.value = bandsSlider.defaultValue
                smoothingSlider.value = smoothingSlider.defaultValue
                if (effectController && effectController.stylize) {
                    effectController.stylize.audioVisualizerEnabled = false
                    effectController.stylize.audioVisualizerBands = 32
                    effectController.stylize.audioVisualizerSmoothing = 50
                }
            }
        }
        ColumnLayout {
            visible: visualizerHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { 
                id: bandsSlider; label: "Bands"; value: effectController ? effectController.stylize.audioVisualizerBands : 32; max: 128; defaultValue: 32; tooltipText: "Number of frequency bands to visualize" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.audioVisualizerBands !== Math.round(value)) effectController.stylize.audioVisualizerBands = Math.round(value) }
            }
            EffectSlider { 
                id: smoothingSlider; label: "Smoothing"; value: effectController ? effectController.stylize.audioVisualizerSmoothing : 50; max: 100; defaultValue: 50; suffix: "%"; tooltipText: "How smoothly the visualizer reacts to audio changes" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.audioVisualizerSmoothing !== value) effectController.stylize.audioVisualizerSmoothing = value }
            }
        }
        
        // v Drop Shadow
        AccordionHeader { 
            id: dropShadowHeader; title: "Drop Shadow"; expanded: false; effectEnabled: effectController ? effectController.stylize.dropShadowEnabled : false; tooltipText: "Adds a shadow behind the clip or text to create depth" 
            onEffectEnabledToggled: (enabled) => { if(effectController && effectController.stylize && effectController.stylize.dropShadowEnabled !== enabled) effectController.stylize.dropShadowEnabled = enabled }
            onResetClicked: {
                distSlider.value = distSlider.defaultValue
                dsOpacitySlider.value = dsOpacitySlider.defaultValue
                dsSoftnessSlider.value = dsSoftnessSlider.defaultValue
                if (effectController && effectController.stylize) {
                    effectController.stylize.dropShadowEnabled = false
                    effectController.stylize.dropShadowDistance = 10
                    effectController.stylize.dropShadowOpacity = 50
                    effectController.stylize.dropShadowSoftness = 20
                }
            }
        }
        ColumnLayout {
            visible: dropShadowHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { 
                id: distSlider; label: "Distance"; value: effectController ? effectController.stylize.dropShadowDistance : 10; max: 100; defaultValue: 10; tooltipText: "Distance between the shadow and the object" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.dropShadowDistance !== value) effectController.stylize.dropShadowDistance = value }
            }
            EffectSlider { 
                id: dsOpacitySlider; label: "Opacity"; value: effectController ? effectController.stylize.dropShadowOpacity : 50; max: 100; defaultValue: 50; suffix: "%"; tooltipText: "Transparency of the drop shadow" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.dropShadowOpacity !== value) effectController.stylize.dropShadowOpacity = value }
            }
            EffectSlider { 
                id: dsSoftnessSlider; label: "Softness"; value: effectController ? effectController.stylize.dropShadowSoftness : 20; max: 100; defaultValue: 20; tooltipText: "How blurry or sharp the edges of the shadow are" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.dropShadowSoftness !== value) effectController.stylize.dropShadowSoftness = value }
            }
        }
        
        // v Glitch Distortion
        AccordionHeader { 
            id: glitchHeader; title: "Glitch / Distortion"; expanded: false; effectEnabled: effectController ? effectController.stylize.glitchEnabled : false; tooltipText: "Adds digital glitch artifacts and distortions" 
            onEffectEnabledToggled: (enabled) => { if(effectController && effectController.stylize && effectController.stylize.glitchEnabled !== enabled) effectController.stylize.glitchEnabled = enabled }
            onResetClicked: {
                warpSlider.value = warpSlider.defaultValue
                rgbSplitSlider.value = rgbSplitSlider.defaultValue
                lensDistSlider.value = lensDistSlider.defaultValue
                if (effectController && effectController.stylize) {
                    effectController.stylize.glitchEnabled = false
                    effectController.stylize.glitchWaveWarp = 0
                    effectController.stylize.glitchRGBSplit = 0
                    effectController.stylize.glitchLensDistortion = 0
                }
            }
        }
        ColumnLayout {
            visible: glitchHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { 
                id: warpSlider; label: "Wave Warp"; value: effectController ? effectController.stylize.glitchWaveWarp : 0; max: 100; defaultValue: 0; tooltipText: "Wavy distortion applied across the image" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.glitchWaveWarp !== value) effectController.stylize.glitchWaveWarp = value }
            }
            EffectSlider { 
                id: rgbSplitSlider; label: "RGB Split"; value: effectController ? effectController.stylize.glitchRGBSplit : 0; max: 100; defaultValue: 0; tooltipText: "Separates the red, green, and blue color channels" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.glitchRGBSplit !== value) effectController.stylize.glitchRGBSplit = value }
            }
            EffectSlider { 
                id: lensDistSlider; label: "Lens Distortion"; value: effectController ? effectController.stylize.glitchLensDistortion : 0; max: 100; defaultValue: 0; tooltipText: "Simulates curved distortion from wide-angle lenses (e.g., fisheye)" 
                onValueChanged: { if(effectController && effectController.stylize && effectController.stylize.glitchLensDistortion !== value) effectController.stylize.glitchLensDistortion = value }
            }
            
            Connections {
                target: effectController ? effectController.stylize : null
                function onAudioVisualizerEnabledChanged() { visualizerHeader.effectEnabled = effectController.stylize.audioVisualizerEnabled }
                function onAudioVisualizerBandsChanged() { bandsSlider.value = effectController.stylize.audioVisualizerBands }
                function onAudioVisualizerSmoothingChanged() { smoothingSlider.value = effectController.stylize.audioVisualizerSmoothing }
                function onDropShadowEnabledChanged() { dropShadowHeader.effectEnabled = effectController.stylize.dropShadowEnabled }
                function onDropShadowDistanceChanged() { distSlider.value = effectController.stylize.dropShadowDistance }
                function onDropShadowOpacityChanged() { dsOpacitySlider.value = effectController.stylize.dropShadowOpacity }
                function onDropShadowSoftnessChanged() { dsSoftnessSlider.value = effectController.stylize.dropShadowSoftness }
                function onGlitchEnabledChanged() { glitchHeader.effectEnabled = effectController.stylize.glitchEnabled }
                function onGlitchWaveWarpChanged() { warpSlider.value = effectController.stylize.glitchWaveWarp }
                function onGlitchRGBSplitChanged() { rgbSplitSlider.value = effectController.stylize.glitchRGBSplit }
                function onGlitchLensDistortionChanged() { lensDistSlider.value = effectController.stylize.glitchLensDistortion }
            }
        }
        
        Item { Layout.fillHeight: true }
    }
}
