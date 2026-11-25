pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: root
    property var items: []
    spacing: 0

    Repeater {
        model: root.items
        delegate: Item {
            id: entry
            required property var modelData
            required property int index
            Layout.fillWidth: true
            implicitHeight: Math.max(label.implicitHeight, value.implicitHeight) + 21

            Text {
                id: label
                y: 10
                width: Math.max(90, (entry.width - 8) * 0.375)
                text: entry.modelData.label
                font.pixelSize: 14
                color: WorkbenchStyle.muted
                wrapMode: Text.Wrap
            }
            Text {
                id: value
                y: 10
                anchors.left: label.right
                anchors.leftMargin: 8
                anchors.right: parent.right
                text: entry.modelData.value
                font.pixelSize: 14
                color: WorkbenchStyle.ink
                wrapMode: Text.Wrap
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: WorkbenchStyle.line
                visible: entry.index < root.items.length - 1
            }
        }
    }
}
