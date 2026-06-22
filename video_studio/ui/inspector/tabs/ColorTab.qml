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
        
        // v Lumetri Color
        AccordionHeader { id: lumetriHeader; title: "Lumetri Color"; expanded: true; tooltipText: "Advanced color correction and grading" }
        Rectangle {
            visible: lumetriHeader.expanded
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: "transparent"
            RowLayout {
                anchors.centerIn: parent
                spacing: 20
                ColorWheel {}
                ColorWheel {}
                ColorWheel {}
            }
        }
        
        // v Vignette
        AccordionHeader { id: vignetteHeader; title: "Vignette"; expanded: false; tooltipText: "Darkens or lightens the edges of the image to draw attention to the center" }
        ColumnLayout {
            visible: vignetteHeader.expanded
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 8
            EffectSlider { label: "Amount"; value: 0; max: 100; tooltipText: "Strength of the vignette effect" }
            EffectSlider { label: "Feather"; value: 50; max: 100; tooltipText: "Softness of the vignette edges" }
        }
        
        // v Chroma Key
        AccordionHeader { id: chromaKeyHeader; title: "Chroma Key (Ultra Key)"; expanded: false; tooltipText: "Removes a specific color from the background (e.g., green screen)" }
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
                    width: 40; height: 24
                    color: "#00ff00" // Default green screen
                    border.color: Theme.divider
                    radius: 2
                }
                Rectangle {
                    width: 24; height: 24
                    color: Theme.surfaceRaised
                    border.color: Theme.divider
                    radius: 2
                    Text { anchors.centerIn: parent; text: "🖌"; color: Theme.text; font.pixelSize: 12 }
                }
                Item { Layout.fillWidth: true }
            }
            
            EffectSlider { label: "Tolerance"; value: 45; max: 100; tooltipText: "How closely a color must match the Key Color to be removed" }
            EffectSlider { label: "Edge Feather"; value: 10; max: 100; tooltipText: "Softens the edges of the keyed area" }
        }
        
        Item { Layout.fillHeight: true }
    }
}

