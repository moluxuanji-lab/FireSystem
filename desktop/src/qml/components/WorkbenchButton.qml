import QtQuick
import QtQuick.Controls
import FluentUI

AbstractButton {
    id: button
    property bool primary: false
    property bool danger: false
    property bool disabled: false
    enabled: !disabled
    hoverEnabled: true
    opacity: enabled ? 1 : 0.48
    horizontalPadding: 12
    verticalPadding: 7
    implicitWidth: label.implicitWidth + horizontalPadding * 2
    implicitHeight: Math.max(33, label.implicitHeight + verticalPadding * 2)
    background: Rectangle {
        radius: 5
        color: button.primary ? (button.down || button.hovered ? Qt.darker(WorkbenchStyle.accent, 1.12) : WorkbenchStyle.accent)
            : button.down || button.hovered ? WorkbenchStyle.hover : WorkbenchStyle.panel
        border.color: button.primary || button.visualFocus ? WorkbenchStyle.accent : WorkbenchStyle.line
        border.width: button.visualFocus ? 2 : 1
    }
    contentItem: Text {
        id: label
        text: button.text
        font.pixelSize: 13
        font.weight: Font.Medium
        color: button.danger ? WorkbenchStyle.danger : button.primary ? (FluTheme.dark ? "#132437" : "white") : WorkbenchStyle.ink
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
    }
}
