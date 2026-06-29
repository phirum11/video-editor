// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI

Menu {
    id: rootMenu
    
    property int clipIndex: -1
    property var timelineController: null
    property int vocalIsolationType: 0
    property bool clipIsMuted: false
    property bool clipHasAudio: false
    
    width: 220
    topPadding: 4
    bottomPadding: 4

    background: Rectangle {
        color: Theme.surfaceInset
        border.color: Theme.divider
        border.width: 1
        radius: 4
    }

    component ContextMenuItem: MenuItem {
        id: menuItem
        property bool isCheckable: false
        property bool isChecked: false
        property string shortcutText: ""
        
        implicitHeight: 32
        
        background: Rectangle {
            implicitHeight: 32
            x: 2; y: 1
            width: parent.width - 4
            height: parent.height - 2
            color: menuItem.highlighted ? "#2792c3" : "transparent"
            radius: 3
        }
        
        contentItem: Item {
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10
                
                // Checkmark symbol for checkable items
                Text {
                    text: "✓"
                    color: "#5ec4e8"
                    font.pixelSize: 14
                    visible: menuItem.isCheckable && menuItem.isChecked
                    Layout.preferredWidth: 14
                }
                
                // Placeholder space if checkable but not checked
                Item {
                    visible: menuItem.isCheckable && !menuItem.isChecked
                    Layout.preferredWidth: 14
                }
                
                Text {
                    text: menuItem.text
                    color: menuItem.highlighted ? Theme.text : "#dce4e7"
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }
                
                Text {
                    text: menuItem.shortcutText
                    color: menuItem.highlighted ? Theme.text : "#6a7b83"
                    font.pixelSize: 12
                    visible: menuItem.shortcutText.length > 0
                }
                
                Text {
                    text: ">"
                    color: menuItem.highlighted ? Theme.text : "#6a7b83"
                    font.pixelSize: 12
                    visible: menuItem.subMenu !== null
                }
            }
        }
        
        onClicked: {
            if (isCheckable) {
                // If this is a radio-group like behavior, we handle it where instantiated
            }
        }
    }

    delegate: ContextMenuItem {}

    ContextMenuItem {
        text: "Remove"
        onClicked: {
            if (rootMenu.timelineController && rootMenu.clipIndex >= 0) {
                let sel = rootMenu.timelineController.selectedClipIndices
                let isMultiSelected = false
                for (let i = 0; i < sel.length; ++i) {
                    if (Number(sel[i]) === Number(rootMenu.clipIndex)) {
                        isMultiSelected = true
                        break
                    }
                }
                isMultiSelected = isMultiSelected && sel.length > 1
                
                if (isMultiSelected) {
                    rootMenu.timelineController.deleteSelectedClips()
                } else {
                    rootMenu.timelineController.deleteLinkedClip(rootMenu.clipIndex)
                }
            }
        }
    }

    ContextMenuItem {
        text: "Group"
        shortcutText: "Ctrl+G"
        visible: rootMenu.timelineController ? rootMenu.timelineController.selectedClipIndices.length > 1 : false
        height: visible ? implicitHeight : 0
        onClicked: {
            if (rootMenu.timelineController) {
                rootMenu.timelineController.groupSelectedClips()
            }
        }
    }

    ContextMenuItem {
        text: "Ungroup"
        shortcutText: "Ctrl+Shift+G"
        property bool canUngroup: {
            if (!rootMenu.timelineController) return false
            let sel = rootMenu.timelineController.selectedClipIndices
            for (let i = 0; i < sel.length; ++i) {
                let item = rootMenu.timelineController.clipAt(sel[i])
                if (item && item.groupId && item.groupId !== "") return true
            }
            return false
        }
        visible: canUngroup
        height: visible ? implicitHeight : 0
        onClicked: {
            if (rootMenu.timelineController) {
                rootMenu.timelineController.ungroupSelectedClips()
            }
        }
    }

    MenuSeparator {
        contentItem: Rectangle {
            implicitHeight: 1
            color: Theme.divider
        }
    }

    ContextMenuItem {
        text: rootMenu.clipIsMuted ? "Unmute" : "Mute"
        visible: rootMenu.clipHasAudio
        height: visible ? implicitHeight : 0
        onClicked: {
            if (rootMenu.timelineController && rootMenu.clipIndex >= 0) {
                rootMenu.timelineController.setClipMuted(rootMenu.clipIndex, !rootMenu.clipIsMuted)
            }
        }
    }

    MenuSeparator {
        visible: rootMenu.clipHasAudio
        height: visible ? implicitHeight : 0
        contentItem: Rectangle {
            implicitHeight: 1
            color: Theme.divider
        }
    }

    Menu {
        id: isolateVoiceMenu
        title: "Isolate voice"
        
        topPadding: 4
        bottomPadding: 4
        
        background: Rectangle {
            implicitWidth: 200
            color: Theme.surfaceInset
            border.color: Theme.divider
            border.width: 1
            radius: 4
        }
        
        delegate: ContextMenuItem {}
        
        ContextMenuItem {
            id: dontIsolateItem
            text: "Don't isolate voice"
            isCheckable: true
            isChecked: rootMenu.vocalIsolationType === 0
            onClicked: {
                if (rootMenu.timelineController && rootMenu.clipIndex >= 0) {
                    rootMenu.timelineController.setVocalIsolation(rootMenu.clipIndex, 0)
                }
            }
        }
        ContextMenuItem {
            id: removeVocalItem
            text: "Remove vocal"
            isCheckable: true
            isChecked: rootMenu.vocalIsolationType === 1
            onClicked: {
                if (rootMenu.timelineController && rootMenu.clipIndex >= 0) {
                    rootMenu.timelineController.setVocalIsolation(rootMenu.clipIndex, 1)
                }
            }
        }
        ContextMenuItem {
            id: keepVocalItem
            text: "Keep vocal"
            isCheckable: true
            isChecked: rootMenu.vocalIsolationType === 2
            onClicked: {
                if (rootMenu.timelineController && rootMenu.clipIndex >= 0) {
                    rootMenu.timelineController.setVocalIsolation(rootMenu.clipIndex, 2)
                }
            }
        }
    }
}
