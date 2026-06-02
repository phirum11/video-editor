// qmllint disable unqualified
// qmllint disable missing-property
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI
pragma ComponentBehavior: Bound

ScrollView {
    id: rootScrollView
    clip: true
    contentWidth: availableWidth
    ColumnLayout {
        width: rootScrollView.availableWidth
        spacing: 2
        
        // v Audio Visualizer
        AccordionHeader { id: visualizerHeader; title: "Audio Visualizer"; expanded: true; tooltipText: "Generates visual elements based on the audio track frequencies" }
        ColumnLayout {
            visible: visualizerHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { label: "Bands"; value: 32; max: 128; tooltipText: "Number of frequency bands to visualize" }
            EffectSlider { label: "Smoothing"; value: 50; max: 100; suffix: "%"; tooltipText: "How smoothly the visualizer reacts to audio changes" }
        }
        
        // v Drop Shadow
        AccordionHeader { id: dropShadowHeader; title: "Drop Shadow"; expanded: false; tooltipText: "Adds a shadow behind the clip or text to create depth" }
        ColumnLayout {
            visible: dropShadowHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { label: "Distance"; value: 10; max: 100; tooltipText: "Distance between the shadow and the object" }
            EffectSlider { label: "Opacity"; value: 50; max: 100; suffix: "%"; tooltipText: "Transparency of the drop shadow" }
            EffectSlider { label: "Softness"; value: 20; max: 100; tooltipText: "How blurry or sharp the edges of the shadow are" }
        }
        
        // v Glitch Distortion
        AccordionHeader { id: glitchHeader; title: "Glitch / Distortion"; expanded: false; tooltipText: "Adds digital glitch artifacts and distortions" }
        ColumnLayout {
            visible: glitchHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { label: "Wave Warp"; value: 0; max: 100; tooltipText: "Wavy distortion applied across the image" }
            EffectSlider { label: "RGB Split"; value: 0; max: 100; tooltipText: "Separates the red, green, and blue color channels" }
            EffectSlider { label: "Lens Distortion"; value: 0; max: 100; tooltipText: "Simulates curved distortion from wide-angle lenses (e.g., fisheye)" }
        }
        
        Item { Layout.fillHeight: true }
    }
}
