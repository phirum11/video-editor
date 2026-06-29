// qmllint disable
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: overlayRoot
    
    // Bind to BlurEffect in C++
    property var blurEffect: null
    
    // These will be bound to the video's actual display rect inside the monitor
    property real videoX: 0
    property real videoY: 0
    property real videoWidth: width
    property real videoHeight: height
    property real minBoxSize: 32
    
    visible: blurEffect && blurEffect.isRegionEnabled && blurEffect.radius > 0
             && videoWidth > 1 && videoHeight > 1
    
    // Internal mapped coordinates (from 0.0-1.0 to pixel coordinates inside the video rect)
    property real mappedWidth: blurEffect ? Math.min(videoWidth, Math.max(minBoxSize, blurEffect.regionWidth * videoWidth)) : 0
    property real mappedHeight: blurEffect ? Math.min(videoHeight, Math.max(minBoxSize, blurEffect.regionHeight * videoHeight)) : 0
    property real mappedX: Math.max(videoX, Math.min(videoX + videoWidth - mappedWidth, videoX + (blurEffect ? blurEffect.regionX * videoWidth : 0)))
    property real mappedY: Math.max(videoY, Math.min(videoY + videoHeight - mappedHeight, videoY + (blurEffect ? blurEffect.regionY * videoHeight : 0)))

    // Prevent recursive updates
    property bool isDragging: false
    property bool pendingUpdate: false
    
    Timer {
        id: throttleTimer
        interval: 40 // ~25fps throttle to keep UI responsive
        repeat: false
        onTriggered: {
            if (pendingUpdate) {
                pendingUpdate = false
                overlayRoot.updateModelInternal()
                throttleTimer.start()
            }
        }
    }
    
    function updateModel() {
        if (!blurEffect || !isDragging) return
        if (throttleTimer.running) {
            pendingUpdate = true
            return
        }
        updateModelInternal()
        throttleTimer.start()
    }

    function updateModelInternal() {
        if (!blurEffect || videoWidth <= 1 || videoHeight <= 1) return
        
        let newRw = Math.min(1.0, Math.max(minBoxSize / videoWidth, box.width / videoWidth))
        let newRh = Math.min(1.0, Math.max(minBoxSize / videoHeight, box.height / videoHeight))
        let newRx = (box.x - videoX) / videoWidth
        let newRy = (box.y - videoY) / videoHeight
        
        newRx = Math.max(0, Math.min(1.0 - newRw, newRx))
        newRy = Math.max(0, Math.min(1.0 - newRh, newRy))
        
        blurEffect.updateRegion(newRx, newRy, newRw, newRh)
    }

    Rectangle {
        id: box
        
        Binding { target: box; property: "x"; value: overlayRoot.mappedX; when: !overlayRoot.isDragging; restoreMode: Binding.RestoreNone }
        Binding { target: box; property: "y"; value: overlayRoot.mappedY; when: !overlayRoot.isDragging; restoreMode: Binding.RestoreNone }
        Binding { target: box; property: "width"; value: overlayRoot.mappedWidth; when: !overlayRoot.isDragging; restoreMode: Binding.RestoreNone }
        Binding { target: box; property: "height"; value: overlayRoot.mappedHeight; when: !overlayRoot.isDragging; restoreMode: Binding.RestoreNone }
        
        color: "transparent"
        border.color: "#e63956" // Red dashed line
        border.width: 2
        radius: 8
        
        // Dashed border effect using a custom Canvas or standard if Qt allows (QML Rectangle doesn't have dashed borders built-in, but we can fake it or just use solid for now)
        // Actually, just a solid red line is fine, or we can use a small repeater to make dashed lines later.
        
        // Dragging the entire box
        MouseArea {
            anchors.fill: parent
            anchors.margins: 10
            drag.target: box
            drag.axis: Drag.XAndYAxis
            drag.minimumX: overlayRoot.videoX
            drag.minimumY: overlayRoot.videoY
            drag.maximumX: Math.max(overlayRoot.videoX, overlayRoot.videoX + overlayRoot.videoWidth - box.width)
            drag.maximumY: Math.max(overlayRoot.videoY, overlayRoot.videoY + overlayRoot.videoHeight - box.height)
            cursorShape: Qt.SizeAllCursor
            
            onPressed: overlayRoot.isDragging = true
            onReleased: {
                overlayRoot.updateModel()
                overlayRoot.isDragging = false
            }
            onPositionChanged: {
                if (pressed) overlayRoot.updateModel()
            }
        }
        
        // Label for the effect (like "Blur 18")
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 12
            anchors.topMargin: 12
            width: 80
            height: 24
            color: "#80222222"
            radius: 12
            
            Text {
                anchors.centerIn: parent
                text: "Blur " + (overlayRoot.blurEffect ? Math.round(overlayRoot.blurEffect.radius) : "0")
                color: "white"
                font.pixelSize: 12
            }
        }
        
        // Resize handle (Bottom Right)
        Rectangle {
            id: resizeHandle
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: -10
            width: 24
            height: 24
            radius: 12
            color: "transparent"
            border.color: "#e63956"
            border.width: 2
            
            Rectangle {
                anchors.centerIn: parent
                width: 10
                height: 10
                color: "#e63956"
                radius: 2
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                
                property real startMouseX: 0
                property real startMouseY: 0
                property real startBoxWidth: 0
                property real startBoxHeight: 0
                
                onPressed: (mouse) => {
                    overlayRoot.isDragging = true
                    let pos = mapToItem(overlayRoot, mouse.x, mouse.y)
                    startMouseX = pos.x
                    startMouseY = pos.y
                    startBoxWidth = box.width
                    startBoxHeight = box.height
                }
                
                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let pos = mapToItem(overlayRoot, mouse.x, mouse.y)
                        let dx = pos.x - startMouseX
                        let dy = pos.y - startMouseY
                        
                        let newWidth = startBoxWidth + dx
                        let newHeight = startBoxHeight + dy
                        
                        // Minimum size
                        newWidth = Math.max(overlayRoot.minBoxSize, newWidth)
                        newHeight = Math.max(overlayRoot.minBoxSize, newHeight)
                        
                        // Clamp to video edges
                        newWidth = Math.min(newWidth, overlayRoot.videoX + overlayRoot.videoWidth - box.x)
                        newHeight = Math.min(newHeight, overlayRoot.videoY + overlayRoot.videoHeight - box.y)
                        
                        box.width = newWidth
                        box.height = newHeight
                        
                        overlayRoot.updateModel()
                    }
                }
                
                onReleased: {
                    overlayRoot.updateModelInternal()
                    overlayRoot.isDragging = false
                }
            }
        }
    }
}
