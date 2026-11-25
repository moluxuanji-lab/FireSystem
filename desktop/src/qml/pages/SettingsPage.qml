pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import FluentUI
import "../components"

WorkbenchPage {
    id: page

    property var service: null

    readonly property var i18n: service ? service.i18n : null

    function t(key, arg0) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision, arg0 === undefined ? "" : String(arg0))
    }

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
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8
        Repeater {
            model: page.service ? page.service.navigationPages.filter(p => p.section === "settings") : []
            delegate: WorkbenchNavButton {
                required property var modelData
                readonly property string suffix: modelData.id.slice("settings-".length)
                objectName: "navCard" + suffix.charAt(0).toUpperCase() + suffix.slice(1)
                Layout.fillWidth: true
                card: true
                visible: modelData.id !== "settings-templates" || page.service.unlocked
                glyph: modelData.glyph
                text: page.t(modelData.label)
                onClicked: page.service.navigateTo(modelData.id)
            }
        }
    }

    SectionCard {
        id: settingsTransferCard
        objectName: "settingsTransferCard"
        Layout.fillWidth: true
        visible: page.service ? page.service.canManage : false

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            WorkbenchButton {
                id: importSettingsBtn
                objectName: "importSettingsBtn"
                text: page.t("导入设置")
                onClicked: importSettingsDialog.open()
            }

            WorkbenchButton {
                id: exportSettingsBtn
                objectName: "exportSettingsBtn"
                text: page.t("导出设置")
                onClicked: {
                    if (page.service) {
                        exportSettingsDialog.currentFile = page.service.suggestedExportFileUrl(
                            page.service.suggestedExportFileName("settings")
                        )
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
            border.color: WorkbenchStyle.line
            color: WorkbenchStyle.surface

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
                    textColor: WorkbenchStyle.muted
                    wrapMode: Text.WordWrap
                }

                FluText {
                    Layout.fillWidth: true
                    text: page.t("将用文件中的 {0} 个账号替换本机的操作员和管理员账号，并退出登录。",
                                 page.service ? page.service.pendingImportAccountCount : 0)
                    font: FluTextStyle.Body
                    textColor: WorkbenchStyle.muted
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.topMargin: 4
                    spacing: 10

                    WorkbenchButton {
                        primary: true
                        id: confirmSettingsImportBtn
                        objectName: "confirmSettingsImportBtn"
                        text: page.t("确认导入")
                        onClicked: {
                            if (page.service) {
                                page.service.confirmSettingsImport()
                            }
                        }
                    }

                    WorkbenchButton {
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
