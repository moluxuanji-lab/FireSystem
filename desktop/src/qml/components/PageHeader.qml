import QtQuick
import QtQuick.Layouts
import FluentUI

ColumnLayout {
    id: root

    property var service: null
    property string parentLabel: ""
    property string parentPage: ""
    property string pageTitle: ""
    property string subtitle: ""

    readonly property var i18n: service ? service.i18n : null

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    Layout.fillWidth: true
    spacing: 6

    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        FluTextButton {
            text: "FireSystem"
            font: FluTextStyle.Caption
            padding: 2
            onClicked: {
                if (root.service) {
                    root.service.navigateTo("settings")
                }
            }
        }

        FluIcon {
            iconSource: FluentIcons.ChevronRight
            iconSize: 10
            iconColor: FluTheme.fontSecondaryColor
        }

        FluTextButton {
            visible: root.parentLabel !== ""
            text: root.parentLabel
            font: FluTextStyle.Caption
            padding: 2
            onClicked: {
                if (root.service && root.parentPage !== "") {
                    root.service.navigateTo(root.parentPage)
                }
            }
        }

        FluIcon {
            visible: root.parentLabel !== ""
            iconSource: FluentIcons.ChevronRight
            iconSize: 10
            iconColor: FluTheme.fontSecondaryColor
        }

        FluText {
            text: root.pageTitle
            font: FluTextStyle.Caption
            textColor: FluTheme.fontPrimaryColor
        }
    }

    FluText {
        Layout.fillWidth: true
        text: root.pageTitle
        font: FluTextStyle.Title
        wrapMode: Text.WordWrap
    }

    FluText {
        Layout.fillWidth: true
        visible: root.subtitle !== ""
        text: root.subtitle
        font: FluTextStyle.Body
        textColor: FluTheme.fontSecondaryColor
        wrapMode: Text.WordWrap
    }
}
