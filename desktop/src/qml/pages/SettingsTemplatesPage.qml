pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

WorkbenchPage {
    id: page

    property var service: null

    readonly property var i18n: service ? service.i18n : null

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    FileDialog {
        id: exportManifestDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.exportTemplateReviewToFile(selectedFile.toString())
            }
        }
    }

    PageHeader {
        service: page.service
        parentLabel: page.t("设置")
        parentPage: "settings"
        pageTitle: page.t("项目模板")
    }

    SectionCard {
        Layout.fillWidth: true

        KeyValueList {
            Layout.fillWidth: true
            items: [
                {
                    label: page.t("当前模板"),
                    value: page.service ? page.service.templateId : "sim.baseline_ring"
                },
                {
                    label: page.t("适用板型"),
                    value: page.service ? page.service.templateBoardId : "kzb_f407"
                },
                {
                    label: page.t("评审状态"),
                    value: page.t(page.service ? page.service.templateReviewStatus : "待复核")
                }
            ]
        }

        RowLayout {
            Layout.topMargin: 4
            spacing: 10

            WorkbenchButton {
                id: recordReviewBtn
                objectName: "recordReviewBtn"
                text: page.t("记录模板复核")
                disabled: !(page.service && page.service.canManage && page.service.templateReviewStatus !== "已复核")
                onClicked: {
                    if (page.service) {
                        page.service.recordTemplateReview()
                    }
                }
            }

            WorkbenchButton {
                id: exportManifestBtn
                objectName: "exportManifestBtn"
                text: page.t("导出评审清单")
                onClicked: {
                    if (page.service) {
                        exportManifestDialog.currentFile = page.service.suggestedExportFileUrl(
                            page.service.suggestedExportFileName("template-review")
                        )
                        exportManifestDialog.open()
                    }
                }
            }
        }
    }
}
