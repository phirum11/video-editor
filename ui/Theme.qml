// qmllint disable
pragma Singleton

import QtQuick

QtObject {
    property bool isDarkMode: true

    property color background: isDarkMode ? "#242423" : "#f5f5f5"
    property color surface: isDarkMode ? "#242423" : text
    property color surfaceRaised: isDarkMode ? "#2b2b2a" : "#f0f0f0"
    property color surfaceHover: isDarkMode ? "#30302f" : "#e4e4e4"
    property color surfacePressed: isDarkMode ? "#363634" : "#d8d8d8"
    property color surfaceInset: isDarkMode ? "#1b1b1a" : "#e8e8e8"
    property color surfaceDeep: isDarkMode ? "#111111" : "#e0e0e0"
    property color divider: isDarkMode ? "#3a3a38" : "#cccccc"
    property color dividerSoft: isDarkMode ? "#30302f" : "#dddddd"
    property color text: isDarkMode ? "#f0f8fa" : "#222222"
    property color textMuted: isDarkMode ? "#8fa2aa" : "#666666"
    property color accent: "#5ec4e8"
}
