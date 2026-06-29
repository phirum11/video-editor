pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import VideoStudioUI

Dialog {
    id: selectedCoverDialog
    property var timelineController: null
    property string selectedCoverPath: ""
    property real selectedTime: 0.1
    property string activeMode: "video" // "video" or "local"
    property string baseVideoPath: ""
    property string localImagePath: ""
    property real videoDuration: 10.0

    title: "Select a cover"
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 760
    height: 660

    function loadTimelineVideo() {
        if (!timelineController || timelineController.clipCount === 0) return
        for (let i = 0; i < timelineController.clipCount; ++i) {
            let c = timelineController.clipAt(i)
            if (c && c.hasVideo && c.filePath) {
                baseVideoPath = c.filePath
                videoDuration = c.sourceDuration > 0 ? c.sourceDuration : (c.durationSeconds > 0 ? c.durationSeconds : 10.0)
                selectedTime = 0.1
                return
            }
        }
    }

    onOpened: loadTimelineVideo()

    background: Rectangle {
        color: "#1c1c1e"
        border.color: Theme.divider
        border.width: 1
        radius: 8
    }

    header: Rectangle {
        color: "#1c1c1e"
        height: 44
        radius: 8

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "Select a cover"
            color: Theme.text
            font.pixelSize: 15
            font.weight: Font.DemiBold
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.divider
        }
    }

    FileDialog {
        id: coverFileDialog
        title: "Select Local Cover Image"
        currentFolder: "file:///C:/we_hunting"
        nameFilters: ["Image Files (*.png *.jpg *.jpeg *.webp *.bmp)", "All Files (*.*)"]
        onAccepted: {
            selectedCoverDialog.localImagePath = selectedFile.toString()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // Top Preview Area
        Rectangle {
            id: previewContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: selectedCoverDialog.activeMode === "video" ? "#000000" : "#222225"
            radius: 6
            clip: true

            // Video Mode Preview
            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                visible: selectedCoverDialog.activeMode === "video"
                source: baseVideoPath !== "" ? "image://media/" + encodeURIComponent(baseVideoPath + "|" + selectedCoverDialog.selectedTime) : ""
                asynchronous: true
            }

            // Local Mode Preview - When Image is Selected
            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                visible: selectedCoverDialog.activeMode === "local" && selectedCoverDialog.localImagePath !== ""
                source: selectedCoverDialog.localImagePath
                asynchronous: true
            }

            // Local Mode Placeholder - Click or drag area
            Rectangle {
                width: 360
                height: 240
                anchors.centerIn: parent
                visible: selectedCoverDialog.activeMode === "local" && selectedCoverDialog.localImagePath === ""
                color: "#2a2a2d"
                radius: 12
                border.color: "#3a3a3f"
                border.width: 2

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 16

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        width: 44
                        height: 44
                        radius: 22
                        color: "#25e5cf"

                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            color: "#ffffff"
                            font.pixelSize: 28
                            font.weight: Font.Light
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Click or drag image to this area"
                        color: Theme.text
                        font.pixelSize: 16
                        font.weight: Font.Medium
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "(recommended ratio 16:9)"
                        color: Theme.textMuted
                        font.pixelSize: 13
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: coverFileDialog.open()
                }
            }

            // Interactive Crop Box Overlay
            Item {
                id: cropOverlay
                anchors.fill: parent
                visible: selectedCoverDialog.activeMode === "video" || (selectedCoverDialog.activeMode === "local" && selectedCoverDialog.localImagePath !== "")

                property real cropX: parent.width * 0.15
                property real cropY: parent.height * 0.15
                property real cropWidth: parent.width * 0.7
                property real cropHeight: parent.height * 0.7

                onWidthChanged: {
                    if (width > 0) {
                        cropX = width * 0.15
                        cropWidth = width * 0.7
                    }
                }
                onHeightChanged: {
                    if (height > 0) {
                        cropY = height * 0.15
                        cropHeight = height * 0.7
                    }
                }

                // Darkened outer areas (Top, Bottom, Left, Right)
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: cropOverlay.cropY
                    color: "#99000000"
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: parent.height - (cropOverlay.cropY + cropOverlay.cropHeight)
                    color: "#99000000"
                }
                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: cropOverlay.cropY
                    anchors.left: parent.left
                    width: cropOverlay.cropX
                    height: cropOverlay.cropHeight
                    color: "#99000000"
                }
                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: cropOverlay.cropY
                    anchors.right: parent.right
                    width: parent.width - (cropOverlay.cropX + cropOverlay.cropWidth)
                    height: cropOverlay.cropHeight
                    color: "#99000000"
                }

                // Crop Box Frame
                Rectangle {
                    x: cropOverlay.cropX
                    y: cropOverlay.cropY
                    width: cropOverlay.cropWidth
                    height: cropOverlay.cropHeight
                    color: "transparent"
                    border.color: "#ffffff"
                    border.width: 1.5

                    // Drag area to move the crop box
                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: 10
                        cursorShape: Qt.SizeAllCursor
                        drag.target: null
                        property real startX: 0
                        property real startY: 0
                        onPressed: function(mouse) {
                            startX = mouse.x
                            startY = mouse.y
                        }
                        onPositionChanged: function(mouse) {
                            if (mouse.buttons & Qt.LeftButton) {
                                let dx = mouse.x - startX
                                let dy = mouse.y - startY
                                let newX = Math.max(0, Math.min(cropOverlay.width - cropOverlay.cropWidth, cropOverlay.cropX + dx))
                                let newY = Math.max(0, Math.min(cropOverlay.height - cropOverlay.cropHeight, cropOverlay.cropY + dy))
                                cropOverlay.cropX = newX
                                cropOverlay.cropY = newY
                            }
                        }
                    }
                }

                // Corner Handles
                // Top-Left Handle
                Rectangle {
                    x: cropOverlay.cropX - width / 2
                    y: cropOverlay.cropY - height / 2
                    width: 18
                    height: 18
                    radius: 9
                    color: "#ffffff"
                    border.color: "#000000"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -10
                        cursorShape: Qt.SizeFDiagCursor
                        property real startX: 0
                        property real startY: 0
                        onPressed: function(mouse) { startX = mouse.x; startY = mouse.y }
                        onPositionChanged: function(mouse) {
                            if (mouse.buttons & Qt.LeftButton) {
                                let dx = mouse.x - startX
                                let dy = mouse.y - startY
                                let newX = Math.max(0, Math.min(cropOverlay.cropX + cropOverlay.cropWidth - 50, cropOverlay.cropX + dx))
                                let newY = Math.max(0, Math.min(cropOverlay.cropY + cropOverlay.cropHeight - 50, cropOverlay.cropY + dy))
                                cropOverlay.cropWidth += (cropOverlay.cropX - newX)
                                cropOverlay.cropHeight += (cropOverlay.cropY - newY)
                                cropOverlay.cropX = newX
                                cropOverlay.cropY = newY
                            }
                        }
                    }
                }

                // Top-Right Handle
                Rectangle {
                    x: cropOverlay.cropX + cropOverlay.cropWidth - width / 2
                    y: cropOverlay.cropY - height / 2
                    width: 18
                    height: 18
                    radius: 9
                    color: "#ffffff"
                    border.color: "#000000"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -10
                        cursorShape: Qt.SizeBDiagCursor
                        property real startX: 0
                        property real startY: 0
                        onPressed: function(mouse) { startX = mouse.x; startY = mouse.y }
                        onPositionChanged: function(mouse) {
                            if (mouse.buttons & Qt.LeftButton) {
                                let dx = mouse.x - startX
                                let dy = mouse.y - startY
                                let newW = Math.max(50, Math.min(cropOverlay.width - cropOverlay.cropX, cropOverlay.cropWidth + dx))
                                let newY = Math.max(0, Math.min(cropOverlay.cropY + cropOverlay.cropHeight - 50, cropOverlay.cropY + dy))
                                cropOverlay.cropHeight += (cropOverlay.cropY - newY)
                                cropOverlay.cropWidth = newW
                                cropOverlay.cropY = newY
                            }
                        }
                    }
                }

                // Bottom-Left Handle
                Rectangle {
                    x: cropOverlay.cropX - width / 2
                    y: cropOverlay.cropY + cropOverlay.cropHeight - height / 2
                    width: 18
                    height: 18
                    radius: 9
                    color: "#ffffff"
                    border.color: "#000000"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -10
                        cursorShape: Qt.SizeBDiagCursor
                        property real startX: 0
                        property real startY: 0
                        onPressed: function(mouse) { startX = mouse.x; startY = mouse.y }
                        onPositionChanged: function(mouse) {
                            if (mouse.buttons & Qt.LeftButton) {
                                let dx = mouse.x - startX
                                let dy = mouse.y - startY
                                let newX = Math.max(0, Math.min(cropOverlay.cropX + cropOverlay.cropWidth - 50, cropOverlay.cropX + dx))
                                let newH = Math.max(50, Math.min(cropOverlay.height - cropOverlay.cropY, cropOverlay.cropHeight + dy))
                                cropOverlay.cropWidth += (cropOverlay.cropX - newX)
                                cropOverlay.cropHeight = newH
                                cropOverlay.cropX = newX
                            }
                        }
                    }
                }

                // Bottom-Right Handle
                Rectangle {
                    x: cropOverlay.cropX + cropOverlay.cropWidth - width / 2
                    y: cropOverlay.cropY + cropOverlay.cropHeight - height / 2
                    width: 18
                    height: 18
                    radius: 9
                    color: "#ffffff"
                    border.color: "#000000"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -10
                        cursorShape: Qt.SizeFDiagCursor
                        property real startX: 0
                        property real startY: 0
                        onPressed: function(mouse) { startX = mouse.x; startY = mouse.y }
                        onPositionChanged: function(mouse) {
                            if (mouse.buttons & Qt.LeftButton) {
                                let dx = mouse.x - startX
                                let dy = mouse.y - startY
                                let newW = Math.max(50, Math.min(cropOverlay.width - cropOverlay.cropX, cropOverlay.cropWidth + dx))
                                let newH = Math.max(50, Math.min(cropOverlay.height - cropOverlay.cropY, cropOverlay.cropHeight + dy))
                                cropOverlay.cropWidth = newW
                                cropOverlay.cropHeight = newH
                            }
                        }
                    }
                }
            }
        }

        // Mode Switcher (Select from video / Local)
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 0

            Rectangle {
                implicitWidth: 140
                implicitHeight: 32
                radius: 4
                color: selectedCoverDialog.activeMode === "video" ? "#303035" : "#141416"
                border.color: selectedCoverDialog.activeMode === "video" ? "#4a4a50" : "transparent"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Select from video"
                    color: selectedCoverDialog.activeMode === "video" ? Theme.text : Theme.textMuted
                    font.pixelSize: 13
                    font.weight: selectedCoverDialog.activeMode === "video" ? Font.DemiBold : Font.Normal
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: selectedCoverDialog.activeMode = "video"
                }
            }

            Rectangle {
                implicitWidth: 80
                implicitHeight: 32
                radius: 4
                color: selectedCoverDialog.activeMode === "local" ? "#303035" : "#141416"
                border.color: selectedCoverDialog.activeMode === "local" ? "#4a4a50" : "transparent"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Local"
                    color: selectedCoverDialog.activeMode === "local" ? Theme.text : Theme.textMuted
                    font.pixelSize: 13
                    font.weight: selectedCoverDialog.activeMode === "local" ? Font.DemiBold : Font.Normal
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: selectedCoverDialog.activeMode = "local"
                }
            }
        }

        // Bottom Bar - Video Mode (Filmstrip Timeline with Scrubber)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "#121214"
            radius: 6
            border.color: Theme.divider
            border.width: 1
            clip: true
            visible: selectedCoverDialog.activeMode === "video"

            ListView {
                id: coverFilmstripList
                anchors.fill: parent
                orientation: ListView.Horizontal
                interactive: false
                model: baseVideoPath !== "" ? 10 : 0
                delegate: Image {
                    required property int index
                    width: coverFilmstripList.width / 10
                    height: coverFilmstripList.height
                    sourceSize.height: coverFilmstripList.height
                    source: baseVideoPath !== "" ? "image://media/" + encodeURIComponent(baseVideoPath + "|" + (index * (selectedCoverDialog.videoDuration / 10.0) + 0.1)) : ""
                    asynchronous: true
                    fillMode: Image.Tile
                    verticalAlignment: Image.AlignTop
                    horizontalAlignment: Image.AlignLeft
                    clip: true
                }
            }

            // Scrubber Handle
            Rectangle {
                id: scrubberHandle
                x: (selectedCoverDialog.selectedTime / selectedCoverDialog.videoDuration) * (parent.width - 16)
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 16
                color: "transparent"

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: -4
                    width: 12
                    height: 14
                    radius: 3
                    color: "#ffffff"
                    border.color: "#333333"
                    border.width: 1
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 2
                    color: "#ffffff"
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onPositionChanged: function(mouse) {
                    if (mouse.buttons & Qt.LeftButton) {
                        let p = Math.max(0.0, Math.min(1.0, mouse.x / width))
                        selectedCoverDialog.selectedTime = p * selectedCoverDialog.videoDuration
                    }
                }
                onPressed: function(mouse) {
                    let p = Math.max(0.0, Math.min(1.0, mouse.x / width))
                    selectedCoverDialog.selectedTime = p * selectedCoverDialog.videoDuration
                }
            }
        }

        // Bottom Bar - Local Mode (File Picker Bar)
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            visible: selectedCoverDialog.activeMode === "local"
            spacing: 12

            Text {
                text: "From"
                color: Theme.text
                font.pixelSize: 13
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                color: "#202024"
                radius: 4
                border.color: "#303035"
                border.width: 1

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: selectedCoverDialog.localImagePath === "" ? "..." : selectedCoverDialog.localImagePath.replace("file://", "")
                    color: selectedCoverDialog.localImagePath === "" ? Theme.textMuted : Theme.text
                    font.pixelSize: 12
                    elide: Text.ElideLeft
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: coverFileDialog.open()
                }
            }

            Rectangle {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                color: "#303035"
                radius: 4
                border.color: "#4a4a50"
                border.width: 1

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/VideoStudioUI/assets/folder.svg"
                    sourceSize: Qt.size(16, 16)
                    opacity: 0.8
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: coverFileDialog.open()
                }
            }
        }
    }

    footer: Rectangle {
        color: "#1c1c1e"
        height: 68
        radius: 8

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Theme.divider
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            Button {
                text: "Create new cover"
                implicitWidth: 145
                implicitHeight: 38
                hoverEnabled: true
                HoverHandler { cursorShape: Qt.PointingHandCursor }
                background: Rectangle {
                    color: parent.hovered ? "#4a4a52" : "#38383e"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.text
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    // Placeholder for create new cover backend action
                }
            }

            Item { Layout.fillWidth: true }

            RowLayout {
                spacing: 14

                Button {
                    text: "Edit cover"
                    implicitWidth: 100
                    implicitHeight: 38
                    hoverEnabled: true
                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                    background: Rectangle {
                        color: parent.hovered ? "#38ffe8" : "#25e5cf"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#000000"
                        font.pixelSize: 14
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        selectedCoverDialog.close()
                        coverEditDialogModal.open()
                    }
                }

                Button {
                    text: "Cancel"
                    implicitWidth: 90
                    implicitHeight: 38
                    hoverEnabled: true
                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                    background: Rectangle {
                        color: parent.hovered ? "#555555" : "#4a4a4a"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.text
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: selectedCoverDialog.close()
                }
            }
        }
    }

    CoverEditDialog {
        id: coverEditDialogModal
        timelineController: selectedCoverDialog.timelineController
        parentSelectDialog: selectedCoverDialog
        coverImagePath: selectedCoverDialog.activeMode === "local" && selectedCoverDialog.localImagePath !== "" ? selectedCoverDialog.localImagePath : selectedCoverDialog.baseVideoPath
        coverTime: selectedCoverDialog.selectedTime
        parent: Overlay.overlay
    }
}
