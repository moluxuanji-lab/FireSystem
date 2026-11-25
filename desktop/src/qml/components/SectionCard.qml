import QtQuick
import QtQuick.Layouts
import FluentUI

FluFrame {
    id: root

    property string title: ""
    property int iconSource: 0
    property string badgeText: ""
    property bool badgeMuted: false
    property Component headerAction: null
    default property alias content: bodyColumn.data

    radius: 8
    padding: 20
    border.color: FluTheme.dividerColor
    color: FluTheme.frameActiveColor

    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: root.title !== "" || root.badgeText !== "" || root.headerAction !== null

            FluIcon {
                visible: root.iconSource !== 0
                iconSource: root.iconSource
                iconSize: 18
                iconColor: FluTheme.primaryColor
            }

            FluText {
                Layout.fillWidth: true
                visible: root.title !== ""
                text: root.title
                font: FluTextStyle.Subtitle
                wrapMode: Text.WordWrap
            }

            StatusChip {
                visible: root.badgeText !== ""
                text: root.badgeText
                tone: root.badgeMuted ? "muted" : "accent"
            }

            FluLoader {
                sourceComponent: root.headerAction
                visible: root.headerAction !== null
            }
        }

        ColumnLayout {
            id: bodyColumn
            Layout.fillWidth: true
            spacing: 12
        }
    }
}
