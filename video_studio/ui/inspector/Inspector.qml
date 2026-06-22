import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import VideoStudioUI
import "tabs"

Rectangle {
    id: inspectorRoot
    color: Theme.background
    border.color: Theme.divider
    border.width: 1

    property int effectTab: 0
    property var effectController: null

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Top Tabs Row
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: Theme.background
            border.color: Theme.divider
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                spacing: 16

                Text {
                    text: "Effects"
                    color: "#2792c3" // Active tab color
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }

                Item { Layout.fillWidth: true }
                
                Text {
                    text: ">>"
                    color: "#8a9ba3"
                    font.pixelSize: 12
                    Layout.rightMargin: 12
                }
            }
            
            // Active tab indicator line
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: 50 // Width of "Effects" text approx
                height: 2
                color: "#2792c3"
            }
        }

        // 2. Search / Selection Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: Theme.background
            border.color: Theme.divider
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Theme.surfaceInset
                    radius: 2
                    border.color: Theme.dividerSoft
                    
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        text: "[No Selection]"
                        color: "#a0a0a0"
                        font.pixelSize: 12
                    }
                }
                
                Image {
                    source: "qrc:/VideoStudioUI/assets/undo.svg"
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    opacity: inspectorResetHover.hovered ? 1.0 : 0.7
                    
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (inspectorRoot.effectController) {
                                inspectorRoot.effectController.resetAll();
                            }
                        }
                    }
                    HoverHandler { id: inspectorResetHover }
                    ToolTip.visible: inspectorResetHover.hovered
                    ToolTip.text: "Reset All Effects"
                }
            }
        }

        // 2.5 Effects Tab Row
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: Theme.background
            border.color: Theme.divider
            border.width: 1

            Flickable {
                id: tabFlickable
                anchors.fill: parent
                contentWidth: tabRow.implicitWidth
                contentHeight: height
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    onWheel: (wheel) => {
                        var newX = tabFlickable.contentX - wheel.angleDelta.y;
                        tabFlickable.contentX = Math.max(0, Math.min(newX, Math.max(0, tabFlickable.contentWidth - tabFlickable.width)));
                    }
                }

                RowLayout {
                    id: tabRow
                    height: parent.height
                    spacing: 24
                    
                    Item { Layout.preferredWidth: 8 }
                    
                    Text {
                        text: "Basic"
                        color: inspectorRoot.effectTab === 0 ? "#2792c3" : "#8a9ba3"
                        font.pixelSize: 11
                        MouseArea { anchors.fill: parent; onClicked: inspectorRoot.effectTab = 0 }
                    }
                    Text {
                        text: "Color"
                        color: inspectorRoot.effectTab === 1 ? "#2792c3" : "#8a9ba3"
                        font.pixelSize: 11
                        MouseArea { anchors.fill: parent; onClicked: inspectorRoot.effectTab = 1 }
                    }
                    Text {
                        text: "Blur"
                        color: inspectorRoot.effectTab === 2 ? "#2792c3" : "#8a9ba3"
                        font.pixelSize: 11
                        MouseArea { anchors.fill: parent; onClicked: inspectorRoot.effectTab = 2 }
                    }
                    Text {
                        text: "Stylize"
                        color: inspectorRoot.effectTab === 3 ? "#2792c3" : "#8a9ba3"
                        font.pixelSize: 11
                        MouseArea { anchors.fill: parent; onClicked: inspectorRoot.effectTab = 3 }
                    }
                    Text {
                        text: "Audio"
                        color: inspectorRoot.effectTab === 4 ? "#2792c3" : "#8a9ba3"
                        font.pixelSize: 11
                        MouseArea { anchors.fill: parent; onClicked: inspectorRoot.effectTab = 4 }
                    }
                    Text {
                        text: "Chroma"
                        color: inspectorRoot.effectTab === 5 ? "#2792c3" : "#8a9ba3"
                        font.pixelSize: 11
                        MouseArea { anchors.fill: parent; onClicked: inspectorRoot.effectTab = 5 }
                    }
                    
                    Item { Layout.preferredWidth: 8 }
                }
            }
        }

        // 3. Stacked Effects Content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: inspectorRoot.effectTab

            BasicTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
                property var effectController: inspectorRoot.effectController
            }
            ColorTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
                property var effectController: inspectorRoot.effectController
            }
            BlurTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
                property var effectController: inspectorRoot.effectController
            }
            StylizeTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
                property var effectController: inspectorRoot.effectController
            }
            AudioTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
                property var effectController: inspectorRoot.effectController
            }
            ChromaTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
                property var effectController: inspectorRoot.effectController
            }
        }
    }
}
