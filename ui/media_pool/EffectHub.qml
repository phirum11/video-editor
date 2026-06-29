pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import VideoStudio.Models 1.0

import VideoStudioUI

Rectangle {
    id: effectHubRoot
    color: Theme.background
    EffectHubModel {
        id: effectsModel
    }
    
    property string currentCategory: "All"

    signal effectActivated(string name, string filePath)
    
    function loadEffects(category) {
        currentCategory = category;
        // qmllint disable missing-property
        effectsModel.loadEffects(category);
        // qmllint enable missing-property
    }
    
    Component.onCompleted: loadEffects("All")
    // qmllint disable
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Search bar area
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 46
            color: Theme.background
            
            Rectangle {
                anchors.fill: parent
                anchors.margins: 10
                radius: 3
                color: Theme.surfaceInset
                border.color: searchInput.activeFocus ? "#66aacf" : Theme.dividerSoft
                border.width: 1

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 9
                    anchors.verticalCenter: parent.verticalCenter
                    text: "\u2315"
                    color: "#9badb5"
                    font.pixelSize: 14
                }

                TextField {
                    id: searchInput
                    anchors.left: parent.left
                    anchors.leftMargin: 28
                    anchors.right: parent.right
                    anchors.rightMargin: 7
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    placeholderText: qsTr("Search Effects")
                    color: Theme.text
                    placeholderTextColor: Theme.textMuted
                    font.pixelSize: 13
                    selectByMouse: true
                    background: Item {}
                    
                    onTextChanged: {
                        // qmllint disable missing-property
                        effectsModel.search(text)
                        // qmllint enable missing-property
                    }
                }
            }
        }
        // qmllint disable
        // Split View (Sidebar + Grid)
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // Left Sidebar: Categories
            Rectangle {
                Layout.preferredWidth: 120
                Layout.fillHeight: true
                color: Theme.background
                border.color: Theme.dividerSoft
                border.width: 1
                clip: true
                
                ScrollView {
                    id: categoriesScrollView
                    anchors.fill: parent
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    clip: true
                    
                    ColumnLayout {
                        width: categoriesScrollView.availableWidth
                        spacing: 2
                        
                        CategoryButton { title: "All" }
                        CategoryButton { title: "Trending" }
                        CategoryButton { title: "Basic" }
                        CategoryButton { title: "Bling" }
                        CategoryButton { title: "Party" }
                        CategoryButton { title: "Retro" }
                        CategoryButton { title: "Comic" }
                        CategoryButton { title: "Split" }
                        CategoryButton { title: "Nature" }
                        CategoryButton { title: "Transitions" }
                    }
                }
            }
            
            // Right Area: Grid View of Effects
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "transparent"
                border.color: Theme.dividerSoft
                border.width: 1
                clip: true
                
                GridView {
                    id: effectsGrid
                    anchors.fill: parent
                    anchors.margins: 12
                    cellWidth: 110
                    cellHeight: 130
                    model: effectsModel
                    clip: true
                    
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    
                    delegate: Item {
                        id: effectDelegateRoot
                        required property string title
                        required property url fileURL
                        readonly property string filePathString: String(fileURL)
                        
                        width: effectsGrid.cellWidth
                        height: effectsGrid.cellHeight

                        Item {
                            id: dragTargetDummy
                        }

                        Rectangle {
                            id: effectDragProxy
                            readonly property string effectTitle: effectDelegateRoot.title
                            readonly property string effectFilePath: effectDelegateRoot.filePathString
                            property point startOverlayPos: Qt.point(0, 0)
                            property bool isDragging: false

                            parent: isDragging ? Overlay.overlay : effectDelegateRoot
                            x: isDragging ? startOverlayPos.x + dragTargetDummy.x : effectThumb.x
                            y: isDragging ? startOverlayPos.y + dragTargetDummy.y : effectThumb.y
                            width: effectThumb.width
                            height: effectThumb.height
                            radius: effectThumb.radius
                            visible: isDragging
                            opacity: 0.84
                            color: "#12242d"
                            border.color: "#66aacf"
                            border.width: 1
                            z: 99999

                            Drag.active: isDragging
                            Drag.source: effectDragProxy
                            Drag.keys: ["videoStudio/effect"]
                            Drag.supportedActions: Qt.CopyAction
                            Drag.hotSpot.x: thumbMouse.pressPos.x
                            Drag.hotSpot.y: thumbMouse.pressPos.y

                            AnimatedImage {
                                anchors.fill: parent
                                anchors.margins: 1
                                source: effectDelegateRoot.fileURL
                                fillMode: Image.PreserveAspectCrop
                                playing: true
                            }
                        }
                        
                        Rectangle {
                            id: effectThumb
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 96
                            height: 96
                            radius: 12
                            color: "#1c2830"
                            border.color: thumbMouse.containsMouse ? "#66aacf" : "#304352"
                            border.width: 1
                            
                            clip: true
                            
                            AnimatedImage {
                                id: effectAnimImage
                                anchors.fill: parent
                                anchors.margins: 1
                                source: effectDelegateRoot.fileURL
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                sourceSize.width: 96
                                sourceSize.height: 96
                                playing: thumbMouse.containsMouse
                                visible: false
                            }
                            
                            Rectangle {
                                id: maskRect
                                anchors.fill: effectAnimImage
                                radius: 12
                                color: "black"
                                visible: false
                                layer.enabled: true
                            }
                            
                            MultiEffect {
                                anchors.fill: effectAnimImage
                                source: effectAnimImage
                                maskEnabled: true
                                maskSource: maskRect
                            }
                            
                            // Star Icon
                            Rectangle {
                                width: 22
                                height: 22
                                radius: 4
                                color: "#80000000"
                                z: 2
                                anchors.right: addButton.left
                                anchors.rightMargin: 4
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 4
                                visible: thumbMouse.containsMouse
                                Text {
                                    anchors.centerIn: parent
                                    text: "☆"
                                    color: "white"
                                    font.pixelSize: 16
                                    anchors.verticalCenterOffset: -1
                                }
                            }
                            
                            // Add Button
                            Rectangle {
                                id: addButton
                                width: 22
                                height: 22
                                radius: 11
                                color: addMouse.pressed ? "#009ea3" : "#00c4cc"
                                z: 3
                                anchors.right: parent.right
                                anchors.rightMargin: 4
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 4
                                visible: thumbMouse.containsMouse
                                Text {
                                    anchors.centerIn: parent
                                    text: "+"
                                    color: "white"
                                    font.pixelSize: 18
                                    font.bold: true
                                    anchors.verticalCenterOffset: -2
                                }
                                MouseArea {
                                    id: addMouse
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        effectHubRoot.effectActivated(effectDelegateRoot.title, effectDelegateRoot.filePathString)
                                    }
                                }
                            }
                            
                            MouseArea {
                                id: thumbMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                z: 1
                                drag.target: dragTargetDummy
                                drag.threshold: 8
                                preventStealing: true

                                property point pressPos: Qt.point(width / 2, height / 2)

                                onPressed: (mouse) => {
                                    pressPos = Qt.point(mouse.x, mouse.y)
                                }

                                onPositionChanged: {
                                    if (drag.active && !effectDragProxy.isDragging) {
                                        effectDragProxy.startOverlayPos = effectThumb.mapToItem(Overlay.overlay, 0, 0)
                                        effectDragProxy.isDragging = true
                                    }
                                }
                                onReleased: {
                                    if (effectDragProxy.isDragging) {
                                        effectDragProxy.Drag.drop()
                                    }
                                    effectDragProxy.isDragging = false
                                    dragTargetDummy.x = 0
                                    dragTargetDummy.y = 0
                                }
                                onCanceled: {
                                    effectDragProxy.isDragging = false
                                    dragTargetDummy.x = 0
                                    dragTargetDummy.y = 0
                                }
                            }
                        }
                        
                        Text {
                            anchors.top: effectThumb.bottom
                            anchors.topMargin: 8
                            anchors.left: parent.left
                            anchors.right: parent.right
                            text: effectDelegateRoot.title
                            color: Theme.text
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            maximumLineCount: 1
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
    
    component CategoryButton: Rectangle {
        id: categoryButtonRoot
        property string title: ""
        
        implicitWidth: 100
        implicitHeight: 32
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        Layout.leftMargin: 6
        Layout.rightMargin: 6
        radius: 6
        color: effectHubRoot.currentCategory === categoryButtonRoot.title ? Theme.surfacePressed : (catMouse.containsMouse ? Theme.surfaceHover : "transparent")
        
        Behavior on color {
            ColorAnimation { duration: 150; easing.type: Easing.OutQuad }
        }
        
        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            width: 3
            height: effectHubRoot.currentCategory === categoryButtonRoot.title ? parent.height - 12 : 0
            radius: 1.5
            color: "#66aacf"
            opacity: effectHubRoot.currentCategory === categoryButtonRoot.title ? 1.0 : 0.0
            
            Behavior on height {
                NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
            }
            Behavior on opacity {
                NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
            }
        }
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: categoryButtonRoot.title
            color: effectHubRoot.currentCategory === categoryButtonRoot.title ? Theme.text : (catMouse.containsMouse ? Theme.text : "#aeb9be")
            font.pixelSize: 13
            font.bold: effectHubRoot.currentCategory === categoryButtonRoot.title
            
            Behavior on color {
                ColorAnimation { duration: 150; easing.type: Easing.OutQuad }
            }
        }
        
        MouseArea {
            id: catMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: effectHubRoot.loadEffects(categoryButtonRoot.title)
        }
    }
}
