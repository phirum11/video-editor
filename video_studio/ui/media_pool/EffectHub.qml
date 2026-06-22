pragma ComponentBehavior: Bound

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
    
    function loadEffects(category) {
        currentCategory = category;
        effectsModel.loadEffects(category);
    }
    
    Component.onCompleted: loadEffects("All")
    
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
                        effectsModel.search(text)
                    }
                }
            }
        }
        
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
                    anchors.fill: parent
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    
                    ColumnLayout {
                        width: parent.width
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
                        
                        width: effectsGrid.cellWidth
                        height: effectsGrid.cellHeight
                        
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
                                source: fileURL
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
                                color: "#00c4cc"
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
                            }
                            
                            MouseArea {
                                id: thumbMouse
                                anchors.fill: parent
                                hoverEnabled: true
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
        
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        color: effectHubRoot.currentCategory === categoryButtonRoot.title ? Theme.surfacePressed : (catMouse.containsMouse ? Theme.surfaceHover : "transparent")
        
        Rectangle {
            width: 3
            height: parent.height
            color: "#66aacf"
            visible: effectHubRoot.currentCategory === categoryButtonRoot.title
        }
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: categoryButtonRoot.title
            color: effectHubRoot.currentCategory === categoryButtonRoot.title ? Theme.text : "#aeb9be"
            font.pixelSize: 13
            font.bold: effectHubRoot.currentCategory === categoryButtonRoot.title
        }
        
        MouseArea {
            id: catMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: effectHubRoot.loadEffects(categoryButtonRoot.title)
        }
    }
}
