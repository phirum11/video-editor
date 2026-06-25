// qmllint disable unqualified
// qmllint disable missing-property
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Shapes
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
        
        // Basic Audio Card
        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 10
            implicitHeight: basicAudioLayout.implicitHeight + 28
            color: "#212121"
            radius: 10
            border.color: "#303030"
            border.width: 1

            ColumnLayout {
                id: basicAudioLayout
                anchors.fill: parent
                anchors.margins: 14
                spacing: 16
                
                // Header row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    
                    CheckBox {
                        id: basicAudioCheck
                        checked: true
                        text: "Basic Audio"
                        font.pixelSize: 14
                        
                        indicator: Rectangle {
                            implicitWidth: 16
                            implicitHeight: 16
                            y: parent.height / 2 - height / 2
                            radius: 4
                            color: basicAudioCheck.checked ? "#38bdf8" : "transparent"
                            border.color: basicAudioCheck.checked ? "#38bdf8" : "#555"
                            Image {
                                anchors.centerIn: parent
                                width: 10
                                height: 10
                                source: "qrc:/VideoStudioUI/assets/check.svg"
                                visible: basicAudioCheck.checked
                            }
                        }
                        contentItem: Text {
                            text: basicAudioCheck.text
                            color: Theme.text
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: basicAudioCheck.indicator.width + basicAudioCheck.spacing
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Image {
                        source: "qrc:/VideoStudioUI/assets/undo.svg"
                        Layout.preferredWidth: 14
                        Layout.preferredHeight: 14
                        opacity: basicUndoArea.containsMouse ? 1.0 : 0.6
                        MouseArea {
                            id: basicUndoArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                volumeSlider.value = 0
                                panSliderInternal.value = 0
                                pitchSlider.value = 0
                            }
                        }
                    }
                }
                
                // Controls Row
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 24
                    
                    // Left Column (Volume & Pitch)
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 200
                        spacing: 20
                        
                        // Volume Slider
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            Text {
                                text: "Volume"
                                color: "#d0d0d0"
                                font.pixelSize: 12
                                Layout.preferredWidth: 60
                            }
                            
                            Slider {
                                id: volumeSlider
                                Layout.fillWidth: true
                                from: -60
                                to: 12
                                value: effectController ? effectController.audio.volume : 0
                                onValueChanged: { if(effectController && effectController.audio.volume !== value) effectController.audio.volume = value }
                                
                                background: Rectangle {
                                    x: volumeSlider.leftPadding
                                    y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                                    width: volumeSlider.availableWidth
                                    height: 14
                                    radius: 7
                                    color: "#181b1f"
                                    border.color: "#111"
                                    clip: true
                                    Rectangle {
                                        width: volumeSlider.visualPosition * parent.width
                                        height: parent.height
                                        radius: parent.radius
                                        color: "#1df2ea"
                                        // Glow outer
                                        Rectangle {
                                            anchors.fill: parent
                                            color: "transparent"
                                            border.color: Theme.text
                                            border.width: 1
                                            opacity: 0.3
                                            radius: parent.radius
                                        }
                                        // Inner meter vertical lines
                                        Row {
                                            anchors.fill: parent
                                            anchors.leftMargin: 4
                                            anchors.rightMargin: 4
                                            spacing: 2
                                            Repeater {
                                                model: Math.max(0, Math.floor((parent.width - 8) / 3))
                                                Rectangle {
                                                    width: 1
                                                    height: 10
                                                    y: 2
                                                    color: "#000000"
                                                    opacity: 0.25
                                                }
                                            }
                                        }
                                    }
                                }
                                
                                handle: Rectangle {
                                    x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                                    y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                                    implicitWidth: 10
                                    implicitHeight: 22
                                    radius: 5
                                    color: volumeSlider.pressed ? "#666" : "#444"
                                    border.color: "#222"
                                    
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 2
                                        height: 10
                                        color: "#888"
                                    }
                                }
                            }
                            
                            Text {
                                text: Math.round(volumeSlider.value) + " dB"
                                color: "#d0d0d0"
                                font.pixelSize: 12
                                Layout.preferredWidth: 35
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                        
                        // Pitch Slider
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            Text {
                                text: "Pitch"
                                color: "#d0d0d0"
                                font.pixelSize: 12
                                Layout.preferredWidth: 60
                            }
                            
                            Slider {
                                id: pitchSlider
                                Layout.fillWidth: true
                                from: -12
                                to: 12
                                value: 0
                                
                                background: Rectangle {
                                    x: pitchSlider.leftPadding
                                    y: pitchSlider.topPadding + pitchSlider.availableHeight / 2 - height / 2
                                    width: pitchSlider.availableWidth
                                    height: 4
                                    radius: 2
                                    color: "#181b1f"
                                }
                                
                                handle: Rectangle {
                                    x: pitchSlider.leftPadding + pitchSlider.visualPosition * (pitchSlider.availableWidth - width)
                                    y: pitchSlider.topPadding + pitchSlider.availableHeight / 2 - height / 2
                                    implicitWidth: 16
                                    implicitHeight: 10
                                    radius: 5
                                    color: pitchSlider.pressed ? "#777" : "#555"
                                    
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 2
                                        height: 6
                                        color: "#aaa"
                                    }
                                }
                            }
                            
                            Text {
                                text: (pitchSlider.value > 0 ? "+" : "") + Math.round(pitchSlider.value) + " st"
                                color: "#d0d0d0"
                                font.pixelSize: 12
                                Layout.preferredWidth: 35
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                    
                    // Pan text and Knob
                    Text {
                        text: "Pan"
                        color: "#d0d0d0"
                        font.pixelSize: 12
                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                        Layout.topMargin: 8
                    }
                    
                    Item {
                        id: panKnobArea
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                        Layout.leftMargin: 15
                        Layout.rightMargin: 10
                        
                        property real panValue: panSliderInternal.value
                        function clampPan(value) {
                            return Math.max(-100, Math.min(100, value))
                        }
                        
                        Shape {
                            anchors.fill: parent
                            layer.enabled: true
                            layer.samples: 4
                            
                            // Base arc
                            ShapePath {
                                fillColor: "transparent"
                                strokeColor: "#181b1f"
                                strokeWidth: 3
                                capStyle: ShapePath.RoundCap
                                
                                PathAngleArc {
                                    centerX: panKnobArea.width / 2
                                    centerY: panKnobArea.height / 2 - 4
                                    radiusX: 24
                                    radiusY: 24
                                    startAngle: 144
                                    sweepAngle: 252
                                }
                            }
                            
                            // Fake glow arc
                            ShapePath {
                                fillColor: "transparent"
                                strokeColor: panKnobArea.panValue !== 0 ? "#441df2ea" : "transparent"
                                strokeWidth: 6
                                capStyle: ShapePath.RoundCap
                                
                                PathAngleArc {
                                    centerX: panKnobArea.width / 2
                                    centerY: panKnobArea.height / 2 - 4
                                    radiusX: 24
                                    radiusY: 24
                                    startAngle: -90
                                    sweepAngle: (panKnobArea.panValue / 100.0) * 126
                                }
                            }
                            
                            // Active arc
                            ShapePath {
                                fillColor: "transparent"
                                strokeColor: panKnobArea.panValue !== 0 ? "#1df2ea" : "transparent"
                                strokeWidth: 3
                                capStyle: ShapePath.RoundCap
                                
                                PathAngleArc {
                                    centerX: panKnobArea.width / 2
                                    centerY: panKnobArea.height / 2 - 4
                                    radiusX: 24
                                    radiusY: 24
                                    startAngle: -90
                                    sweepAngle: (panKnobArea.panValue / 100.0) * 126
                                }
                            }
                        }
                        
                        Rectangle {
                            width: 42
                            height: 42
                            radius: 21
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -4
                            color: "#25282c"
                            border.color: "#333"
                            border.width: 1
                            
                            // Pointer
                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: "transparent"
                                border.color: "#1df2ea"
                                border.width: 2
                                x: 21 + 12 * Math.sin((panKnobArea.panValue / 100.0) * Math.PI * 0.7) - 7
                                y: 21 - 12 * Math.cos((panKnobArea.panValue / 100.0) * Math.PI * 0.7) - 7
                                
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 6; height: 6; radius: 3
                                    color: "#1df2ea"
                                }
                            }
                        }
                        
                        Text {
                            text: "L"
                            color: "#888"
                            font.pixelSize: 10
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.leftMargin: -2
                        }
                        Text {
                            text: "Center"
                            color: "#888"
                            font.pixelSize: 10
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "R"
                            color: "#888"
                            font.pixelSize: 10
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right
                            anchors.rightMargin: -2
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            preventStealing: true
                            cursorShape: Qt.PointingHandCursor
                            property real lastX
                            property real lastY
                            readonly property real dragSensitivity: 0.85
                            onPressed: function(mouse) {
                                lastX = mouse.x
                                lastY = mouse.y
                                mouse.accepted = true
                            }
                            onPositionChanged: function(mouse) {
                                if (!pressed)
                                    return

                                const horizontalDelta = mouse.x - lastX
                                const verticalDelta = lastY - mouse.y
                                const nextValue = panSliderInternal.value + (horizontalDelta + verticalDelta) * dragSensitivity
                                panSliderInternal.value = panKnobArea.clampPan(nextValue)
                                lastX = mouse.x
                                lastY = mouse.y
                            }
                            onDoubleClicked: panSliderInternal.value = 0
                        }
                        
                        Slider {
                            id: panSliderInternal
                            visible: false
                            from: -100
                            to: 100
                            value: effectController ? effectController.audio.pan : 0
                            onValueChanged: { if(effectController && effectController.audio.pan !== value) effectController.audio.pan = value }
                        }
                    }
                }
            }
        }
        // 10-Band Equalizer
        NeonEqualizer {
            effectController: rootScrollView.effectController || (typeof effectController !== "undefined" ? effectController : null)
        }
        
        Connections {
            target: effectController ? effectController.audio : null
            function onVolumeChanged() { volumeSlider.value = effectController.audio.volume }
            function onPanChanged() { panSliderInternal.value = effectController.audio.pan }
        }
        
        Item { Layout.fillHeight: true }
    }
}

