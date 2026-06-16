// qmllint disable unqualified
// qmllint disable missing-property
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root
    property string label: "Slider"
    property real labelWidth: 85
    property alias value: sliderControl.value
    property real defaultValue: 100
    property real min: 0
    property real max: 1000
    property string suffix: ""
    property string tooltipText: ""

    implicitHeight: 24
    implicitWidth: rowLayout.implicitWidth
    Layout.fillWidth: true

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        spacing: 6
        
        HoverHandler { id: hoverHandler }
        ToolTip.visible: hoverHandler.hovered && root.tooltipText !== ""
        ToolTip.text: root.tooltipText
        ToolTip.delay: 500

        Text {
            Layout.preferredWidth: root.labelWidth
            Layout.minimumWidth: root.labelWidth
            Layout.maximumWidth: root.labelWidth
            elide: Text.ElideRight
            text: root.label
            color: "#d0d0d0"
            font.pixelSize: 12
            visible: root.labelWidth > 0
        }

        Slider {
            id: sliderControl
            Layout.fillWidth: true
            from: root.min
            to: root.max
            value: root.defaultValue
            
            background: Rectangle {
                x: sliderControl.leftPadding
                y: sliderControl.topPadding + sliderControl.availableHeight / 2 - height / 2
                implicitWidth: 10
                implicitHeight: 2
                width: sliderControl.availableWidth
                height: 2
                color: "#444"
                
                Rectangle {
                    width: sliderControl.visualPosition * parent.width
                    height: parent.height
                    color: "#2792c3" // Premiere's blue
                }
            }
            
            handle: Rectangle {
                x: sliderControl.leftPadding + sliderControl.visualPosition * (sliderControl.availableWidth - width)
                y: sliderControl.topPadding + sliderControl.availableHeight / 2 - height / 2
                implicitWidth: 10
                implicitHeight: 10
                radius: 5
                color: sliderControl.pressed ? "#fff" : "#ccc"
            }
        }

        Text {
            Layout.preferredWidth: 40
            Layout.minimumWidth: 40
            Layout.maximumWidth: 40
            text: Math.round(sliderControl.value).toString() + root.suffix
            color: "#d0d0d0"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignRight
        }
        
        // Reset icon
        Image {
            source: "qrc:/VideoStudioUI/assets/undo.svg"
            Layout.preferredWidth: 12
            Layout.preferredHeight: 12
            opacity: resetMouse.containsMouse ? 1.0 : 0.6
            fillMode: Image.PreserveAspectFit
            
            MouseArea {
                id: resetMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: sliderControl.value = root.defaultValue
            }
        }
    }
}
