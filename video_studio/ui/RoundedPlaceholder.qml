import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

Item {
    id: root
    
    property alias contentContainer: container

    Rectangle {
        id: maskRect
        anchors.fill: parent
        radius: 8
        visible: false
        layer.enabled: true
    }

    Item {
        id: container
        anchors.fill: parent
        visible: false
        layer.enabled: true
    }

    MultiEffect {
        source: container
        anchors.fill: container
        maskEnabled: true
        maskSource: maskRect
    }
}
