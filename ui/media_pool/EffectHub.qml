pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: effectHubRoot
    color: Theme.background
    
    ListModel {
        id: effectsModel
    }
    
    property string currentCategory: "Trending"
    
    function loadEffects(category) {
        currentCategory = category;
        effectsModel.clear();
        if (category === "Trending") {
            effectsModel.append({title: "Halo Blur"});
            effectsModel.append({title: "Smart Sharpen"});
            effectsModel.append({title: "Edge Glow"});
            effectsModel.append({title: "Zoom 3D"});
            effectsModel.append({title: "Camera Shake"});
            effectsModel.append({title: "Fisheye"});
        } else if (category === "Basic") {
            effectsModel.append({title: "Diamond Zoom"});
            effectsModel.append({title: "Slow Zoom"});
            effectsModel.append({title: "Blur"});
            effectsModel.append({title: "Slanted Blur"});
        } else if (category === "Bling") {
            effectsModel.append({title: "Sparkle"});
            effectsModel.append({title: "Kira"});
            effectsModel.append({title: "Star"});
            effectsModel.append({title: "Starlight"});
        } else if (category === "Party") {
            effectsModel.append({title: "Strobe"});
            effectsModel.append({title: "Neon Outline"});
            effectsModel.append({title: "Color Negative"});
            effectsModel.append({title: "Club Lights"});
        } else if (category === "Retro") {
            effectsModel.append({title: "Film Frame"});
            effectsModel.append({title: "VHS"});
            effectsModel.append({title: "1998"});
            effectsModel.append({title: "Noise"});
            effectsModel.append({title: "Nostalgia"});
        } else if (category === "Transitions") {
            effectsModel.append({title: "Pull In"});
            effectsModel.append({title: "Pull Out"});
            effectsModel.append({title: "Spin CW"});
            effectsModel.append({title: "Spin CCW"});
            effectsModel.append({title: "Cross Dissolve"});
        } else {
            // Default empty state placeholders
            effectsModel.append({title: "Effect 1"});
            effectsModel.append({title: "Effect 2"});
            effectsModel.append({title: "Effect 3"});
        }
    }
    
    Component.onCompleted: loadEffects("Trending")
    
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
                    color: "#dce4e7"
                    placeholderTextColor: "#6f8188"
                    font.pixelSize: 13
                    selectByMouse: true
                    background: Item {}
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
                        
                        CategoryButton { title: "Favorites" }
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
                    cellWidth: 84
                    cellHeight: 100
                    model: effectsModel
                    clip: true
                    
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    
                    delegate: Item {
                        id: effectDelegateRoot
                        required property string title
                        width: effectsGrid.cellWidth
                        height: effectsGrid.cellHeight
                        
                        Rectangle {
                            id: effectThumb
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 64
                            height: 64
                            radius: 8
                            color: "#1c2830"
                            border.color: thumbMouse.containsMouse ? "#66aacf" : "#304352"
                            border.width: 1
                            
                            clip: true
                            
                            Image {
                                anchors.fill: parent
                                anchors.margins: 1
                                source: effectHubRoot.currentCategory === "Retro" ? "qrc:/VideoStudioUI/assets/retro_effect.png" : 
                                        effectHubRoot.currentCategory === "Party" || effectHubRoot.currentCategory === "Bling" ? "qrc:/VideoStudioUI/assets/party_effect.png" : 
                                        "qrc:/VideoStudioUI/assets/trending_effect.png"
                                fillMode: Image.PreserveAspectCrop
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
                            color: "#dce4e7"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
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
            color: effectHubRoot.currentCategory === categoryButtonRoot.title ? "#ffffff" : "#aeb9be"
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
