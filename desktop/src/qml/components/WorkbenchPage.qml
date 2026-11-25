import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

Item {
    id: page
    default property alias content: column.data
    readonly property real viewportWidth: Window.window ? Window.window.width : width
    readonly property int inset: viewportWidth <= 760 ? 12 : viewportWidth <= 1000 ? 15
        : viewportWidth <= 1250 ? 20 : Math.min(40, Math.round(viewportWidth * 0.0225))

    Flickable {
        id: scroll
        anchors.fill: parent
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        contentHeight: column.y + column.implicitHeight + 32
        ScrollBar.vertical: FluScrollBar {}

        ColumnLayout {
            id: column
            x: (scroll.width - width) / 2
            y: page.viewportWidth <= 760 ? 14 : page.viewportWidth <= 1000 ? 18 : page.viewportWidth >= 1550 ? 27 : 22
            width: Math.max(0, Math.min(1670, scroll.width) - 2 * page.inset)
            spacing: 16
        }
    }
}
