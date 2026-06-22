pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import VideoStudioUI
import "../utils/ExportUtils.js" as ExportUtils

Dialog {
    id: exportDialog
    property bool isVideoExpanded: true
    property bool isVideoChecked: true
    property bool isAudioExpanded: true
    property bool isAudioChecked: true
    property var timelineController: null
    property var subtitleController: null
    property bool isExporting: false
    property real currentExportProgress: 0.0
    property real exportStartTime: 0
    property string exportTimeStr: "00:00:00"
    property string lastExportMessage: ""
    property string selectedSubtitleFont: "Khmer UI"
    property bool isSubtitleExpanded: false
    property string fontSearchText: ""
    property bool isVertical: typeof mainWindow !== "undefined" ? mainWindow.isVerticalLayout : false

    function sanitizedName() {
        const clean = nameField.text.trim().replace(/[\\/:*?"<>|]+/g, "_")
        return clean.length > 0 ? clean : "ExportedVideo"
    }

    function preferredExtension() {
        if (!exportDialog.isVideoChecked && exportDialog.isAudioChecked)
            return audioFormatCombo.currentText.toLowerCase()

        const format = formatCombo.currentText.toLowerCase()
        if (format === "webm") return "webm"
        if (format === "mkv") return "mkv"
        if (format === "mov") return "mov"
        return "mp4"
    }

    function replaceFileNameFromName() {
        const currentPath = pathField.text.length > 0 ? pathField.text : "C:/we_hunting/video_studio/export.mp4"
        const normalized = currentPath.replace(/\\/g, "/")
        const slash = normalized.lastIndexOf("/")
        const folder = slash >= 0 ? normalized.substring(0, slash + 1) : "C:/we_hunting/video_studio/"
        pathField.text = folder + sanitizedName() + "." + preferredExtension()
    }

    function updateOutputExtension() {
        const normalized = pathField.text.replace(/\\/g, "/")
        const slash = normalized.lastIndexOf("/")
        const dot = normalized.lastIndexOf(".")
        const withoutExtension = dot > slash ? normalized.substring(0, dot) : normalized
        pathField.text = withoutExtension + "." + preferredExtension()
    }

    function localPathFromUrl(fileUrl) {
        let path = fileUrl.toString()
        if (path.indexOf("file:///") === 0)
            path = path.substring(8)
        else if (path.indexOf("file://") === 0)
            path = path.substring(7)
        return decodeURIComponent(path).replace(/\\/g, "/")
    }

    title: "Export Media"
    modal: true
    anchors.centerIn: Overlay.overlay

    Timer {
        id: exportTimer
        interval: 1000
        running: exportDialog.isExporting
        repeat: true
        onTriggered: {
            const elapsedMs = Date.now() - exportDialog.exportStartTime
            const totalSeconds = Math.floor(elapsedMs / 1000)
            const hours = Math.floor(totalSeconds / 3600)
            const minutes = Math.floor((totalSeconds % 3600) / 60)
            const seconds = totalSeconds % 60
            
            exportDialog.exportTimeStr = (hours < 10 ? "0" : "") + hours + ":" +
                            (minutes < 10 ? "0" : "") + minutes + ":" +
                            (seconds < 10 ? "0" : "") + seconds
        }
    }

    VideoExporter {
        id: videoExporter
        onExportProgress: function(progress) {
            exportDialog.currentExportProgress = progress
        }
        onExportFinished: function(success, message) {
            exportDialog.isExporting = false
            exportDialog.currentExportProgress = success ? 1.0 : 0.0
            exportTimer.stop()
            console.log("Export finished: ", success, message)
            if (success) {
                exportDialog.close()
            } else {
                exportDialog.lastExportMessage = message
                exportErrorDialog.open()
            }
        }
    }

    FolderDialog {
        id: exportFileDialog
        title: "Export media to folder"
        currentFolder: "file:///C:/we_hunting"
        onAccepted: {
            let folder = exportDialog.localPathFromUrl(selectedFolder)
            if (!folder.endsWith("/")) folder += "/"
            pathField.text = folder + exportDialog.sanitizedName() + "." + exportDialog.preferredExtension()
        }
    }
    width: 780
    height: 660

    Component.onCompleted: replaceFileNameFromName()
    
    background: Rectangle {
        color: "#181818"
        border.color: Theme.divider
        border.width: 1
        radius: 6
    }
    
    header: Rectangle {
        color: "#181818"
        height: 44
        radius: 6
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "Export"
            color: Theme.text
            font.pixelSize: 16
            font.weight: Font.DemiBold
        }
        
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.divider
        }
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 32

        // Left Panel - Cover Image Placeholder
        Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            color: "#282828"
            radius: 8
            
            Button {
                id: editCoverBtn
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 12
                anchors.leftMargin: 12
                text: "✎ Edit cover"
                background: Rectangle {
                    color: "#000000"
                    opacity: 0.5
                    radius: 4
                }
                contentItem: Text {
                    text: editCoverBtn.text
                    color: Theme.text
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        // Right Panel - Export Settings
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 14
            
            RowLayout {
                spacing: 16
                Text {
                    text: "Export timeline"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                Text {
                    text: "Timeline 01"
                    color: "#777777"
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }
            }
            
            RowLayout {
                spacing: 16
                Text {
                    text: "Name"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                TextField {
                    id: nameField
                    text: "ExportedVideo"
                    Layout.fillWidth: true
                    placeholderText: "Enter video name"
                    color: Theme.text
                    font.pixelSize: 13
                    onTextEdited: exportDialog.replaceFileNameFromName()
                    background: Rectangle {
                        color: "#1a1a1a"
                        border.color: Theme.divider
                        border.width: 1
                        radius: 4
                        implicitHeight: 32
                    }
                }
            }
            
            RowLayout {
                spacing: 16
                Text {
                    text: "Export to"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    TextField {
                        id: pathField
                        text: "C:/we_hunting/video_studio/export.mp4"
                        Layout.fillWidth: true
                        placeholderText: "Select export path..."
                        color: Theme.text
                        font.pixelSize: 13
                        readOnly: true
                        background: Rectangle {
                            color: "#1a1a1a"
                            border.color: Theme.divider
                            border.width: 1
                            radius: 4
                            implicitHeight: 32
                        }
                    }
                    Button {
                        id: browseExportBtn
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        icon.source: "qrc:/VideoStudioUI/assets/folder.svg"
                        icon.color: Theme.text
                        
                        background: Rectangle {
                            color: "#1a1a1a"
                            border.color: Theme.divider
                            border.width: 1
                            radius: 4
                        }
                        onClicked: exportFileDialog.open()
                    }
                }
            }
            
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Theme.divider
                Layout.topMargin: 8
                Layout.bottomMargin: 8
            }
            
            RowLayout {
                spacing: 8
                
                Rectangle {
                    implicitWidth: 14
                    implicitHeight: 14
                    radius: 2
                    color: exportDialog.isVideoChecked ? "#25e5cf" : "transparent"
                    border.color: exportDialog.isVideoChecked ? "#25e5cf" : "#777777"
                    Layout.alignment: Qt.AlignVCenter
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/VideoStudioUI/assets/check.svg"
                        width: 14
                        height: 14
                        visible: exportDialog.isVideoChecked
                        fillMode: Image.PreserveAspectFit
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isVideoChecked = !exportDialog.isVideoChecked
                            exportDialog.updateOutputExtension()
                        }
                    }
                }
                
                Text {
                    text: "Video"
                    color: Theme.text
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    Layout.alignment: Qt.AlignVCenter
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isVideoExpanded = !exportDialog.isVideoExpanded;
                            if (exportDialog.isVideoExpanded) exportDialog.isAudioExpanded = false;
                        }
                    }
                }
                
                Image {
                    source: exportDialog.isVideoExpanded ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
                    width: 16
                    height: 16
                    Layout.alignment: Qt.AlignVCenter
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isVideoExpanded = !exportDialog.isVideoExpanded;
                            if (exportDialog.isVideoExpanded) exportDialog.isAudioExpanded = false;
                        }
                    }
                }
                Item { Layout.fillWidth: true }
            }
            
            RowLayout {
                spacing: 16
                visible: exportDialog.isVideoExpanded
                Text {
                    text: "Resolution"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                CapCutComboBox {
                    id: resolutionCombo
                    Layout.fillWidth: true
                    model: ["1080P", "4K", "720P", "480P", "360P"]
                }
            }
            
            RowLayout {
                spacing: 16
                visible: exportDialog.isVideoExpanded
                Text {
                    text: "Bit rate"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                CapCutComboBox {
                    id: bitrateCombo
                    Layout.fillWidth: true
                    model: ["Higher", "Recommended", "Custom"]
                }
            }
            
            RowLayout {
                spacing: 16
                visible: exportDialog.isVideoExpanded
                Text {
                    text: "Codec"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                CapCutComboBox {
                    id: codecCombo
                    Layout.fillWidth: true
                    model: ["H.264", "HEVC", "AV1"]
                }
            }
            
            RowLayout {
                spacing: 16
                visible: exportDialog.isVideoExpanded
                Text {
                    text: "Format"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                CapCutComboBox {
                    id: formatCombo
                    Layout.fillWidth: true
                    model: ["MP4", "MOV", "MKV", "WebM"]
                    onCurrentTextChanged: exportDialog.updateOutputExtension()
                }
            }
            
            RowLayout {
                spacing: 16
                visible: exportDialog.isVideoExpanded
                Text {
                    text: "Frame rate"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                CapCutComboBox {
                    id: frameRateCombo
                    Layout.fillWidth: true
                    model: ["60 fps", "50 fps", "30 fps", "25 fps", "24 fps"]
                }
            }
            
            RowLayout {
                spacing: 8
                Layout.topMargin: 16
                
                Rectangle {
                    implicitWidth: 14
                    implicitHeight: 14
                    radius: 2
                    color: exportDialog.isAudioChecked ? "#25e5cf" : "transparent"
                    border.color: exportDialog.isAudioChecked ? "#25e5cf" : "#777777"
                    Layout.alignment: Qt.AlignVCenter
                    
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/VideoStudioUI/assets/check.svg"
                        width: 14
                        height: 14
                        visible: exportDialog.isAudioChecked
                        fillMode: Image.PreserveAspectFit
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isAudioChecked = !exportDialog.isAudioChecked
                            exportDialog.updateOutputExtension()
                        }
                    }
                }
                
                Text {
                    text: "Audio"
                    color: Theme.text
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    Layout.alignment: Qt.AlignVCenter
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isAudioExpanded = !exportDialog.isAudioExpanded;
                            if (exportDialog.isAudioExpanded) exportDialog.isVideoExpanded = false;
                        }
                    }
                }
                
                Image {
                    source: exportDialog.isAudioExpanded ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
                    width: 14
                    height: 14
                    Layout.alignment: Qt.AlignVCenter
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isAudioExpanded = !exportDialog.isAudioExpanded;
                            if (exportDialog.isAudioExpanded) exportDialog.isVideoExpanded = false;
                        }
                    }
                }
                Item { Layout.fillWidth: true }
            }
            
            RowLayout {
                spacing: 16
                visible: exportDialog.isAudioExpanded
                Text {
                    text: "Format"
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.preferredWidth: 100
                }
                CapCutComboBox {
                    id: audioFormatCombo
                    Layout.fillWidth: true
                    model: ["AAC", "MP3", "WAV", "FLAC", "OGG"]
                    onCurrentTextChanged: exportDialog.updateOutputExtension()
                }
            }
            
            // ──── Subtitle Font Section ────
            RowLayout {
                spacing: 8
                Layout.topMargin: 16
                
                Text {
                    text: "🔤"
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignVCenter
                }
                Text {
                    text: "Subtitle Font"
                    color: Theme.text
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    Layout.alignment: Qt.AlignVCenter
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isSubtitleExpanded = !exportDialog.isSubtitleExpanded
                        }
                    }
                }
                
                Image {
                    source: exportDialog.isSubtitleExpanded ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
                    width: 14
                    height: 14
                    Layout.alignment: Qt.AlignVCenter
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            exportDialog.isSubtitleExpanded = !exportDialog.isSubtitleExpanded
                        }
                    }
                }
                Item { Layout.fillWidth: true }
                
                Text {
                    text: exportDialog.selectedSubtitleFont
                    color: "#777777"
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignVCenter
                }
            }
            
            // Subtitle Font Picker Content
            ColumnLayout {
                spacing: 8
                visible: exportDialog.isSubtitleExpanded
                Layout.fillWidth: true
                
                // Search box
                Rectangle {
                    Layout.fillWidth: true
                    height: 32
                    color: "#1a1a1a"
                    border.color: fontSearchInput.activeFocus ? "#25e5cf" : Theme.divider
                    border.width: 1
                    radius: 4
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 6
                        
                        Text {
                            text: "🔍"
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignVCenter
                        }
                        
                        TextInput {
                            id: fontSearchInput
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            color: Theme.text
                            font.pixelSize: 13
                            clip: true
                            onTextChanged: exportDialog.fontSearchText = text
                            
                            Text {
                                anchors.fill: parent
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Search fonts..."
                                color: "#555555"
                                font.pixelSize: 13
                                visible: !fontSearchInput.text && !fontSearchInput.activeFocus
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
                
                // Font list
                Rectangle {
                    Layout.fillWidth: true
                    height: 120
                    color: "#1a1a1a"
                    border.color: Theme.divider
                    border.width: 1
                    radius: 4
                    clip: true
                    
                    ListView {
                        id: fontListView
                        anchors.fill: parent
                        anchors.margins: 2
                        model: {
                            const allFonts = Qt.fontFamilies()
                            if (!exportDialog.fontSearchText) return allFonts
                            const search = exportDialog.fontSearchText.toLowerCase()
                            return allFonts.filter(function(f) { return f.toLowerCase().indexOf(search) >= 0 })
                        }
                        currentIndex: -1
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }
                        delegate: Rectangle {
                            id: fontDelegate
                            required property string modelData
                            required property int index
                            width: fontListView.width - 10
                            height: 28
                            color: fontDelegate.modelData === exportDialog.selectedSubtitleFont ? "#2a3a3a" : (fontMouseArea.containsMouse ? "#222222" : "transparent")
                            radius: 3
                            
                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: fontDelegate.modelData
                                color: fontDelegate.modelData === exportDialog.selectedSubtitleFont ? "#25e5cf" : Theme.textMuted
                                font.pixelSize: 13
                                font.family: fontDelegate.modelData
                                elide: Text.ElideRight
                                width: parent.width - 16
                            }
                            
                            MouseArea {
                                id: fontMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    exportDialog.selectedSubtitleFont = fontDelegate.modelData
                                }
                            }
                        }
                    }
                }
                
                // Preview
                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#111111"
                    border.color: Theme.divider
                    border.width: 1
                    radius: 4
                    
                    Text {
                        anchors.centerIn: parent
                        text: "AaBbCc អ ខ គ ១២៣"
                        color: Theme.text
                        font.pixelSize: 18
                        font.family: exportDialog.selectedSubtitleFont
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
    
    footer: Rectangle {
        color: "#181818"
        height: 60
        radius: 6
        
        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Theme.divider
        }
        
        Item {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            
            // Standard Export Footer
            RowLayout {
                anchors.fill: parent
                visible: !exportDialog.isExporting
                
                RowLayout {
                    spacing: 8
                    Image {
                        source: "qrc:/VideoStudioUI/assets/film.svg"
                        width: 16
                        height: 16
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        text: "Duration: " + (exportDialog.timelineController ? ExportUtils.formatDuration(exportDialog.timelineController.timelineEndSeconds) : "--") + 
                              " | Size: about " + (exportDialog.timelineController ? ExportUtils.formatSize(exportDialog.timelineController.timelineEndSeconds, resolutionCombo.currentText, exportDialog.isVideoChecked) : "--")
                        color: Theme.textMuted
                        font.pixelSize: 13
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                RowLayout {
                    spacing: 12
                    
                    Button {
                        id: exportBtn
                        text: "Export"
                        implicitWidth: 80
                        implicitHeight: 32
                        background: Rectangle {
                            color: exportBtn.hovered ? "#38ffe8" : "#25e5cf"
                            radius: 4
                        }
                        contentItem: Text {
                            text: exportBtn.text
                            color: "#000000"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (!exportDialog.timelineController || exportDialog.timelineController.timelineEndSeconds <= 0) {
                                emptyWarningDialog.open()
                                return
                            }
                            exportDialog.isExporting = true
                            exportDialog.currentExportProgress = 0.0
                            exportDialog.exportStartTime = Date.now()
                            exportTimer.start()
                            videoExporter.startExportWithSettings(exportDialog.timelineController, pathField.text, {
                                "exportVideo": exportDialog.isVideoChecked,
                                "exportAudio": exportDialog.isAudioChecked,
                                "resolution": resolutionCombo.currentText,
                                "bitrate": bitrateCombo.currentText,
                                "codec": codecCombo.currentText,
                                "format": formatCombo.currentText,
                                "frameRate": frameRateCombo.currentText,
                                "audioFormat": audioFormatCombo.currentText,
                                "subtitleFont": exportDialog.subtitleController ? exportDialog.subtitleController.font.family : exportDialog.selectedSubtitleFont,
                                "subtitleFontSize": exportDialog.subtitleController ? (exportDialog.subtitleController.font.pixelSize > 0 ? exportDialog.subtitleController.font.pixelSize : exportDialog.subtitleController.font.pointSize) : 48,
                                "subtitleVerticalPosition": exportDialog.subtitleController ? exportDialog.subtitleController.verticalPosition : -1.0,
                                "isVertical": exportDialog.isVertical
                            })
                        }
                    }
                    
                    Button {
                        id: cancelBtn
                        text: "Cancel"
                        implicitWidth: 80
                        implicitHeight: 32
                        background: Rectangle {
                            color: cancelBtn.hovered ? "#555555" : "#4a4a4a"
                            radius: 4
                        }
                        contentItem: Text {
                            text: cancelBtn.text
                            color: Theme.text
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: exportDialog.close()
                    }
                }
            }
            
            // Exporting Progress UI
            RowLayout {
                anchors.fill: parent
                visible: exportDialog.isExporting
                spacing: 12
                
                Text {
                    text: (exportDialog.currentExportProgress * 100).toFixed(1) + "%"
                    color: Theme.text
                    font.pixelSize: 13
                }
                
                Rectangle {
                    width: 1
                    height: 14
                    color: Theme.divider
                }
                
                Text {
                    text: exportDialog.exportTimeStr
                    color: Theme.textMuted
                    font.pixelSize: 13
                }
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 4
                    color: "#333333"
                    radius: 2
                    
                    Rectangle {
                        width: parent.width * exportDialog.currentExportProgress
                        height: parent.height
                        color: "#25e5cf"
                        radius: 2
                    }
                }
                
                Button {
                    id: backHomeBtn
                    text: "Back to home"
                    implicitWidth: 100
                    implicitHeight: 32
                    background: Rectangle {
                        color: backHomeBtn.hovered ? "#38ffe8" : "#25e5cf"
                        radius: 4
                    }
                    contentItem: Text {
                        text: backHomeBtn.text
                        color: "#000000"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: exportDialog.close()
                }
                
                Button {
                    id: cancelProgressBtn
                    text: "Cancel"
                    implicitWidth: 80
                    implicitHeight: 32
                    background: Rectangle {
                        color: cancelProgressBtn.hovered ? "#555555" : "#4a4a4a"
                        radius: 4
                    }
                    contentItem: Text {
                        text: cancelProgressBtn.text
                        color: Theme.text
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        videoExporter.cancelExport()
                    }
                }
            }
        }
    }

    component CapCutComboBox: ComboBox {
        id: control
        
        delegate: ItemDelegate {
            id: delegateItem
            required property string modelData
            width: control.width
            height: 32
            contentItem: Text {
                text: delegateItem.modelData
                color: delegateItem.highlighted ? Theme.text : Theme.textMuted
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
                leftPadding: 8
            }
            background: Rectangle {
                color: "transparent"
                radius: 2
            }
        }
        
        indicator: Image {
            x: control.width - width - 12
            y: (control.availableHeight - height) / 2
            width: 16
            height: 16
            source: control.popup.visible ? "qrc:/VideoStudioUI/assets/chevron-up.svg" : "qrc:/VideoStudioUI/assets/chevron-down.svg"
            fillMode: Image.PreserveAspectFit
        }
        
        contentItem: Text {
            leftPadding: 12
            rightPadding: control.indicator.width + control.spacing
            text: control.displayText
            font.pixelSize: 13
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        
        background: Rectangle {
            implicitWidth: 120
            implicitHeight: 32
            color: "#1a1a1a"
            border.color: control.pressed ? Theme.divider : Theme.divider
            border.width: 1
            radius: 4
        }
        
        popup: Popup {
            width: control.width
            implicitHeight: contentItem.implicitHeight + 2
            padding: 1
            
            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: control.popup.visible ? control.delegateModel : null
                currentIndex: control.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator { }
            }
            
            background: Rectangle {
                color: "#1a1a1a"
                border.color: Theme.divider
                border.width: 1
                radius: 4
            }
        }
    }

    Dialog {
        id: emptyWarningDialog
        title: "No video to export"
        anchors.centerIn: parent
        width: 300
        height: 150
        modal: true
        background: Rectangle {
            color: "#1e1e1e"
            radius: 8
            border.color: Theme.divider
            border.width: 1
        }
        
        contentItem: ColumnLayout {
            spacing: 16
            Text {
                text: "The timeline is empty. Please add clips to the timeline before exporting."
                color: Theme.text
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            
            Button {
                id: okBtn
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 100
                implicitHeight: 32
                background: Rectangle {
                    color: "#25e5cf"
                    radius: 4
                }
                contentItem: Text {
                    text: okBtn.text
                    color: "#000000"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: emptyWarningDialog.close()
            }
        }
    }

    Dialog {
        id: exportErrorDialog
        title: "Export failed"
        anchors.centerIn: parent
        width: 420
        height: 210
        modal: true
        background: Rectangle {
            color: "#1e1e1e"
            radius: 8
            border.color: Theme.divider
            border.width: 1
        }

        contentItem: ColumnLayout {
            spacing: 16
            Text {
                text: exportDialog.lastExportMessage.length > 0 ? exportDialog.lastExportMessage : "The export failed."
                color: Theme.text
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                maximumLineCount: 5
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Button {
                id: exportErrorOkBtn
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: 100
                implicitHeight: 32
                background: Rectangle {
                    color: "#25e5cf"
                    radius: 4
                }
                contentItem: Text {
                    text: exportErrorOkBtn.text
                    color: "#000000"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: exportErrorDialog.close()
            }
        }
    }
}
