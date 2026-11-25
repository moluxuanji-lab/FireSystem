pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import FluentUI
import "../components"

FluScrollablePage {
    id: page

    property var service: null
    property var hostWindow: null

    readonly property var i18n: service ? service.i18n : null

    function t(key, arg0) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision, arg0 === undefined ? "" : String(arg0))
    }

    spacing: 16
    leftPadding: 28
    rightPadding: 28
    topPadding: 22
    bottomPadding: 28

    FileDialog {
        id: exportSettingsDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.exportSettingsToFile(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: importSettingsDialog
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.prepareSettingsImportFromFile(selectedFile.toString())
            }
        }
    }

    PageHeader {
        service: page.service
        pageTitle: page.t("设置")
        subtitle: page.t("管理工作站外观、账号、模板与版本。")
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.maximumWidth: 920
        spacing: 10

        Repeater {
            model: [
                { id: "settings-accounts", label: "账号", icon: FluentIcons.People, gated: false, objName: "navCardAccounts" },
                { id: "settings-license", label: "许可证", icon: FluentIcons.Permissions, gated: false, objName: "navCardLicense" },
                { id: "settings-templates", label: "项目模板", icon: FluentIcons.Folder, gated: true, objName: "navCardTemplates" },
                { id: "settings-general", label: "通用", icon: FluentIcons.Design, gated: false, objName: "navCardGeneral" },
                { id: "settings-about", label: "关于", icon: FluentIcons.Info, gated: false, objName: "navCardAbout" }
            ]

            delegate: FluControl {
                id: navCard
                required property var modelData

                objectName: navCard.modelData.objName
                visible: !navCard.modelData.gated || (page.service ? page.service.unlocked : false)
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? 56 : 0

                background: Rectangle {
                    radius: 8
                    color: navCard.hovered
                        ? (FluTheme.dark ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.03))
                        : FluTheme.frameActiveColor
                    border.color: FluTheme.dividerColor
                    border.width: 1
                }

                onClicked: {
                    if (page.service) {
                        page.service.navigateTo(navCard.modelData.id)
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    spacing: 12

                    FluIcon {
                        iconSource: navCard.modelData.icon
                        iconSize: 18
                        iconColor: FluTheme.primaryColor
                    }

                    FluText {
                        Layout.fillWidth: true
                        text: page.t(navCard.modelData.label)
                        font: FluTextStyle.BodyStrong
                    }

                    FluIcon {
                        iconSource: FluentIcons.ChevronRight
                        iconSize: 14
                        iconColor: FluTheme.fontSecondaryColor
                    }
                }
            }
        }
    }

    SectionCard {
        id: settingsTransferCard
        objectName: "settingsTransferCard"
        Layout.fillWidth: true
        Layout.maximumWidth: 920
        visible: page.service ? page.service.canManage : false

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            FluButton {
                id: importSettingsBtn
                objectName: "importSettingsBtn"
                text: page.t("导入设置")
                onClicked: importSettingsDialog.open()
            }

            FluButton {
                id: exportSettingsBtn
                objectName: "exportSettingsBtn"
                text: page.t("导出设置")
                onClicked: {
                    if (page.service) {
                        exportSettingsDialog.currentFile = "file:///" + page.service.suggestedExportFileName("settings")
                        exportSettingsDialog.open()
                    }
                }
            }
        }

        FluFrame {
            id: confirmSettingsImportCard
            objectName: "confirmSettingsImportCard"
            Layout.fillWidth: true
            visible: page.service ? page.service.hasPendingSettingsImport : false
            padding: 16
            radius: 8
            border.color: FluTheme.dividerColor
            color: FluTheme.dark ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.025)

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                FluText {
                    text: page.t("确认导入设置")
                    font: FluTextStyle.Subtitle
                }

                FluText {
                    Layout.fillWidth: true
                    visible: page.service ? page.service.pendingImportHasAppearance : false
                    text: page.t("将应用文件中的语言和主题。")
                    font: FluTextStyle.Body
                    textColor: FluTheme.fontSecondaryColor
                    wrapMode: Text.WordWrap
                }

                FluText {
                    Layout.fillWidth: true
                    text: page.t("将用文件中的 {0} 个账号替换本机的操作员和管理员账号，并退出登录。",
                                 page.service ? page.service.pendingImportAccountCount : 0)
                    font: FluTextStyle.Body
                    textColor: FluTheme.fontSecondaryColor
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.topMargin: 4
                    spacing: 10

                    FluFilledButton {
                        id: confirmSettingsImportBtn
                        objectName: "confirmSettingsImportBtn"
                        text: page.t("确认导入")
                        onClicked: {
                            if (page.service) {
                                page.service.confirmSettingsImport()
                            }
                        }
                    }

                    FluButton {
                        objectName: "cancelSettingsImportBtn"
                        text: page.t("取消")
                        onClicked: {
                            if (page.service) {
                                page.service.cancelSettingsImport()
                            }
                        }
                    }
                }
            }
        }
    }
}
