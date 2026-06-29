pragma ComponentBehavior: Bound
// qmllint disable
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VideoStudioUI
import VideoStudio.Core
import VideoStudio.Models
import "feature" as BackendFeatures

Dialog {
    id: coverEditDialog
    property var timelineController: null
    property var parentSelectDialog: null
    property string coverImagePath: ""

    GimpProcessor {
        id: gimpProcessor
        onProcessingFinished: (success, resultPath, operation) => {
            console.log("GIMP processing completed:", operation, success, resultPath)
        }
        onGimpEditorUpdateReceived: (updatedPath) => {
            console.log("GIMP external update received:", updatedPath)
        }
    }
    property real coverTime: 0.1

    // Editing state properties
    property string activeTab: "text" // "templates", "text", "stickers", "effects"
    property string coverText: "CINEMATIC TRAILER"
    property string coverTextColor: "#ffffff"
    property string coverTextBg: "transparent"
    property int coverTextSize: 32
    property string coverFontFamily: "Inter"
    property int coverLetterSpacing: 0
    property bool coverTextShadow: true
    property bool coverTextOutline: false
    property bool coverTextGlow: false
    property string coverTextTransform: "none" // "none", "uppercase", "lowercase"
    property real textElementX: 300
    property real textElementY: 300
    property bool showTextOverlay: true

    property string activeSticker: ""
    property real stickerX: 150
    property real stickerY: 150
    property real stickerScale: 1.2
    property real stickerOpacity: 1.0

    property bool hasVignette: false
    property bool hasNeonBorder: false
    property string activeFilter: "none" // "none", "warm", "cool", "vintage", "bw"
    property bool hasFilmGrain: false
    property bool removeBackground: false
    property bool smartCutout: false
    property bool aiUpscale: false
    property bool objectOutline: false
    property bool glossyReflection: false
    property real coverImageScale: 1.0

    title: "Edit Cover - Add Text & Styling"
    modal: true
    
    // Full screen setup
    x: 0
    y: 0
    width: parent ? parent.width : 1280
    height: parent ? parent.height : 720
    margins: 0
    padding: 0

    background: Rectangle {
        color: "#0e0e11" // Deep immersive studio background
    }

    header: Rectangle {
        color: "#18181c"
        height: 54

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24

            Text {
                text: coverEditDialog.title
                color: Theme.text
                font.pixelSize: 16
                font.weight: Font.Bold
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "Professional Cover Studio"
                color: Theme.textMuted
                font.pixelSize: 13
                font.weight: Font.Medium
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.preferredWidth: 16 }

            Button {
                implicitWidth: 32
                implicitHeight: 32
                background: Rectangle {
                    color: parent.hovered ? "#33333c" : "transparent"
                    radius: 16
                }
                contentItem: Image {
                    source: "qrc:/VideoStudioUI/assets/x.svg"
                    sourceSize: Qt.size(16, 16)
                    fillMode: Image.PreserveAspectFit
                    horizontalAlignment: Image.AlignHCenter
                    verticalAlignment: Image.AlignVCenter
                }
                onClicked: coverEditDialog.close()
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#27272e"
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Left Navigation Sidebar
        Rectangle {
            Layout.preferredWidth: 92
            Layout.fillHeight: true
            color: "#18181c"
            radius: 12
            border.color: "#27272e"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
                spacing: 16

                // Templates Tab Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    color: coverEditDialog.activeTab === "templates" ? "#222228" : "transparent"
                    radius: 8
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/VideoStudioUI/assets/grid.svg"
                            sourceSize: Qt.size(24, 24)
                            opacity: coverEditDialog.activeTab === "templates" ? 1.0 : 0.5
                        }
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Templates"
                            color: coverEditDialog.activeTab === "templates" ? Theme.text : Theme.textMuted
                            font.pixelSize: 12
                            font.weight: coverEditDialog.activeTab === "templates" ? Font.Bold : Font.Normal
                        }
                    }
                    
                    MouseArea { 
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: coverEditDialog.activeTab = "templates" 
                    }
                }

                // Text Tab Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    color: coverEditDialog.activeTab === "text" ? "#222228" : "transparent"
                    radius: 8
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/VideoStudioUI/assets/type.svg"
                            sourceSize: Qt.size(24, 24)
                            opacity: coverEditDialog.activeTab === "text" ? 1.0 : 0.5
                        }
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Text"
                            color: coverEditDialog.activeTab === "text" ? Theme.text : Theme.textMuted
                            font.pixelSize: 12
                            font.weight: coverEditDialog.activeTab === "text" ? Font.Bold : Font.Normal
                        }
                    }
                    
                    MouseArea { 
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: coverEditDialog.activeTab = "text" 
                    }
                }

                // Stickers Tab Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    color: coverEditDialog.activeTab === "stickers" ? "#222228" : "transparent"
                    radius: 8
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/VideoStudioUI/assets/star.svg"
                            sourceSize: Qt.size(24, 24)
                            opacity: coverEditDialog.activeTab === "stickers" ? 1.0 : 0.5
                        }
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Stickers"
                            color: coverEditDialog.activeTab === "stickers" ? Theme.text : Theme.textMuted
                            font.pixelSize: 12
                            font.weight: coverEditDialog.activeTab === "stickers" ? Font.Bold : Font.Normal
                        }
                    }
                    
                    MouseArea { 
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: coverEditDialog.activeTab = "stickers" 
                    }
                }

                // Effects Tab Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    color: coverEditDialog.activeTab === "effects" ? "#222228" : "transparent"
                    radius: 8
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/VideoStudioUI/assets/sliders.svg"
                            sourceSize: Qt.size(24, 24)
                            opacity: coverEditDialog.activeTab === "effects" ? 1.0 : 0.5
                        }
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Effects"
                            color: coverEditDialog.activeTab === "effects" ? Theme.text : Theme.textMuted
                            font.pixelSize: 12
                            font.weight: coverEditDialog.activeTab === "effects" ? Font.Bold : Font.Normal
                        }
                    }
                    
                    MouseArea { 
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: coverEditDialog.activeTab = "effects" 
                    }
                }

                // Backend Core Tab Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    color: coverEditDialog.activeTab === "backend" ? "#222228" : "transparent"
                    radius: 8
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "qrc:/VideoStudioUI/assets/cpu.svg" // Fallback to settings or cpu if available, otherwise just text
                            sourceSize: Qt.size(24, 24)
                            opacity: coverEditDialog.activeTab === "backend" ? 1.0 : 0.5
                        }
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "GIMP Core"
                            color: coverEditDialog.activeTab === "backend" ? Theme.text : Theme.textMuted
                            font.pixelSize: 12
                            font.weight: coverEditDialog.activeTab === "backend" ? Font.Bold : Font.Normal
                        }
                    }
                    
                    MouseArea { 
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: coverEditDialog.activeTab = "backend" 
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Middle Preview Canvas (Hero Workspace)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#08080a"
            radius: 12
            border.color: "#27272e"
            border.width: 1
            clip: true

            // Subtle background grid pattern representation
            Grid {
                id: bgPatternGrid
                anchors.fill: parent
                rows: 10
                columns: 10
                opacity: 0.03
                Repeater {
                    model: 100
                    Rectangle {
                        width: bgPatternGrid.width / 10
                        height: bgPatternGrid.height / 10
                        color: "transparent"
                        border.color: "#ffffff"
                        border.width: 1
                    }
                }
            }

            // Floating status bar overlay at top of canvas
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 44
                color: "transparent"
                z: 10
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    Rectangle {
                        color: "#222228"
                        radius: 14
                        implicitWidth: 110
                        implicitHeight: 28
                        border.color: "#33333c"
                        border.width: 1
                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6
                            Image {
                                source: "qrc:/VideoStudioUI/assets/film.svg"
                                sourceSize: Qt.size(14, 14)
                            }
                            Text { 
                                text: "16:9 Canvas"
                                color: Theme.text
                                font.pixelSize: 12
                                font.weight: Font.Medium 
                            }
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        color: "#222228"
                        radius: 14
                        implicitWidth: 140
                        implicitHeight: 28
                        border.color: "#33333c"
                        border.width: 1
                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6
                            Image {
                                source: "qrc:/VideoStudioUI/assets/tool-zoom.svg"
                                sourceSize: Qt.size(14, 14)
                            }
                            Text { 
                                text: "Fit View (100%)"
                                color: Theme.text
                                font.pixelSize: 12
                                font.weight: Font.Medium 
                            }
                        }
                    }
                }
            }

            // Base Image / Video Frame
            Image {
                anchors.centerIn: parent
                width: (parent.width - 64) * coverEditDialog.coverImageScale
                height: (parent.height - 64) * coverEditDialog.coverImageScale
                fillMode: Image.PreserveAspectFit
                source: coverEditDialog.coverImagePath !== "" ? (coverEditDialog.coverImagePath.startsWith("file://") ? coverEditDialog.coverImagePath : "image://media/" + encodeURIComponent(coverEditDialog.coverImagePath + "|" + coverEditDialog.coverTime)) : ""
                asynchronous: true
                opacity: coverEditDialog.removeBackground ? 0.9 : 1.0
            }

            // Remove Background Simulated Grid
            Grid {
                id: checkerGrid
                anchors.fill: parent
                anchors.margins: 32
                rows: 25
                columns: 25
                z: -1
                visible: coverEditDialog.removeBackground
                Repeater {
                    model: 625
                    Rectangle {
                        width: checkerGrid.width / 25
                        height: checkerGrid.height / 25
                        color: (index % 2 === 0) ? "#1a1a20" : "#25252e"
                    }
                }
            }

            // Smart Cutout Dashed Frame
            Rectangle {
                anchors.fill: parent
                anchors.margins: 40
                color: "transparent"
                border.color: "#38bdf8"
                border.width: 2
                radius: 12
                visible: coverEditDialog.smartCutout
                opacity: 0.85
            }

            // AI Upscale High-Def Enhancement Indicator
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 44
                width: 140
                height: 32
                radius: 16
                color: "#121215"
                border.color: "#38bdf8"
                border.width: 1
                visible: coverEditDialog.aiUpscale
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 6
                    Text { text: "⚡"; font.pixelSize: 14; color: "#38bdf8" }
                    Text { text: "AI 4K UPSCALED"; font.pixelSize: 11; font.weight: Font.Bold; color: "#38bdf8" }
                }
            }

            // Object Outline Glow
            Rectangle {
                anchors.fill: parent
                anchors.margins: 32
                color: "transparent"
                border.color: "#38bdf8"
                border.width: 3
                radius: 12
                visible: coverEditDialog.objectOutline
                opacity: 0.7
            }

            // Glossy Reflection Overlay
            Rectangle {
                anchors.fill: parent
                anchors.margins: 32
                color: "transparent"
                visible: coverEditDialog.glossyReflection
                radius: 12
                clip: true
                Rectangle {
                    x: -parent.width / 2
                    y: -parent.height / 2
                    width: parent.width * 2
                    height: parent.height
                    rotation: 30
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 0.45; color: "#05ffffff" }
                        GradientStop { position: 0.5; color: "#25ffffff" }
                        GradientStop { position: 0.55; color: "transparent" }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }
            }

            // Color Filters Overlay
            Rectangle {
                anchors.fill: parent
                anchors.margins: 32
                color: coverEditDialog.activeFilter === "warm" ? "#15ffaa00" :
                       coverEditDialog.activeFilter === "cool" ? "#1500aaff" :
                       coverEditDialog.activeFilter === "vintage" ? "#208b4513" :
                       coverEditDialog.activeFilter === "bw" ? "#33888888" : "transparent"
                visible: coverEditDialog.activeFilter !== "none"
                radius: 12
            }

            // Film Grain Overlay (Simulated via subtle fine grid)
            Grid {
                id: filmGrainGrid
                anchors.fill: parent
                anchors.margins: 32
                rows: 30    
                columns: 30
                opacity: 0.04
                visible: coverEditDialog.hasFilmGrain
                Repeater {
                    model: 900
                    Rectangle {
                        width: filmGrainGrid.width / 30
                        height: filmGrainGrid.height / 30
                        color: (index % 2 === 0) ? "#000000" : "#ffffff"
                    }
                }
            }

            // Vignette Effect Overlay
            Rectangle {
                anchors.fill: parent
                anchors.margins: 32
                visible: coverEditDialog.hasVignette
                color: "transparent"
                border.color: "#bb000000"
                border.width: 60
                opacity: 0.8
                radius: 12
            }

            // Neon Border Effect
            Rectangle {
                anchors.fill: parent
                anchors.margins: 44
                visible: coverEditDialog.hasNeonBorder
                color: "transparent"
                border.color: "#25e5cf"
                border.width: 4
                radius: 16
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -6
                    color: "transparent"
                    border.color: "#ff3860"
                    border.width: 2
                    radius: 22
                    opacity: 0.7
                }
            }

            // Interactive Sticker Overlay
            Rectangle {
                x: coverEditDialog.stickerX
                y: coverEditDialog.stickerY
                width: 100
                height: 100
                color: "transparent"
                visible: coverEditDialog.activeSticker !== ""

                Image {
                    anchors.centerIn: parent
                    source: coverEditDialog.activeSticker
                    sourceSize: Qt.size(80 * coverEditDialog.stickerScale, 80 * coverEditDialog.stickerScale)
                    fillMode: Image.PreserveAspectFit
                    opacity: coverEditDialog.stickerOpacity
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeAllCursor
                    drag.target: parent
                }
            }

            // Interactive Text Overlay Box
            Rectangle {
                id: textOverlayBox
                x: coverEditDialog.textElementX
                y: coverEditDialog.textElementY
                width: overlayText.contentWidth + 40
                height: overlayText.contentHeight + 24
                radius: 8
                color: coverEditDialog.coverTextBg
                border.color: textDragArea.containsMouse ? "#25e5cf" : "#ffffff"
                border.width: textDragArea.containsMouse ? 2 : 1
                visible: coverEditDialog.showTextOverlay && coverEditDialog.coverText !== ""

                Text {
                    id: overlayText
                    anchors.centerIn: parent
                    text: coverEditDialog.coverTextTransform === "uppercase" ? coverEditDialog.coverText.toUpperCase() :
                          coverEditDialog.coverTextTransform === "lowercase" ? coverEditDialog.coverText.toLowerCase() : coverEditDialog.coverText
                    color: coverEditDialog.coverTextColor
                    font.family: coverEditDialog.coverFontFamily
                    font.pixelSize: coverEditDialog.coverTextSize
                    font.weight: Font.Bold
                    font.letterSpacing: coverEditDialog.coverLetterSpacing
                    style: coverEditDialog.coverTextOutline ? Text.Outline : (coverEditDialog.coverTextShadow ? Text.Raised : Text.Normal)
                    styleColor: coverEditDialog.coverTextOutline ? "#000000" : "#88000000"

                    // Glow Effect Behind Text
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: -8
                        color: coverEditDialog.coverTextColor
                        radius: 16
                        opacity: 0.25
                        z: -1
                        visible: coverEditDialog.coverTextGlow
                    }
                }

                MouseArea {
                    id: textDragArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeAllCursor
                    drag.target: textOverlayBox
                    onPositionChanged: {
                        coverEditDialog.textElementX = textOverlayBox.x
                        coverEditDialog.textElementY = textOverlayBox.y
                    }
                }
            }
        }

        // Right Properties Panel (Advanced Inspector)
        Rectangle {
            Layout.preferredWidth: 340       
            Layout.fillHeight: true          
            color: "#18181c"
            radius: 12
            border.color: "#27272e"
            border.width: 1
            clip: true

            // Templates Panel
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: coverEditDialog.activeTab === "templates"
                spacing: 20

                Text {
                    text: "Cover Templates"
                    color: Theme.text
                    font.pixelSize: 16
                    font.weight: Font.Bold
                }

                ScrollView {
                    id: templateScrollView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ColumnLayout {
                        width: templateScrollView.availableWidth
                        spacing: 16

                        // Template 1: Vlog Style
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: "#202026"
                            radius: 10
                            border.color: "#303038"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text { 
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "VLOG DAILY"
                                    color: "#ffdd57"
                                    font.pixelSize: 20
                                    font.weight: Font.Bold 
                                }
                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Bold Yellow • Semi-transparent Dark"
                                    color: Theme.textMuted
                                    font.pixelSize: 11
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: { 
                                    coverEditDialog.coverText = "VLOG DAILY"
                                    coverEditDialog.coverTextColor = "#ffdd57"
                                    coverEditDialog.coverTextBg = "#99000000"
                                    coverEditDialog.coverTextSize = 38 
                                }
                            }
                        }

                        // Template 2: Gaming Neon
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: "#202026"
                            radius: 10
                            border.color: "#303038"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text { 
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "GAMING HIGHLIGHTS"
                                    color: "#25e5cf"
                                    font.pixelSize: 18
                                    font.weight: Font.Bold 
                                }
                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Cyan Neon • Translucent Cyan"
                                    color: Theme.textMuted
                                    font.pixelSize: 11
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: { 
                                    coverEditDialog.coverText = "GAMING HIGHLIGHTS"
                                    coverEditDialog.coverTextColor = "#25e5cf"
                                    coverEditDialog.coverTextBg = "#4025e5cf"
                                    coverEditDialog.coverTextSize = 34 
                                }
                            }
                        }

                        // Template 3: Cinematic
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: "#202026"
                            radius: 10
                            border.color: "#303038"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text { 
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "CINEMATIC TRAILER"
                                    color: "#ffffff"
                                    font.pixelSize: 18
                                    font.weight: Font.Bold
                                    font.letterSpacing: 2 
                                }
                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Clean White • Minimalist Transparent"
                                    color: Theme.textMuted
                                    font.pixelSize: 11
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: { 
                                    coverEditDialog.coverText = "CINEMATIC TRAILER"
                                    coverEditDialog.coverTextColor = "#ffffff"
                                    coverEditDialog.coverTextBg = "transparent"
                                    coverEditDialog.coverTextSize = 36 
                                }
                            }
                        }
                    }
                }
            }

            // Backend Core Panel
            ScrollView {
                id: backendScrollView
                anchors.fill: parent
                anchors.margins: 20
                visible: coverEditDialog.activeTab === "backend"
                clip: true

                ColumnLayout {
                    width: backendScrollView.availableWidth
                    spacing: 20

                    Text {
                        text: "Advanced C++ Backend Core"
                        color: Theme.text
                        font.pixelSize: 16
                        font.weight: Font.Bold
                    }
                    
                    Text {
                        text: "These modules run highly optimized multithreaded logic directly via the EditorBackend controller."
                        color: Theme.textMuted
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    BackendFeatures.GeglOperationsCard { Layout.fillWidth: true }
                    BackendFeatures.PaintDynamicsCard { Layout.fillWidth: true }
                    BackendFeatures.InteractiveToolsCard { Layout.fillWidth: true }
                    BackendFeatures.PdbPluginsCard { Layout.fillWidth: true }
                    BackendFeatures.XcfProjectCard { Layout.fillWidth: true }
                }
            }

            // Text Panel
            ScrollView {
                id: textScrollView
                anchors.fill: parent
                anchors.margins: 20
                visible: coverEditDialog.activeTab === "text"
                clip: true

                ColumnLayout {
                    width: textScrollView.availableWidth
                    spacing: 16

                    Text {
                        text: "Text Editing"
                        color: Theme.text
                        font.pixelSize: 16
                        font.weight: Font.Bold
                    }

                    TextField {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        color: Theme.text
                        font.pixelSize: 14
                        text: coverEditDialog.coverText
                        onTextEdited: coverEditDialog.coverText = text
                        background: Rectangle { color: "#121215"; radius: 8; border.color: "#27272e" }
                    }

                    Text { text: "Font Family"; color: Theme.textMuted; font.pixelSize: 13; Layout.topMargin: 8 }
                    GridLayout {
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 10
                        Layout.fillWidth: true
                        Button { 
                            text: "Inter"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.coverFontFamily === "Inter" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverFontFamily === "Inter" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverFontFamily = "Inter" 
                        }
                        Button { 
                            text: "Roboto"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.coverFontFamily === "Roboto" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverFontFamily === "Roboto" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverFontFamily = "Roboto" 
                        }
                        Button { 
                            text: "Cinzel"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.coverFontFamily === "Cinzel" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverFontFamily === "Cinzel" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverFontFamily = "Cinzel" 
                        }
                        Button { 
                            text: "Courier"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.coverFontFamily === "Courier" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverFontFamily === "Courier" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverFontFamily = "Courier" 
                        }
                    }

                    FeatureSlider {
                        title: "Font Size"
                        from: 18
                        to: 80
                        stepSize: 1
                        value: coverEditDialog.coverTextSize
                        valueSuffix: "px"
                        onValueModified: (newValue) => coverEditDialog.coverTextSize = newValue
                    }

                    FeatureSlider {
                        title: "Letter Spacing"
                        from: 0
                        to: 10
                        stepSize: 1
                        value: coverEditDialog.coverLetterSpacing
                        valueSuffix: ""
                        onValueModified: (newValue) => coverEditDialog.coverLetterSpacing = newValue
                    }

                    CoverColorPicker {
                        title: "Text Color"
                        selectedColor: coverEditDialog.coverTextColor
                        onColorSelected: (newColor) => coverEditDialog.coverTextColor = newColor
                    }

                    Text { text: "Text Case / Transform"; color: Theme.textMuted; font.pixelSize: 13; Layout.topMargin: 8 }
                    RowLayout {
                        spacing: 10
                        Button { 
                            text: "Normal"; implicitWidth: 80; implicitHeight: 36
                            background: Rectangle { color: coverEditDialog.coverTextTransform === "none" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverTextTransform === "none" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverTextTransform = "none" 
                        }
                        Button { 
                            text: "UPPER"; implicitWidth: 80; implicitHeight: 36
                            background: Rectangle { color: coverEditDialog.coverTextTransform === "uppercase" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverTextTransform === "uppercase" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverTextTransform = "uppercase" 
                        }
                        Button { 
                            text: "lower"; implicitWidth: 80; implicitHeight: 36
                            background: Rectangle { color: coverEditDialog.coverTextTransform === "lowercase" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverTextTransform === "lowercase" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverTextTransform = "lowercase" 
                        }
                    }

                    Text { text: "Background Style"; color: Theme.textMuted; font.pixelSize: 13; Layout.topMargin: 8 }
                    RowLayout {
                        spacing: 10
                        Button { 
                            text: "Dark"; implicitWidth: 80; implicitHeight: 36
                            background: Rectangle { color: coverEditDialog.coverTextBg === "#80000000" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverTextBg === "#80000000" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverTextBg = "#80000000" 
                        }
                        Button { 
                            text: "Neon"; implicitWidth: 80; implicitHeight: 36
                            background: Rectangle { color: coverEditDialog.coverTextBg === "#4025e5cf" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverTextBg === "#4025e5cf" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverTextBg = "#4025e5cf" 
                        }
                        Button { 
                            text: "None"; implicitWidth: 80; implicitHeight: 36
                            background: Rectangle { color: coverEditDialog.coverTextBg === "transparent" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.coverTextBg === "transparent" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.coverTextBg = "transparent" 
                        }
                    }

                    Text { text: "Text Effects & Styling"; color: Theme.textMuted; font.pixelSize: 13; Layout.topMargin: 8 }

                    FeatureSwitch {
                        text: "Text Shadow"
                        checked: coverEditDialog.coverTextShadow
                        onToggled: (isChecked) => coverEditDialog.coverTextShadow = isChecked
                    }

                    FeatureSwitch {
                        text: "Text Outline"
                        checked: coverEditDialog.coverTextOutline
                        onToggled: (isChecked) => coverEditDialog.coverTextOutline = isChecked
                    }

                    FeatureSwitch {
                        text: "Neon Glow Effect"
                        checked: coverEditDialog.coverTextGlow
                        onToggled: (isChecked) => coverEditDialog.coverTextGlow = isChecked
                    }

                    Item { Layout.preferredHeight: 20 }
                }
            }

            // Stickers Panel
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                visible: coverEditDialog.activeTab === "stickers"
                spacing: 16

                Text {
                    text: "Stickers"
                    color: Theme.text
                    font.pixelSize: 16
                    font.weight: Font.Bold
                }

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 20
                    Layout.alignment: Qt.AlignHCenter
                    Rectangle { 
                        width: 110; height: 110; color: "#202026"; radius: 12
                        border.color: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-fire.svg" ? "#ffffff" : "#303038"
                        border.width: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-fire.svg" ? 2 : 1
                        Image { anchors.centerIn: parent; source: "qrc:/VideoStudioUI/assets/sticker-fire.svg"; sourceSize: Qt.size(48, 48) } 
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: coverEditDialog.activeSticker = "qrc:/VideoStudioUI/assets/sticker-fire.svg" } 
                    }
                    Rectangle { 
                        width: 110; height: 110; color: "#202026"; radius: 12
                        border.color: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-sparkle.svg" ? "#ffffff" : "#303038"
                        border.width: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-sparkle.svg" ? 2 : 1
                        Image { anchors.centerIn: parent; source: "qrc:/VideoStudioUI/assets/sticker-sparkle.svg"; sourceSize: Qt.size(48, 48) } 
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: coverEditDialog.activeSticker = "qrc:/VideoStudioUI/assets/sticker-sparkle.svg" } 
                    }
                    Rectangle { 
                        width: 110; height: 110; color: "#202026"; radius: 12
                        border.color: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-boom.svg" ? "#ffffff" : "#303038"
                        border.width: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-boom.svg" ? 2 : 1
                        Image { anchors.centerIn: parent; source: "qrc:/VideoStudioUI/assets/sticker-boom.svg"; sourceSize: Qt.size(48, 48) } 
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: coverEditDialog.activeSticker = "qrc:/VideoStudioUI/assets/sticker-boom.svg" } 
                    }
                    Rectangle { 
                        width: 110; height: 110; color: "#202026"; radius: 12
                        border.color: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-pin.svg" ? "#ffffff" : "#303038"
                        border.width: coverEditDialog.activeSticker === "qrc:/VideoStudioUI/assets/sticker-pin.svg" ? 2 : 1
                        Image { anchors.centerIn: parent; source: "qrc:/VideoStudioUI/assets/sticker-pin.svg"; sourceSize: Qt.size(48, 48) } 
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: coverEditDialog.activeSticker = "qrc:/VideoStudioUI/assets/sticker-pin.svg" } 
                    }
                }

                FeatureSlider {
                    title: "Sticker Size"
                    from: 0.5
                    to: 2.5
                    stepSize: 0.1
                    value: coverEditDialog.stickerScale
                    valueSuffix: "x"
                    onValueModified: (newValue) => coverEditDialog.stickerScale = newValue
                }

                FeatureSlider {
                    title: "Sticker Opacity"
                    from: 0.2
                    to: 1.0
                    stepSize: 0.05
                    value: coverEditDialog.stickerOpacity
                    valueSuffix: ""
                    onValueModified: (newValue) => coverEditDialog.stickerOpacity = newValue
                }

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    text: "Clear Sticker"
                    background: Rectangle { color: "#303038"; radius: 8 }
                    contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 14; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: coverEditDialog.activeSticker = ""
                }

                Item { Layout.fillHeight: true }
            }

            // Effects Panel
            ScrollView {
                anchors.fill: parent
                anchors.margins: 20
                visible: coverEditDialog.activeTab === "effects"
                contentWidth: width
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    Text {
                        text: "Cover Effects & Filters"
                        color: Theme.text
                        font.pixelSize: 16
                        font.weight: Font.Bold
                    }

                    Text { text: "Color Filters"; color: Theme.textMuted; font.pixelSize: 13 }
                    GridLayout {
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 10
                        Layout.fillWidth: true
                        Button { 
                            text: "None"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.activeFilter === "none" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.activeFilter === "none" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.activeFilter = "none" 
                        }
                        Button { 
                            text: "Warm"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.activeFilter === "warm" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.activeFilter === "warm" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.activeFilter = "warm" 
                        }
                        Button { 
                            text: "Cool"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.activeFilter === "cool" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.activeFilter === "cool" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.activeFilter = "cool" 
                        }
                        Button { 
                            text: "Vintage"; Layout.fillWidth: true; Layout.preferredHeight: 36
                            background: Rectangle { color: coverEditDialog.activeFilter === "vintage" ? "#222228" : "#121215"; radius: 6; border.color: coverEditDialog.activeFilter === "vintage" ? "#ffffff" : "#27272e" }
                            contentItem: Text { text: parent.text; color: Theme.text; font.pixelSize: 13; font.weight: Font.Medium; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: coverEditDialog.activeFilter = "vintage" 
                        }
                    }

                    Text { text: "AI & Advanced Features"; color: Theme.textMuted; font.pixelSize: 13; Layout.topMargin: 8 }

                    FeatureSlider {
                        title: "Cover Image Scale"
                        from: 0.5
                        to: 2.0
                        stepSize: 0.05
                        value: coverEditDialog.coverImageScale
                        valueSuffix: "x"
                        onValueModified: (newValue) => coverEditDialog.coverImageScale = newValue
                    }

                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 46
                        text: gimpProcessor.isProcessing ? "Opening in GIMP..." : "Edit in GIMP (External Studio)"
                        enabled: !gimpProcessor.isProcessing
                        background: Rectangle { 
                            color: gimpProcessor.isProcessing ? "#303038" : "#38bdf8"
                            radius: 8 
                        }
                        contentItem: Text { 
                            text: parent.text
                            color: gimpProcessor.isProcessing ? Theme.textMuted : "#0f172a"
                            font.pixelSize: 14
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter 
                        }
                        onClicked: {
                            gimpProcessor.exportToGimp(coverEditDialog.coverImagePath)
                        }
                    }

                    FeatureSwitch {
                        text: "Remove Background"
                        checked: coverEditDialog.removeBackground
                        onToggled: (isChecked) => {
                            coverEditDialog.removeBackground = isChecked
                            if (isChecked) {
                                gimpProcessor.removeBackground(coverEditDialog.coverImagePath)
                            }
                        }
                    }

                    FeatureSwitch {
                        text: "Smart Cutout"
                        checked: coverEditDialog.smartCutout
                        onToggled: (isChecked) => coverEditDialog.smartCutout = isChecked
                    }

                    FeatureSwitch {
                        text: "AI 4K Upscale"
                        checked: coverEditDialog.aiUpscale
                        onToggled: (isChecked) => coverEditDialog.aiUpscale = isChecked
                    }

                    FeatureSwitch {
                        text: "Object Outline"
                        checked: coverEditDialog.objectOutline
                        onToggled: (isChecked) => coverEditDialog.objectOutline = isChecked
                    }

                    FeatureSwitch {
                        text: "Glossy Reflection"
                        checked: coverEditDialog.glossyReflection
                        onToggled: (isChecked) => coverEditDialog.glossyReflection = isChecked
                    }

                    Text { text: "Styling & Overlays"; color: Theme.textMuted; font.pixelSize: 13; Layout.topMargin: 8 }

                    FeatureSwitch {
                        text: "Film Grain"
                        checked: coverEditDialog.hasFilmGrain
                        onToggled: (isChecked) => coverEditDialog.hasFilmGrain = isChecked
                    }

                    FeatureSwitch {
                        text: "Vignette Effect"
                        checked: coverEditDialog.hasVignette
                        onToggled: (isChecked) => coverEditDialog.hasVignette = isChecked
                    }

                    FeatureSwitch {
                        text: "Neon Border"
                        checked: coverEditDialog.hasNeonBorder
                        onToggled: (isChecked) => coverEditDialog.hasNeonBorder = isChecked
                    }

                    Item { Layout.preferredHeight: 30 }
                }
            }
        }
    }

    footer: Rectangle {
        color: "#18181c"
        height: 72

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: "#27272e"
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24

            Button {
                implicitWidth: 160
                implicitHeight: 42
                hoverEnabled: true
                HoverHandler { cursorShape: Qt.PointingHandCursor }
                background: Rectangle {
                    color: parent.hovered ? "#33333c" : "#222228"
                    radius: 8
                    border.color: "#3a3a44"
                    border.width: 1
                }
                contentItem: RowLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Image {
                        source: "qrc:/VideoStudioUI/assets/arrow-left.svg"
                        sourceSize: Qt.size(16, 16)
                    }
                    Text {
                        text: "Back to Select"
                        color: Theme.text
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }
                }
                onClicked: {
                    coverEditDialog.close()
                    if (parentSelectDialog) parentSelectDialog.open()
                }
            }

            Item { Layout.fillWidth: true }

            RowLayout {
                spacing: 16

                Button {
                    text: "Save Cover"
                    implicitWidth: 140
                    implicitHeight: 42
                    hoverEnabled: true
                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                    background: Rectangle {
                        color: parent.hovered ? "#38ffe8" : "#25e5cf"
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#000000"
                        font.pixelSize: 15
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        coverEditDialog.accept()
                        coverEditDialog.close()
                    }
                }

                Button {
                    text: "Cancel"
                    implicitWidth: 100
                    implicitHeight: 42
                    hoverEnabled: true
                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                    background: Rectangle {
                        color: parent.hovered ? "#44444e" : "#303038"
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.text
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: coverEditDialog.close()
                }
            }
        }
    }
}
