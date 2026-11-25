import QtQuick
import QtQuick.Layouts
import FluentUI

Rectangle {
    id: root

    property string text: ""
    property string kind: "info" // "info" | "warning" | "error"

    readonly property color barColor: {
        if (kind === "error") return FluTheme.dark ? FluColors.Red.lighter : FluColors.Red.dark
        if (kind === "warning") return FluTheme.dark ? FluColors.Orange.lighter : FluColors.Orange.dark
        return FluTheme.primaryColor
    }

    implicitHeight: row.implicitHeight + 22
    radius: 6
    color: FluTheme.dark ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.03)
    border.color: FluTheme.dividerColor
    border.width: 1

    Rectangle {
        width: 3
        radius: 1.5
        color: root.barColor
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 1
    }

    RowLayout {
        id: row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 14
        anchors.rightMargin: 13
        spacing: 10

        FluIcon {
            iconSource: root.kind === "error"
                ? FluentIcons.ErrorBadge
                : (root.kind === "warning" ? FluentIcons.Warning : FluentIcons.Info)
            iconSize: 16
            iconColor: root.barColor
        }

        FluText {
            Layout.fillWidth: true
            text: root.text
            font: FluTextStyle.Body
            wrapMode: Text.WordWrap
        }
    }
}
