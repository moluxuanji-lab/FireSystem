import QtQuick
import "WorkbenchGlyphs.js" as Glyphs

Item {
    id: icon
    required property string glyph
    property color ink: WorkbenchStyle.muted
    property int size: 18
    implicitWidth: size
    implicitHeight: size

    Image {
        anchors.fill: parent
        sourceSize: Qt.size(icon.size * Screen.devicePixelRatio, icon.size * Screen.devicePixelRatio)
        source: "data:image/svg+xml;charset=utf-8," + encodeURIComponent(
            '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"' +
            ' fill="none" stroke="' + icon.ink + '" stroke-width="1.65" stroke-linecap="round" stroke-linejoin="round">' +
            (Glyphs.paths[icon.glyph] || "") + '</svg>')
    }
}
