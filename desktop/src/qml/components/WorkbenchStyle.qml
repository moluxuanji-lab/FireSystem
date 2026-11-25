pragma Singleton
import QtQuick
import FluentUI

QtObject {
    readonly property color ink: FluTheme.dark ? "#e5edf7" : "#192a42"
    readonly property color muted: FluTheme.dark ? "#acb9c9" : "#607087"
    readonly property color accent: FluTheme.dark ? "#89b9ff" : "#2563b1"
    readonly property color line: FluTheme.dark ? "#354455" : "#dce3eb"
    readonly property color panel: FluTheme.dark ? "#202c39" : "#ffffff"
    readonly property color surface: FluTheme.dark ? "#17212c" : "#f5f7fa"
    readonly property color rail: FluTheme.dark ? "#1b2734" : "#eef2f7"
    readonly property color tint: FluTheme.dark ? "#263d59" : "#e6effb"
    readonly property color hover: FluTheme.dark ? "#2b394b" : "#eff4fa"
    readonly property color good: FluTheme.dark ? "#8cd7b7" : "#1a7964"
    readonly property color danger: FluTheme.dark ? "#ffb7af" : "#b74642"
}
