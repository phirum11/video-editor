pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Rectangle {
    id: aiAssistRoot

    property color panelTop: Theme.surface
    property color panelBody: Theme.background
    property color panelLine: Theme.dividerSoft
    property color textPrimary: Theme.text
    property color textMuted: Theme.textMuted
    property color accent: Theme.accent

    color: panelBody
    clip: true

    ListModel {
        id: aiToolsModel

        ListElement {
            title: "Auto-Transcribe"
            detail: "Create subtitles from clip audio"
        }
        ListElement {
            title: "Import SRT"
            detail: "Parse subtitle files into captions"
        }
        ListElement {
            title: "AI Video Dubber"
            detail: "Generate translated voice tracks"
        }
        ListElement {
            title: "Remove Silence"
            detail: "Find and trim quiet gaps"
        }
        ListElement {
            title: "Auto Color Match"
            detail: "Balance clips to a reference look"
        }
        ListElement {
            title: "Subject Tracking"
            detail: "Track people or objects across frames"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 46
            color: aiAssistRoot.panelTop

            Rectangle {
                anchors.fill: parent
                anchors.margins: 10
                radius: 3
                color: Theme.surfaceInset
                border.color: searchInput.activeFocus ? aiAssistRoot.accent : Theme.dividerSoft
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
                    placeholderText: qsTr("Search AI tools")
                    color: aiAssistRoot.textPrimary
                    placeholderTextColor: Theme.textMuted
                    font.pixelSize: 13
                    selectByMouse: true
                    background: Item {}
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: aiAssistRoot.panelLine
            border.width: 1
            clip: true

            GridView {
                id: aiToolsGrid
                anchors.fill: parent
                anchors.margins: 12
                cellWidth: Math.max(132, Math.floor(width / Math.max(1, Math.floor(width / 160))))
                cellHeight: 96
                model: aiToolsModel
                clip: true

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                delegate: AbstractButton {
                    id: aiToolButton
                    required property string title
                    required property string detail

                    width: aiToolsGrid.cellWidth - 8
                    height: 84
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 6
                        color: aiToolButton.hovered ? Theme.surfaceHover : Theme.surfaceRaised
                        border.color: aiToolButton.hovered ? aiAssistRoot.accent : "#2b3f49"
                        border.width: 1
                    }

                    contentItem: ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                radius: 4
                                color: "#102831"
                                border.color: aiToolButton.hovered ? "#86c7e8" : "#345769"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "AI"
                                    color: aiAssistRoot.accent
                                    font.pixelSize: 10
                                    font.weight: Font.DemiBold
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: aiToolButton.title
                                color: aiToolButton.hovered ? Theme.text : aiAssistRoot.textPrimary
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: aiToolButton.detail
                            color: aiAssistRoot.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
