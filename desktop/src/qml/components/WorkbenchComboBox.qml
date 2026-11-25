import QtQuick
import QtQuick.Controls

ComboBox {
    id: control
    implicitHeight: 35
    leftPadding: 10
    rightPadding: 30
    font.pixelSize: 14
    palette.text: WorkbenchStyle.ink
    palette.windowText: WorkbenchStyle.ink
    palette.window: WorkbenchStyle.panel
    palette.base: WorkbenchStyle.panel
    palette.highlight: WorkbenchStyle.tint
    palette.highlightedText: WorkbenchStyle.ink
    background: Rectangle {
        radius: 5
        border.color: control.visualFocus ? WorkbenchStyle.accent : WorkbenchStyle.line
        color: WorkbenchStyle.panel
    }
    contentItem: Text {
        text: control.displayText
        font: control.font
        color: WorkbenchStyle.ink
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: WorkbenchIcon {
        x: control.width - width - 10
        anchors.verticalCenter: parent.verticalCenter
        glyph: "expand"
        size: 12
    }
}
