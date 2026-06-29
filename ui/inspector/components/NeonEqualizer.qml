// qmllint disable
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI

Rectangle {
    id: root
    Layout.fillWidth: true
    Layout.margins: 10
    implicitHeight: eqLayout.implicitHeight + 28
    color: "#1c1c1e" // Darker sleek background
    radius: 8
    border.color: "#2c2c2e"
    border.width: 1

    property var effectController: null
    
    // Signal to trigger canvas repaint
    signal eqChanged()

    ColumnLayout {
        id: eqLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12
        
        // Header row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            CheckBox {
                id: eqCheck
                checked: true
                text: "10-Band Equalizer"
                font.pixelSize: 14
                font.weight: Font.Medium
                
                indicator: Rectangle {
                    implicitWidth: 18
                    implicitHeight: 18
                    y: parent.height / 2 - height / 2
                    radius: 4
                    color: eqCheck.checked ? "#38bdf8" : "transparent"
                    border.color: eqCheck.checked ? "#38bdf8" : "#555"
                    Image {
                        anchors.centerIn: parent
                        width: 12
                        height: 12
                        source: "qrc:/VideoStudioUI/assets/check.svg"
                        visible: eqCheck.checked
                    }
                }
                contentItem: Text {
                    text: eqCheck.text
                    color: Theme.text
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: eqCheck.indicator.width + eqCheck.spacing
                }
            }
            
            Item { Layout.fillWidth: true }
            
            Image {
                source: "qrc:/VideoStudioUI/assets/undo.svg"
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                opacity: undoArea.containsMouse ? 1.0 : 0.5
                MouseArea {
                    id: undoArea
                    anchors.fill: parent
                    anchors.margins: -5
                    hoverEnabled: true
                    onClicked: {
                        for (let i = 0; i < eqRepeater.count; ++i) {
                            let col = eqRepeater.itemAt(i)
                            if (col) col.resetSlider()
                        }
                    }
                }
            }
        }
        
        // Equalizer Body
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 280
            visible: eqCheck.checked
            color: "transparent"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // Top section: Canvas Frequency Curve
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    color: "#111111"
                    radius: 6
                    border.color: "#222"
                    clip: true
                    
                    // Background horizontal grid lines
                    Rectangle { x: 0; y: 15; width: parent.width; height: 1; color: "#333" }
                    Rectangle { x: 0; y: 40; width: parent.width; height: 1; color: "#555" } // zero line
                    Rectangle { x: 0; y: 65; width: parent.width; height: 1; color: "#333" }
                    
                    Text { x: 4; y: 7; text: "+15dB"; color: "#888"; font.pixelSize: 10 }
                    Text { x: 4; y: 32; text: "zero"; color: "#aaa"; font.pixelSize: 10 }
                    Text { x: 4; y: 57; text: "-15dB"; color: "#888"; font.pixelSize: 10 }
                    
                    Canvas {
                        id: eqCanvas
                        anchors.fill: parent
                        
                        Connections {
                            target: root
                            function onEqChanged() { eqCanvas.requestPaint() }
                        }
                        
                        onPaint: {
                            let ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            
                            let points = [];
                            const padX = width / 10 / 2;
                            
                            // Gather points
                            for (let i = 0; i < eqRepeater.count; ++i) {
                                let slider = eqRepeater.itemAt(i).children[0].children[2]; // Access the Slider
                                let x = (i * (width / 10)) + padX;
                                // visualPosition: 0 is top (+15dB), 1 is bottom (-15dB)
                                // We want to map this to the canvas height. 
                                // Canvas height is 80. +15dB is at y=15, -15dB is at y=65.
                                // Range is 50 pixels, centered at 40.
                                let y = 15 + (slider.visualPosition * 50);
                                points.push({x: x, y: y});
                            }
                            
                            // Extend points to edges for a continuous curve
                            points.unshift({x: 0, y: points[0].y});
                            points.push({x: width, y: points[points.length-1].y});
                            
                            // Draw spline
                            ctx.beginPath();
                            ctx.moveTo(points[0].x, points[0].y);
                            for (let j = 0; j < points.length - 1; j++) {
                                let p0 = (j > 0) ? points[j - 1] : points[j];
                                let p1 = points[j];
                                let p2 = points[j + 1];
                                let p3 = (j != points.length - 2) ? points[j + 2] : p2;

                                let cp1x = p1.x + (p2.x - p0.x) / 6;
                                let cp1y = p1.y + (p2.y - p0.y) / 6;
                                let cp2x = p2.x - (p3.x - p1.x) / 6;
                                let cp2y = p2.y - (p3.y - p1.y) / 6;

                                ctx.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, p2.x, p2.y);
                            }
                            
                            ctx.lineWidth = 2;
                            ctx.strokeStyle = "#1df2ea";
                            ctx.stroke();
                            
                            // Fill area under curve
                            ctx.lineTo(width, height);
                            ctx.lineTo(0, height);
                            ctx.closePath();
                            
                            let gradient = ctx.createLinearGradient(0, 15, 0, 65);
                            gradient.addColorStop(0, "rgba(29, 242, 234, 0.4)");
                            gradient.addColorStop(1, "rgba(29, 242, 234, 0.0)");
                            ctx.fillStyle = gradient;
                            ctx.fill();
                        }
                    }
                }
                
                // Bottom section: Sliders
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    // Continuous zero line behind sliders
                    Rectangle {
                        x: 0
                        y: 83 // (160 / 2) + 3 (approx center of sliders)
                        width: parent.width
                        height: 1
                        color: "#444"
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        spacing: 0
                        
                        Repeater {
                            id: eqRepeater
                            model: [
                                { label: "32 Hz", prop: "eq32", hoverColor: "#1a2c3a" },
                                { label: "64 Hz", prop: "eq64", hoverColor: "#1a2c3a" },
                                { label: "125 Hz", prop: "eq125", hoverColor: "#1a2c3a" },
                                { label: "250 Hz", prop: "eq250", hoverColor: "#1a2c3a" },
                                { label: "500 Hz", prop: "eq500", hoverColor: "#163130" },
                                { label: "1 kHz", prop: "eq1k", hoverColor: "#163130" },
                                { label: "2 kHz", prop: "eq2k", hoverColor: "#222222" },
                                { label: "4 kHz", prop: "eq4k", hoverColor: "#222222" },
                                { label: "8 kHz", prop: "eq8k", hoverColor: "#333333" },
                                { label: "16 kHz", prop: "eq16k", hoverColor: "#222222" }
                            ]
                            
                            Rectangle {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                color: hoverArea.containsMouse ? modelData.hoverColor : "transparent"
                                radius: 4
                                
                                MouseArea {
                                    id: hoverArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    acceptedButtons: Qt.NoButton
                                }
                                
                                function resetSlider() {
                                    bandSlider.value = 0
                                }
                                
                                function updateValue(val) {
                                    bandSlider.value = val
                                }
                                
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 4
                                    
                                    Text {
                                        text: Math.round(bandSlider.value) + " dB"
                                        color: Theme.text
                                        font.pixelSize: 11
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                    
                                    Slider {
                                        id: bandSlider
                                        Layout.fillHeight: true
                                        Layout.alignment: Qt.AlignHCenter
                                        orientation: Qt.Vertical
                                        from: -15
                                        to: 15
                                        value: (root.effectController && root.effectController.audio) ? root.effectController.audio[modelData.prop] : 0
                                        
                                        onValueChanged: {
                                            if (root.effectController && root.effectController.audio && root.effectController.audio[modelData.prop] !== value) {
                                                root.effectController.audio[modelData.prop] = value
                                            }
                                            root.eqChanged()
                                        }
                                        
                                        background: Rectangle {
                                            x: bandSlider.leftPadding + bandSlider.availableWidth / 2 - width / 2
                                            y: bandSlider.topPadding + 6
                                            implicitWidth: 3
                                            implicitHeight: 100
                                            width: implicitWidth
                                            height: bandSlider.availableHeight - 12
                                            radius: 2
                                            color: "#181b1f"
                                            
                                            Rectangle {
                                                width: parent.width
                                                y: bandSlider.value >= 0 ? bandSlider.visualPosition * parent.height : parent.height / 2
                                                height: Math.abs(bandSlider.visualPosition - 0.5) * parent.height
                                                radius: 2
                                                color: "#38bdf8"
                                            }
                                        }
                                        
                                        handle: Rectangle {
                                            x: bandSlider.leftPadding + bandSlider.availableWidth / 2 - width / 2
                                            y: bandSlider.topPadding + bandSlider.visualPosition * (bandSlider.availableHeight - height)
                                            implicitWidth: 20
                                            implicitHeight: 12
                                            radius: 6
                                            border.color: "#111"
                                            border.width: 1
                                            
                                            gradient: Gradient {
                                                GradientStop { position: 0.0; color: bandSlider.pressed ? "#666" : "#777" }
                                                GradientStop { position: 1.0; color: bandSlider.pressed ? "#444" : "#555" }
                                            }
                                            
                                            // Equals icon inside handle
                                            Column {
                                                anchors.centerIn: parent
                                                spacing: 2
                                                Rectangle { width: 8; height: 1.5; color: "#eee" }
                                                Rectangle { width: 8; height: 1.5; color: "#eee" }
                                            }
                                        }
                                    }
                                    
                                    Text {
                                        text: modelData.label
                                        color: "#aaa"
                                        font.pixelSize: 10
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        Connections {
            target: root.effectController ? root.effectController.audio : null
            function onEq32Changed() { if (eqRepeater.itemAt(0)) eqRepeater.itemAt(0).updateValue(root.effectController.audio.eq32) }
            function onEq64Changed() { if (eqRepeater.itemAt(1)) eqRepeater.itemAt(1).updateValue(root.effectController.audio.eq64) }
            function onEq125Changed() { if (eqRepeater.itemAt(2)) eqRepeater.itemAt(2).updateValue(root.effectController.audio.eq125) }
            function onEq250Changed() { if (eqRepeater.itemAt(3)) eqRepeater.itemAt(3).updateValue(root.effectController.audio.eq250) }
            function onEq500Changed() { if (eqRepeater.itemAt(4)) eqRepeater.itemAt(4).updateValue(root.effectController.audio.eq500) }
            function onEq1kChanged() { if (eqRepeater.itemAt(5)) eqRepeater.itemAt(5).updateValue(root.effectController.audio.eq1k) }
            function onEq2kChanged() { if (eqRepeater.itemAt(6)) eqRepeater.itemAt(6).updateValue(root.effectController.audio.eq2k) }
            function onEq4kChanged() { if (eqRepeater.itemAt(7)) eqRepeater.itemAt(7).updateValue(root.effectController.audio.eq4k) }
            function onEq8kChanged() { if (eqRepeater.itemAt(8)) eqRepeater.itemAt(8).updateValue(root.effectController.audio.eq8k) }
            function onEq16kChanged() { if (eqRepeater.itemAt(9)) eqRepeater.itemAt(9).updateValue(root.effectController.audio.eq16k) }
        }
    }
    
    Component.onCompleted: {
        // Trigger initial draw
        root.eqChanged()
    }
}
