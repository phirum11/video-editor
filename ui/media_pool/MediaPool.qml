pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import VideoStudioUI

Rectangle {
    id: mediaPoolRoot

    property color panelTop: Theme.surface
    property color panelBody: Theme.background
    property color panelLine: Theme.divider
    property alias mediaPoolController: mediaPoolController
    property color textPrimary: Theme.text
    property color textMuted: Theme.textMuted
    property color accent: Theme.accent

    color: mediaPoolRoot.panelBody
    border.color: mediaPoolRoot.panelLine
    border.width: 1
    clip: true

    property int currentTab: 0
    property var selectedMediaIndices: []
    property string viewMode: "grid"
    property real thumbnailZoom: 0.5
    property var addedMediaPaths: ({})

    signal mediaActivated(string name, string filePath, real duration, bool hasVideo, bool hasAudio)
    signal mediaDeleted(string filePath)
    signal effectActivated(string name, string filePath)

    function deleteSelectedMedia() {
        if (selectedMediaIndices.length === 0)
            return

        // Sort descending so indices don't shift when deleting
        const sortedIndices = selectedMediaIndices.slice().sort((a, b) => b - a)
        for (let i = 0; i < sortedIndices.length; i++) {
            mediaPoolController.removeMediaAt(sortedIndices[i])
        }
        selectedMediaIndices = []
    }

    function markMediaAdded(filePath) {
        const nextPaths = {}
        for (const path in addedMediaPaths)
            nextPaths[path] = addedMediaPaths[path]
        nextPaths[filePath] = true
        addedMediaPaths = nextPaths
    }

    function forgetMediaAdded(filePath) {
        const nextPaths = {}
        for (const path in addedMediaPaths) {
            if (path !== filePath)
                nextPaths[path] = addedMediaPaths[path]
        }
        addedMediaPaths = nextPaths
    }

    function isMediaAdded(filePath) {
        return addedMediaPaths[filePath] === true
    }

    function activateMedia(index, name, filePath, duration, hasVideo, hasAudio) {
        if (!selectedMediaIndices.includes(index)) {
            selectedMediaIndices = [index]
        }
        markMediaAdded(filePath)
        mediaActivated(name, filePath, duration, hasVideo, hasAudio)
    }

    function formatDuration(seconds) {
        const safeSeconds = Number.isFinite(seconds) && seconds > 0 ? Math.floor(seconds) : 0
        const secs = safeSeconds % 60
        const mins = Math.floor(safeSeconds / 60) % 60
        const hours = Math.floor(safeSeconds / 3600)
        return hours > 0
            ? String(hours).padStart(2, "0") + ":" + String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0")
            : String(mins).padStart(2, "0") + ":" + String(secs).padStart(2, "0")
    }

    MediaPoolController {
        id: mediaPoolController
        onMediaRemoved: function(filePath) {
            mediaPoolRoot.forgetMediaAdded(filePath)
            mediaPoolRoot.mediaDeleted(filePath)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: Theme.background

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 1
                anchors.rightMargin: 1
                spacing: 0

                TabButton {
                    text: qsTr("Project Browser")
                    checked: mediaPoolRoot.currentTab === 0
                    onClicked: mediaPoolRoot.currentTab = 0
                }

                TabButton {
                    text: qsTr("Effect Hub")
                    checked: mediaPoolRoot.currentTab === 1
                    onClicked: mediaPoolRoot.currentTab = 1
                }



                Item {
                    Layout.fillWidth: true
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: mediaPoolRoot.currentTab

            // Tab 0: Project Browser
            ColumnLayout {
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 68
                    color: mediaPoolRoot.panelTop

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        anchors.topMargin: 6
                        anchors.bottomMargin: 7
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 7

                            Image {
                                Layout.preferredWidth: 16
                                Layout.preferredHeight: 16
                                source: "qrc:/VideoStudioUI/assets/folder.svg"
                                opacity: 0.85
                            }

                            Text {
                                Layout.fillWidth: true
                                text: qsTr("No project loaded")
                                color: mediaPoolRoot.textMuted
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 26
                                radius: 3
                                color: Theme.surfaceInset
                                border.color: searchInput.activeFocus ? mediaPoolRoot.accent : Theme.dividerSoft
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
                                    placeholderText: qsTr("Search")
                                    color: mediaPoolRoot.textPrimary
                                    placeholderTextColor: Theme.textMuted
                                    font.pixelSize: 13
                                    selectByMouse: true
                                    background: Item {}
                                    onTextChanged: mediaPoolController.searchQuery = text
                                }
                            }

                            AbstractButton {
                                id: deleteButton
                                text: qsTr("Delete")
                                implicitWidth: 64
                                implicitHeight: 26
                                enabled: mediaPoolRoot.selectedMediaIndices.length > 0
                                hoverEnabled: enabled
                                opacity: enabled ? 1 : 0.42

                                HoverHandler { cursorShape: Qt.PointingHandCursor }

                                background: Rectangle {
                                    radius: 3
                                    color: deleteButton.hovered ? "#3a2528" : "transparent"
                                    border.color: deleteButton.hovered ? "#8c4a55" : "transparent"
                                    border.width: 1
                                }

                                contentItem: Text {
                                    text: deleteButton.text
                                    color: deleteButton.hovered ? "#ffdce1" : mediaPoolRoot.textMuted
                                    font.pixelSize: 13
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: mediaPoolRoot.deleteSelectedMedia()
                            }

                            AbstractButton {
                                id: importButton
                                text: qsTr("Import")
                                implicitWidth: 72
                                implicitHeight: 26
                                hoverEnabled: true

                                HoverHandler { cursorShape: Qt.PointingHandCursor }

                                background: Rectangle {
                                    radius: 3
                                    color: importButton.hovered ? "#293b44" : "transparent"
                                    border.color: importButton.hovered ? "#486a7d" : "transparent"
                                    border.width: 1
                                }

                                contentItem: Text {
                                    text: importButton.text
                                    color: importButton.hovered ? mediaPoolRoot.textPrimary : mediaPoolRoot.textMuted
                                    font.pixelSize: 13
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: mediaPoolController.importMediaFiles()
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: mediaPoolRoot.panelBody
                    border.color: Theme.dividerSoft
                    border.width: 1
                    clip: true

                    DropArea {
                        anchors.fill: parent
                        onDropped: function(drop) {
                            if (!drop.urls || drop.urls.length === 0)
                                return

                            for (let i = 0; i < drop.urls.length; ++i)
                                mediaPoolController.importMediaFileUrl(drop.urls[i])

                            drop.acceptProposedAction()
                        }
                    }

                    GridView {
                        id: mediaGrid
                        anchors.fill: parent
                        topMargin: 12
                        leftMargin: 12
                        rightMargin: 12
                        bottomMargin: 12
                        interactive: false // Disabled so Flickable doesn't steal drags from rubber band
                        z: 1

                        // Smooth animated scrolling (like CapCut)
                        Behavior on contentY {
                            SmoothedAnimation {
                                velocity: 1500
                                duration: -1
                            }
                        }

                        // Mouse wheel scrolling since interactive is false
                        WheelHandler {
                            onWheel: (event) => {
                                // Use actual wheel delta for proportional, natural feel
                                let pixelDelta = event.angleDelta.y * 2.5
                                let target = mediaGrid.contentY - pixelDelta
                                let minY = mediaGrid.originY
                                let maxY = mediaGrid.originY + mediaGrid.contentHeight - mediaGrid.height
                                mediaGrid.contentY = Math.max(minY, Math.min(target, maxY))
                            }
                        }

                        MouseArea {
                            id: rubberBandArea
                            parent: mediaGrid
                            anchors.fill: mediaGrid
                            z: -1
                            preventStealing: true
                            
                            // startPos stored in CONTENT coordinates (not viewport)
                            property point startContentPos: Qt.point(0,0)
                            property bool isDragging: false
                            property var initialSelection: []
                            property real lastMouseX: 0
                            property real lastMouseY: 0

                            // Helper: compute selection rect in CONTENT coordinates
                            function contentSelectionRect() {
                                let curContentX = lastMouseX + mediaGrid.contentX
                                let curContentY = lastMouseY + mediaGrid.contentY
                                return Qt.rect(
                                    Math.min(startContentPos.x, curContentX),
                                    Math.min(startContentPos.y, curContentY),
                                    Math.abs(curContentX - startContentPos.x),
                                    Math.abs(curContentY - startContentPos.y)
                                )
                            }

                            // Helper: update visual rubber band (viewport coordinates)
                            function updateRubberBandVisual() {
                                let cRect = contentSelectionRect()
                                rubberBand.x = cRect.x - mediaGrid.contentX
                                rubberBand.y = cRect.y - mediaGrid.contentY
                                rubberBand.width = cRect.width
                                rubberBand.height = cRect.height
                            }

                            // Helper: run hit-test on items (content coordinates)
                            function updateSelection() {
                                let cRect = contentSelectionRect()
                                let selected = initialSelection.slice()
                                let itemsPerRow = Math.max(1, Math.floor(mediaGrid.width / mediaGrid.cellWidth))
                                for (let i = 0; i < mediaGrid.count; i++) {
                                    let col = i % itemsPerRow
                                    let row = Math.floor(i / itemsPerRow)
                                    // Item positions in content coordinates (no offset needed)
                                    let itemX = col * mediaGrid.cellWidth
                                    let itemY = row * mediaGrid.cellHeight
                                    let itemRect = Qt.rect(itemX, itemY, mediaGrid.cellWidth, mediaGrid.cellHeight)
                                    
                                    if (cRect.x < itemRect.x + itemRect.width &&
                                        cRect.x + cRect.width > itemRect.x &&
                                        cRect.y < itemRect.y + itemRect.height &&
                                        cRect.y + cRect.height > itemRect.y) {
                                        if (!selected.includes(i)) selected.push(i)
                                    }
                                }
                                mediaPoolRoot.selectedMediaIndices = selected
                            }

                            // Auto-scroll timer: when dragging near edges, auto-scroll the grid
                            Timer {
                                id: autoScrollTimer
                                interval: 16 // ~60fps
                                repeat: true
                                running: rubberBandArea.isDragging
                                onTriggered: {
                                    let edgeZone = 40
                                    let mouseY = rubberBandArea.lastMouseY
                                    let scrollSpeed = 0

                                    if (mouseY < edgeZone) {
                                        scrollSpeed = -Math.round((edgeZone - mouseY) * 0.3)
                                    } else if (mouseY > mediaGrid.height - edgeZone) {
                                        scrollSpeed = Math.round((mouseY - (mediaGrid.height - edgeZone)) * 0.3)
                                    }

                                    if (scrollSpeed !== 0) {
                                        let minY = mediaGrid.originY
                                        let maxY = mediaGrid.originY + mediaGrid.contentHeight - mediaGrid.height
                                        mediaGrid.contentY = Math.max(minY, Math.min(mediaGrid.contentY + scrollSpeed, maxY))
                                        rubberBandArea.updateRubberBandVisual()
                                        rubberBandArea.updateSelection()
                                    }
                                }
                            }
                            
                            onPressed: (mouse) => {
                                // Store start position in CONTENT coordinates
                                startContentPos = Qt.point(mouse.x + mediaGrid.contentX, mouse.y + mediaGrid.contentY)
                                lastMouseX = mouse.x
                                lastMouseY = mouse.y
                                isDragging = true
                                rubberBand.x = mouse.x
                                rubberBand.y = mouse.y
                                rubberBand.width = 0
                                rubberBand.height = 0
                                rubberBand.visible = true
                                if (mouse.modifiers & Qt.ControlModifier) {
                                    initialSelection = mediaPoolRoot.selectedMediaIndices.slice()
                                } else {
                                    initialSelection = []
                                    mediaPoolRoot.selectedMediaIndices = []
                                }
                            }
                            onPositionChanged: (mouse) => {
                                if (!isDragging) return
                                lastMouseX = mouse.x
                                lastMouseY = mouse.y
                                updateRubberBandVisual()
                                updateSelection()
                            }
                            onReleased: {
                                isDragging = false
                                rubberBand.visible = false
                            }
                            onCanceled: {
                                isDragging = false
                                rubberBand.visible = false
                            }
                        }

                        Rectangle {
                            id: rubberBand
                            parent: rubberBandArea
                            visible: false
                            color: Qt.rgba(mediaPoolRoot.accent.r, mediaPoolRoot.accent.g, mediaPoolRoot.accent.b, 0.2)
                            border.color: mediaPoolRoot.accent
                            border.width: 1
                            z: 9999
                        }

                        cellWidth: mediaPoolRoot.viewMode === "list"
                            ? Math.max(120, width - 16)
                            : Math.round(104 + mediaPoolRoot.thumbnailZoom * 120)
                        cellHeight: mediaPoolRoot.viewMode === "list"
                            ? 64
                            : Math.round(96 + mediaPoolRoot.thumbnailZoom * 120)
                        
                        model: mediaPoolController.mediaModel
                        clip: true

                        ScrollBar.vertical: ScrollBar {
                            policy: mediaGrid.contentHeight > mediaGrid.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                            active: true
                            interactive: true
                        }

                        visible: count > 0
                        onCountChanged: {
                            if (count === 0)
                                mediaPoolRoot.selectedMediaIndices = []
                            else {
                                // Filter out invalid indices
                                const validIndices = mediaPoolRoot.selectedMediaIndices.filter(idx => idx < count)
                                mediaPoolRoot.selectedMediaIndices = validIndices
                            }
                        }

                        delegate: Item {
                            id: mediaDelegate
                            required property int index
                            required property string name
                            required property string filePath
                            required property real duration
                            required property bool hasVideo
                            required property bool hasAudio

                            readonly property string mediaName: name
                            readonly property string mediaFilePath: filePath
                            readonly property real mediaDuration: duration
                            readonly property bool mediaHasVideo: hasVideo
                            readonly property bool mediaHasAudio: hasAudio
                            property bool isRenaming: false
                            readonly property bool listMode: mediaPoolRoot.viewMode === "list"
                            readonly property bool addedToTimeline: mediaPoolRoot.isMediaAdded(filePath)
                            readonly property bool cardHovered: delegateHover.hovered || dragMouse.containsMouse || quickAddButton.hovered
                            readonly property real thumbnailWidth: listMode
                                ? 72
                                : Math.max(84, mediaGrid.cellWidth - 20)
                            readonly property real thumbnailHeight: listMode
                                ? 44
                                : Math.max(52, mediaGrid.cellHeight - 54)

                            width: mediaGrid.cellWidth
                            height: mediaGrid.cellHeight
                            z: dragMouse.drag.active ? 10 : 0

                            HoverHandler {
                                id: delegateHover
                            }

                                Rectangle {
                                id: thumbnail
                                x: mediaDelegate.listMode ? 8 : (parent.width - width) / 2
                                y: mediaDelegate.listMode ? 8 : 0
                                width: mediaDelegate.thumbnailWidth
                                height: mediaDelegate.thumbnailHeight
                                radius: 6
                                color: mediaDelegate.cardHovered ? "#253139" : "#20292f"
                                clip: true

                                Image {
                                    id: thumbImage
                                    anchors.fill: parent
                                    source: mediaDelegate.filePath.toLowerCase().endsWith(".srt") ? "" : ("image://media/" + encodeURIComponent(mediaDelegate.filePath))
                                    sourceSize.width: Math.ceil(thumbnail.width)
                                    sourceSize.height: Math.ceil(thumbnail.height)
                                    asynchronous: true
                                    fillMode: Image.PreserveAspectCrop
                                    visible: false
                                }

                                Rectangle {
                                    id: maskRect
                                    anchors.fill: parent
                                    radius: 6
                                    color: "black"
                                    visible: false
                                    layer.enabled: true
                                }

                                MultiEffect {
                                    anchors.fill: thumbImage
                                    source: thumbImage
                                    maskEnabled: true
                                    maskSource: maskRect
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    color: "transparent"
                                    border.color: mediaPoolRoot.selectedMediaIndices.includes(mediaDelegate.index)
                                        ? mediaPoolRoot.accent
                                        : (mediaDelegate.cardHovered ? "#435b66" : "#000000")
                                    border.width: mediaPoolRoot.selectedMediaIndices.includes(mediaDelegate.index) ? 3 : 1
                                    opacity: mediaPoolRoot.selectedMediaIndices.includes(mediaDelegate.index) ? 1.0 : (mediaDelegate.cardHovered ? 1.0 : 0.2)
                                    radius: parent.radius
                                }

                                // SRT file placeholder icon
                                Text {
                                    anchors.centerIn: parent
                                    text: "📝"
                                    font.pixelSize: 24
                                    visible: mediaDelegate.filePath.toLowerCase().endsWith(".srt")
                                }
                            }

                            Item {
                                id: dragTargetDummy
                            }

                            Rectangle {
                                id: dragProxy
                                readonly property var sourceMedia: mediaDelegate
                                readonly property string mediaName: mediaDelegate.mediaName
                                readonly property string mediaFilePath: mediaDelegate.mediaFilePath
                                readonly property real mediaDuration: mediaDelegate.mediaDuration
                                readonly property bool mediaHasVideo: mediaDelegate.mediaHasVideo
                                readonly property bool mediaHasAudio: mediaDelegate.mediaHasAudio

                                property point startOverlayPos: Qt.point(0, 0)
                                property bool isDragging: false
                                property var selectedMediaList: []

                                onIsDraggingChanged: {
                                    if (isDragging) {
                                        let list = []
                                        let indices = mediaPoolRoot.selectedMediaIndices.slice().sort((a,b) => a - b)
                                        if (indices.length === 0 || !indices.includes(mediaDelegate.index)) {
                                            indices = [mediaDelegate.index]
                                        }
                                        for (let i = 0; i < indices.length; i++) {
                                            let info = mediaPoolController.getMediaAt(indices[i])
                                            list.push(info)
                                        }
                                        selectedMediaList = list
                                    }
                                }

                                parent: isDragging ? Overlay.overlay : mediaDelegate
                                x: isDragging ? startOverlayPos.x + dragTargetDummy.x : thumbnail.x
                                y: isDragging ? startOverlayPos.y + dragTargetDummy.y : thumbnail.y
                                width: thumbnail.width
                                height: thumbnail.height
                                radius: 2
                                visible: isDragging
                                opacity: 0.82
                                color: "#12242d"
                                border.color: mediaPoolRoot.accent
                                border.width: 1
                                z: 99999

                                Drag.active: isDragging
                                Drag.source: dragProxy
                                Drag.keys: ["videoStudio/media"]
                                Drag.supportedActions: Qt.CopyAction
                                Drag.hotSpot.x: width / 2
                                Drag.hotSpot.y: height / 2

                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 1
                                    source: mediaDelegate.filePath.toLowerCase().endsWith(".srt") ? "" : ("image://media/" + encodeURIComponent(mediaDelegate.filePath))
                                    sourceSize.width: Math.ceil(thumbnail.width)
                                    sourceSize.height: Math.ceil(thumbnail.height)
                                    fillMode: Image.PreserveAspectCrop
                                }

                                // Count badge (like CapCut) — shows number of selected items
                                Rectangle {
                                    id: dragCountBadge
                                    x: parent.width - width / 2
                                    y: -height / 2
                                    width: Math.max(22, dragCountText.implicitWidth + 10)
                                    height: 22
                                    radius: 11
                                    color: mediaPoolRoot.accent
                                    visible: dragProxy.selectedMediaList.length > 1
                                    z: 100

                                    Text {
                                        id: dragCountText
                                        anchors.centerIn: parent
                                        text: dragProxy.selectedMediaList.length
                                        color: "#ffffff"
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                }
                            }

                            Rectangle {
                                id: addedBadge
                                x: thumbnail.x + 5
                                y: thumbnail.y + 5
                                width: addedBadgeLabel.implicitWidth + 10
                                height: 18
                                radius: 3
                                visible: mediaDelegate.addedToTimeline
                                color: "#111417"
                                z: 4

                                Text {
                                    id: addedBadgeLabel
                                    anchors.centerIn: parent
                                    text: qsTr("Added")
                                    color: "#e8f2f4"
                                    font.pixelSize: 11
                                }
                            }

                            Rectangle {
                                id: durationBadge
                                x: thumbnail.x + thumbnail.width - width - 5
                                y: thumbnail.y + 5
                                width: durationLabel.implicitWidth + 10
                                height: 18
                                radius: 3
                                visible: mediaDelegate.mediaDuration > 0
                                color: "#000000"
                                opacity: 0.72
                                z: 4

                                Text {
                                    id: durationLabel
                                    anchors.centerIn: parent
                                    text: mediaPoolRoot.formatDuration(mediaDelegate.mediaDuration)
                                    color: Theme.text
                                    font.pixelSize: 11
                                }
                            }

                            AbstractButton {
                                id: quickAddButton
                                x: thumbnail.x + thumbnail.width - width - 7
                                y: thumbnail.y + thumbnail.height - height - 7
                                width: 22
                                height: 22
                                visible: mediaDelegate.cardHovered && !dragMouse.drag.active
                                hoverEnabled: true
                                z: 5

                                HoverHandler { cursorShape: Qt.PointingHandCursor }

                                background: Rectangle {
                                    radius: width / 2
                                    color: quickAddButton.pressed ? "#11aebf" : "#17c9d7"
                                    border.color: "#adfbff"
                                    border.width: quickAddButton.hovered ? 1 : 0
                                }

                                contentItem: Text {
                                    text: "+"
                                    color: Theme.text
                                    font.pixelSize: 18
                                    font.weight: Font.DemiBold
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: mediaPoolRoot.activateMedia(
                                    mediaDelegate.index,
                                    mediaDelegate.mediaName,
                                    mediaDelegate.mediaFilePath,
                                    mediaDelegate.mediaDuration,
                                    mediaDelegate.mediaHasVideo,
                                    mediaDelegate.mediaHasAudio
                                )
                            }

                            Text {
                                id: nameText
                                x: mediaDelegate.listMode ? thumbnail.x + thumbnail.width + 10 : thumbnail.x
                                y: mediaDelegate.listMode
                                    ? Math.round((parent.height - height) / 2)
                                    : thumbnail.y + thumbnail.height + 8
                                width: mediaDelegate.listMode
                                    ? parent.width - x - 10
                                    : thumbnail.width
                                height: 18
                                text: mediaDelegate.name
                                color: mediaPoolRoot.textPrimary   
                                font.pixelSize: 12     
                                horizontalAlignment: mediaDelegate.listMode ? Text.AlignLeft : Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter    
                                elide: Text.ElideRight
                                visible: !mediaDelegate.isRenaming    
                            }

                            TextField {    
                                id: renameInput
                                x: nameText.x - 4
                                y: nameText.y - 2    
                                width: nameText.width + 8
                                height: nameText.height + 4
                                text: mediaDelegate.name    
                                visible: mediaDelegate.isRenaming
                                color: mediaPoolRoot.textPrimary
                                font.pixelSize: 12    
                                horizontalAlignment: mediaDelegate.listMode ? TextInput.AlignLeft : TextInput.AlignHCenter
                                verticalAlignment: TextInput.AlignVCenter
                                background: Rectangle {
                                    color: "#1a1a1a"    
                                    border.color: mediaPoolRoot.accent
                                    border.width: 1   
                                    radius: 2
                                }     
                                onEditingFinished: {
                                    if (mediaDelegate.isRenaming) {
                                        mediaPoolController.renameMediaAt(mediaDelegate.index, text)
                                        mediaDelegate.isRenaming = false
                                    }
                                }
                                onActiveFocusChanged: {     
                                    if (!activeFocus && mediaDelegate.isRenaming) {
                                        mediaPoolController.renameMediaAt(mediaDelegate.index, text)
                                        mediaDelegate.isRenaming = false
                                    }
                                }
                            }

                            Menu {
                                id: contextMenu
                                
                                background: Rectangle {
                                    implicitWidth: 180
                                    color: "#282828"
                                    radius: 6
                                    border.color: Theme.surfaceHover
                                    border.width: 1
                                }
                                
                                MenuItem {
                                    id: itemOpen
                                    text: "Open file location"
                                    implicitHeight: 32
                                    contentItem: Text {
                                        leftPadding: 16
                                        text: itemOpen.text
                                        color: itemOpen.highlighted ? Theme.text : Theme.textMuted
                                        font.pixelSize: 13
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: itemOpen.highlighted ? Theme.surfaceHover : "transparent"
                                        radius: 4
                                        anchors.fill: parent
                                        anchors.margins: 4
                                    }
                                    onTriggered: mediaPoolController.openFileLocation(mediaDelegate.index)
                                }
                                
                                MenuItem {
                                    id: itemRename
                                    text: "Rename"
                                    implicitHeight: 32
                                    contentItem: Text {
                                        leftPadding: 16
                                        text: itemRename.text
                                        color: itemRename.highlighted ? Theme.text : Theme.textMuted
                                        font.pixelSize: 13
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: itemRename.highlighted ? Theme.surfaceHover : "transparent"
                                        radius: 4
                                        anchors.fill: parent
                                        anchors.margins: 4
                                    }
                                    onTriggered: {
                                        mediaDelegate.isRenaming = true
                                        renameInput.forceActiveFocus()
                                    }
                                }
                                
                                MenuItem {
                                    id: itemDelete
                                    text: "Delete"
                                    implicitHeight: 32
                                    contentItem: Text {
                                        leftPadding: 16
                                        text: itemDelete.text
                                        color: itemDelete.highlighted ? Theme.text : Theme.textMuted
                                        font.pixelSize: 13
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: itemDelete.highlighted ? Theme.surfaceHover : "transparent"
                                        radius: 4
                                        anchors.fill: parent
                                        anchors.margins: 4
                                    }
                                    onTriggered: mediaPoolRoot.deleteSelectedMedia()
                                }
                            }

                            MouseArea {
                                id: dragMouse
                                x: thumbnail.x
                                y: thumbnail.y
                                width: mediaDelegate.listMode ? (nameText.x + nameText.width - thumbnail.x) : thumbnail.width
                                height: mediaDelegate.listMode ? thumbnail.height : (nameText.y + nameText.height - thumbnail.y)
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                drag.target: dragTargetDummy
                                drag.threshold: 8

                                onPressed: (mouse) => {
                                    if (!mediaPoolRoot.selectedMediaIndices.includes(mediaDelegate.index)) {
                                        if (mouse.modifiers & Qt.ControlModifier) {
                                            mediaPoolRoot.selectedMediaIndices = mediaPoolRoot.selectedMediaIndices.concat([mediaDelegate.index])
                                        } else {
                                            mediaPoolRoot.selectedMediaIndices = [mediaDelegate.index]
                                        }
                                    }
                                }
                                onPositionChanged: {
                                    if (drag.active && !dragProxy.isDragging) {
                                        dragProxy.startOverlayPos = thumbnail.mapToItem(Overlay.overlay, 0, 0)
                                        dragProxy.isDragging = true
                                    }
                                }
                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.RightButton) {
                                        if (!mediaPoolRoot.selectedMediaIndices.includes(mediaDelegate.index)) {
                                            mediaPoolRoot.selectedMediaIndices = [mediaDelegate.index]
                                        }
                                        contextMenu.popup()
                                    } else {
                                        if (mouse.modifiers & Qt.ControlModifier) {
                                            let current = mediaPoolRoot.selectedMediaIndices.slice()
                                            let pos = current.indexOf(mediaDelegate.index)
                                            if (pos === -1) current.push(mediaDelegate.index)
                                            else current.splice(pos, 1)
                                            mediaPoolRoot.selectedMediaIndices = current
                                        } else {
                                            mediaPoolRoot.selectedMediaIndices = [mediaDelegate.index]
                                        }
                                    }
                                }
                                onDoubleClicked: mediaPoolRoot.activateMedia(
                                    mediaDelegate.index,
                                    mediaDelegate.mediaName,
                                    mediaDelegate.mediaFilePath,
                                    mediaDelegate.mediaDuration,
                                    mediaDelegate.mediaHasVideo,
                                    mediaDelegate.mediaHasAudio
                                )
                                onReleased: {
                                    if (dragProxy.isDragging) {
                                        dragProxy.Drag.drop()
                                    }
                                    dragProxy.isDragging = false
                                    dragTargetDummy.x = 0
                                    dragTargetDummy.y = 0
                                }
                                onCanceled: {
                                    dragProxy.isDragging = false
                                    dragTargetDummy.x = 0
                                    dragTargetDummy.y = 0
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 48, 230)
                        spacing: 10
                        visible: mediaGrid.count === 0

                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 38
                            Layout.preferredHeight: 38
                            source: "qrc:/VideoStudioUI/assets/folder.svg"
                            opacity: 0.32
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("No media imported")
                            color: Theme.textMuted
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                MediaPoolFooter {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    viewMode: mediaPoolRoot.viewMode
                    zoomValue: mediaPoolRoot.thumbnailZoom
                    panelLine: Theme.divider
                    textPrimary: mediaPoolRoot.textPrimary
                    textMuted: mediaPoolRoot.textMuted
                    accent: mediaPoolRoot.accent
                    onViewModeRequested: function(mode) { mediaPoolRoot.viewMode = mode }
                    onZoomValueRequested: function(value) { mediaPoolRoot.thumbnailZoom = value }
                }
            }

            // Tab 1: Effect Hub
            EffectHub {
                Layout.fillWidth: true
                Layout.fillHeight: true
                onEffectActivated: function(name, filePath) {
                    mediaPoolRoot.effectActivated(name, filePath)
                }
            }


        }
    }
    component TabButton: AbstractButton {
        id: tab
        implicitWidth: Math.max(tabLabel.implicitWidth + 20, 82)
        implicitHeight: 32
        Layout.minimumWidth: 62
        Layout.preferredWidth: implicitWidth
        hoverEnabled: true

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        background: Rectangle {
            color: tab.checked ? mediaPoolRoot.panelTop : tab.hovered ? Theme.surfaceHover : Theme.background
            border.color: tab.checked ? mediaPoolRoot.panelLine : "transparent"
            border.width: tab.checked ? 1 : 0
        }

        contentItem: Text {
            id: tabLabel
            text: tab.text
            color: tab.checked ? mediaPoolRoot.textPrimary : "#89979d"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

}
