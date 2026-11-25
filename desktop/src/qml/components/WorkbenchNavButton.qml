import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AbstractButton {
    id: button
    required property string glyph
    property bool selected: false
    property bool compact: false
    property bool card: false
    implicitHeight: card ? 54 : 37
    leftPadding: card ? 18 : 13
    rightPadding: leftPadding
    hoverEnabled: true
    Accessible.name: text
    ToolTip.visible: compact && hovered
    ToolTip.text: text

    background: Rectangle {
        radius: button.card ? 8 : 6
        color: button.selected ? Qt.tint(WorkbenchStyle.rail, Qt.alpha(WorkbenchStyle.accent, 0.12))
            : button.hovered ? WorkbenchStyle.hover : button.card ? WorkbenchStyle.panel : "transparent"
        border.color: button.visualFocus ? WorkbenchStyle.accent : button.card ? WorkbenchStyle.line : "transparent"
        Rectangle {
            visible: button.selected
            anchors.verticalCenter: parent.verticalCenter
            width: 3
            height: 17
            radius: 2
            color: WorkbenchStyle.accent
        }
    }

    contentItem: RowLayout {
        spacing: button.card ? 14 : 13
        WorkbenchIcon {
            Layout.alignment: button.compact ? Qt.AlignHCenter : Qt.AlignLeft
            glyph: button.glyph
            ink: button.card || button.selected ? WorkbenchStyle.accent : WorkbenchStyle.muted
        }
        Text {
            Layout.fillWidth: true
            visible: !button.compact
            text: button.text
            font.pixelSize: button.card ? 15 : 14
            font.weight: button.selected ? Font.DemiBold : Font.Normal
            color: button.selected ? WorkbenchStyle.accent : WorkbenchStyle.ink
            elide: Text.ElideRight
        }
        WorkbenchIcon { visible: button.card; glyph: "chevron"; size: 14 }
    }
}
