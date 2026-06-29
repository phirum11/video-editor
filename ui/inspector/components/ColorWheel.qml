// qmllint disable
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI

Item {
    id: root
    implicitWidth: 80
    implicitHeight: 110

    function reset() {
        wheelArea.handleX = wheelArea.width / 2;
        wheelArea.handleY = 4;
        colorSlider.value = 0;
    }

    property color selectedColor: {
        let cx = wheelArea.width / 2;
        let cy = wheelArea.height / 2;
        let dx = wheelArea.handleX - cx;
        let dy = wheelArea.handleY - cy;
        let maxRadius = Math.min(cx, cy) - 3;
        let distance = Math.min(maxRadius, Math.sqrt(dx * dx + dy * dy));
        let saturation = distance > 0 ? distance / maxRadius : 0;
        let angle = Math.atan2(dy, dx) * 180 / Math.PI;
        if (angle < 0) angle += 360;
        let lightness = 0.5 + (colorSlider.value / 200.0);
        return Qt.hsla(angle / 360, saturation, lightness, 1.0);
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // The Color Wheel Ring
        Item {
            id: wheelArea
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 60
            Layout.preferredHeight: 60
            
            property real handleX: width / 2
            property real handleY: 4 // top margin equivalent

            Canvas {
                anchors.fill: parent
                onPaint: {
                    let ctx = getContext("2d");
                    let cx = width / 2;
                    let cy = height / 2;
                    let radius = Math.min(cx, cy);
                    
                    // Draw outer color ring
                    for (let angle = 0; angle <= 360; angle += 2) {
                        let startAngle = (angle - 1) * Math.PI / 180;
                        let endAngle = (angle + 2) * Math.PI / 180;
                        ctx.beginPath();
                        ctx.moveTo(cx, cy);
                        ctx.arc(cx, cy, radius, startAngle, endAngle);
                        ctx.closePath();
                        ctx.fillStyle = "hsl(" + angle + ", 100%, 50%)";
                        ctx.fill();
                    }
                    
                    // Draw inner hole to make it a ring
                    ctx.globalCompositeOperation = "destination-out";
                    ctx.beginPath();
                    ctx.arc(cx, cy, radius * 0.5, 0, 2 * Math.PI);
                    ctx.fill();
                    
                    // Reset
                    ctx.globalCompositeOperation = "source-over";
                }
            }

            // Color dot indicator
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.selectedColor
                border.color: Theme.text
                border.width: 1
                x: wheelArea.handleX - width / 2
                y: wheelArea.handleY - height / 2
            }
            
            MouseArea {
                anchors.fill: parent
                preventStealing: true
                
                function updatePosition(mouse) {
                    let cx = wheelArea.width / 2;
                    let cy = wheelArea.height / 2;
                    let maxRadius = Math.min(cx, cy) - 3;
                    
                    let dx = mouse.x - cx;
                    let dy = mouse.y - cy;
                    let distance = Math.sqrt(dx * dx + dy * dy);
                    
                    if (distance > maxRadius) {
                        dx = (dx / distance) * maxRadius;
                        dy = (dy / distance) * maxRadius;
                    }
                    
                    wheelArea.handleX = cx + dx;
                    wheelArea.handleY = cy + dy;
                }
                
                onPressed: (mouse) => updatePosition(mouse)
                onPositionChanged: (mouse) => updatePosition(mouse)
            }
        }

        // The Slider below the wheel
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            // Color preview box left
            Rectangle {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 12
                color: root.selectedColor
                radius: 2
                border.color: "#333"
                border.width: 1
            }
            
            // Slider line
            Slider {
                id: colorSlider
                Layout.fillWidth: true
                Layout.preferredHeight: 12
                from: -100
                to: 100
                value: 0
                
                background: Rectangle {
                    x: colorSlider.leftPadding
                    y: colorSlider.topPadding + colorSlider.availableHeight / 2 - height / 2
                    width: colorSlider.availableWidth
                    height: 2
                    color: "#555"
                }
                
                handle: Rectangle {
                    x: colorSlider.leftPadding + colorSlider.visualPosition * (colorSlider.availableWidth - width)
                    anchors.verticalCenter: parent.verticalCenter
                    width: 10
                    height: 10
                    radius: 5
                    color: colorSlider.pressed ? Theme.text : "#ddd"
                }
            }
            
            // Number value right
            Rectangle {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 12
                color: "#161616"
                radius: 2
                Text {
                    anchors.centerIn: parent
                    text: Math.round(colorSlider.value)
                    color: "#ccc"
                    font.pixelSize: 9
                }
            }
        }
    }
}
