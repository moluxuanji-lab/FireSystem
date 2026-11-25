pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import FluentUI

ColumnLayout {
    id: root

    property var items: []

    spacing: 0

    Repeater {
        model: root.items

        delegate: ColumnLayout {
            id: kvDelegate
            required property var modelData
            required property int index

            Layout.fillWidth: true
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                Layout.bottomMargin: 10
                spacing: 16

                FluText {
                    Layout.preferredWidth: 160
                    Layout.minimumWidth: 90
                    text: String(kvDelegate.modelData.label || "")
                    font: FluTextStyle.Body
                    textColor: FluTheme.fontSecondaryColor
                    wrapMode: Text.WordWrap
                }

                FluText {
                    Layout.fillWidth: true
                    text: String(kvDelegate.modelData.value || "")
                    font: kvDelegate.modelData.monospace
                        ? Qt.font({ family: "Consolas", pixelSize: 13 })
                        : FluTextStyle.BodyStrong
                    wrapMode: Text.WrapAnywhere
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: FluTheme.dividerColor
                visible: kvDelegate.index < root.items.length - 1
            }
        }
    }
}
