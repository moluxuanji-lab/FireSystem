pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import "../components"

WorkbenchPage {
    id: page

    property var service: null

    readonly property var i18n: service ? service.i18n : null
    property bool showTrustDetails: false

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    PageHeader {
        service: page.service
        parentLabel: page.t("设置")
        parentPage: "settings"
        pageTitle: page.t("关于")
    }

    SectionCard {
        Layout.fillWidth: true
        title: page.t("版本与兼容性")
        glyph: "monitor"

        ColumnLayout {
            id: compatTable
            objectName: "compatTable"
            Layout.fillWidth: true
            spacing: 0
            readonly property var columnSource: ["组件", "版本 / 标识", "适用范围"]
            readonly property var rows: [
                columnSource.map(label => page.t(label)),
                [page.t("桌面兼容基线"), "1.0.0-rc1", "Windows / Linux / macOS"],
                [page.t("项目模板"), page.service ? page.service.templateVersion : "", page.service ? page.service.templateId : ""],
                [page.t("管理 API"), "1.0", "kzb_f407"],
                [page.t("配置模型"), "p04.config.draft1", page.t("模拟模板")],
                [page.t("流程 IR"), "WF02-v1", page.t("本地仿真")]
            ]
            Repeater {
                model: compatTable.rows
                delegate: Rectangle {
                    id: tableRow
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    implicitHeight: cells.implicitHeight + 20
                    color: index === 0 ? WorkbenchStyle.surface : WorkbenchStyle.panel
                    RowLayout {
                        id: cells
                        x: 12
                        width: parent.width - 24
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 24
                        Repeater {
                            model: tableRow.modelData
                            delegate: Text {
                                required property string modelData
                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                text: modelData
                                color: tableRow.index === 0 ? WorkbenchStyle.muted : WorkbenchStyle.ink
                                font.pixelSize: tableRow.index === 0 ? 13 : 14
                                font.weight: tableRow.index === 0 ? Font.DemiBold : Font.Normal
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                    Rectangle {
                        visible: tableRow.index < compatTable.rows.length - 1
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: WorkbenchStyle.line
                    }
                }
            }
        }
    }

    SectionCard {
        id: trustCatalogCard
        objectName: "trustCatalogCard"
        Layout.fillWidth: true
        visible: page.service ? page.service.unlocked : false
        title: page.t("信任目录")
        glyph: "shield"

        KeyValueList {
            Layout.fillWidth: true
            items: [
                { label: page.t("目录域"), value: page.t("仅演示") },
                { label: page.t("模板发布"), value: page.t("演示模板评审") },
                { label: page.t("签发根"), value: page.t("原型样例根") + " · SIM-ROOT-01" },
                { label: page.t("部署许可"), value: page.t("未获生产签发") }
            ]
        }

        RowLayout {
            Layout.topMargin: 2
            spacing: 10

            WorkbenchButton {
                id: trustDetailBtn
                objectName: "trustDetailBtn"
                text: page.showTrustDetails ? page.t("收起详情") : page.t("查看信任记录")
                onClicked: page.showTrustDetails = !page.showTrustDetails
            }
        }

        NoticeBanner {
            id: trustDetailBanner
            objectName: "trustDetailBanner"
            Layout.fillWidth: true
            visible: page.showTrustDetails
            kind: "info"
            text: page.t("SIM-ROOT-01 只识别演示文件类型。正式发行由离线签发方提供凭据。")
        }
    }
}
