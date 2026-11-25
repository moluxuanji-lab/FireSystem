import QtQuick
import QtQuick.Layouts
import FluentUI

Rectangle {
    id: root

    property string text: ""
    property string tone: "accent" // "accent" | "good" | "warn" | "error" | "muted"

    readonly property color pillTextColor: {
        if (tone === "error") return FluTheme.dark ? FluColors.Red.lighter : FluColors.Red.dark
        if (tone === "warn") return FluTheme.dark ? FluColors.Orange.lighter : FluColors.Orange.dark
        if (tone === "good") return FluTheme.dark ? FluColors.Green.lighter : FluColors.Green.dark
        if (tone === "muted") return FluTheme.fontSecondaryColor
        return FluTheme.primaryColor
    }

    readonly property color pillBgColor: {
        if (tone === "muted") return FluTheme.dark ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.05)
        return FluTools.withOpacity(pillTextColor, 0.14)
    }

    implicitWidth: contentRow.implicitWidth + 18
    implicitHeight: 24
    radius: 12
    color: pillBgColor

    RowLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6

        Rectangle {
            Layout.preferredWidth: 7
            Layout.preferredHeight: 7
            radius: 3.5
            color: root.pillTextColor
        }

        FluText {
            text: root.text
            font: FluTextStyle.Caption
            textColor: root.pillTextColor
        }
    }
}
