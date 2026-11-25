import QtQuick

Rectangle {
    id: chip
    property string text: ""
    property string tone: "accent"
    implicitWidth: label.implicitWidth + 18
    implicitHeight: 23
    radius: 12
    color: tone === "muted" ? WorkbenchStyle.surface : WorkbenchStyle.tint
    Text {
        id: label
        anchors.centerIn: parent
        text: chip.text
        font.pixelSize: 12
        font.weight: Font.DemiBold
        color: chip.tone === "muted" ? WorkbenchStyle.muted : WorkbenchStyle.accent
    }
}
