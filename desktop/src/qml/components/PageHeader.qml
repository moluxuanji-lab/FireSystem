import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    property var service
    property string parentLabel: ""
    property string parentPage: ""
    property string pageTitle: ""
    Layout.fillWidth: true
    Layout.bottomMargin: 5
    spacing: 13

    component Crumb: AbstractButton {
        id: link
        implicitWidth: label.implicitWidth
        implicitHeight: 18
        contentItem: Text {
            id: label
            text: link.text
            color: WorkbenchStyle.accent
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 12
            font.underline: link.hovered || link.visualFocus
        }
    }

    RowLayout {
        spacing: 8
        Crumb {
            text: "FireSystem"
            onClicked: if (root.service) root.service.navigateTo(root.service.unlocked ? "overview" : root.service.accessLanding())
        }
        WorkbenchIcon { glyph: "chevron"; size: 11 }
        Crumb {
            visible: root.parentLabel !== ""
            text: root.parentLabel
            onClicked: if (root.service) root.service.navigateTo(root.parentPage)
        }
        WorkbenchIcon { visible: root.parentLabel !== ""; glyph: "chevron"; size: 11 }
        Text { text: root.pageTitle; color: WorkbenchStyle.ink; font.pixelSize: 12 }
    }
    Text {
        Layout.fillWidth: true
        text: root.pageTitle
        color: WorkbenchStyle.ink
        font.pixelSize: Math.max(25, Math.min(34, Math.round((Window.window ? Window.window.width : root.width) * 0.021)))
        font.weight: Font.DemiBold
        wrapMode: Text.WordWrap
    }
}
