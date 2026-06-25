import QtQuick
import VideoStudioUI

Rectangle {
    id: gapBoxRoot
    
    property real timelineEndSeconds: 0
    property real pixelsPerSecond: 18
    property real scrollOffset: 0
    
    // Position and size based on the timeline length
    x: -scrollOffset
    y: 0
    width: Math.max(0, timelineEndSeconds * pixelsPerSecond)
    height: parent.height
    
    color: Theme.surfaceRaised
    radius: 4
}
