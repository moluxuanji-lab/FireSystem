import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    property string title: ""
    property string glyph: ""
    property Component headerAction: null
    default property alias content: bodyColumn.data
    readonly property int inset: width < 400 ? 15 : 20

    radius: 10
    implicitHeight: mainColumn.implicitHeight + inset * 2
    color: WorkbenchStyle.panel
    border.color: WorkbenchStyle.line

    ColumnLayout {
        id: mainColumn
        x: root.inset
        y: root.inset
        width: root.width - 2 * root.inset
        spacing: 15
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: root.title !== "" || root.headerAction !== null
            WorkbenchIcon { visible: root.glyph !== ""; glyph: root.glyph; ink: WorkbenchStyle.accent; size: 19 }
            Text {
                Layout.fillWidth: true
                text: root.title
                font.pixelSize: 17
                font.weight: Font.DemiBold
                color: WorkbenchStyle.ink
                wrapMode: Text.WordWrap
            }
            Loader { sourceComponent: root.headerAction; visible: root.headerAction !== null }
        }
        ColumnLayout {
            id: bodyColumn
            Layout.fillWidth: true
            spacing: 15
        }
    }
}
