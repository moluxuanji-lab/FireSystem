pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import FluentUI
import "../components"

FluScrollablePage {
    id: page

    property var service: null
    property var hostWindow: null

    readonly property var i18n: service ? service.i18n : null
    property bool showTrustDetails: false

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    spacing: 16
    leftPadding: 28
    rightPadding: 28
    topPadding: 22
    bottomPadding: 28

    PageHeader {
        service: page.service
        parentLabel: page.t("设置")
        parentPage: "settings"
        pageTitle: page.t("关于")
    }

    SectionCard {
        Layout.fillWidth: true
        Layout.maximumWidth: 920
        title: page.t("版本与兼容性")
        iconSource: FluentIcons.Info

        FluTableView {
            id: compatTable
            objectName: "compatTable"
            Layout.fillWidth: true
            Layout.preferredHeight: 38 + 5 * 42 + 2
            verticalHeaderVisible: false

            Component {
                id: headerComponentCol
                FluText {
                    text: page.t("组件")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Component {
                id: headerVersionCol
                FluText {
                    text: page.t("版本 / 标识")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Component {
                id: headerScopeCol
                FluText {
                    text: page.t("适用范围")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            columnSource: [
                {
                    title: compatTable.customItem(headerComponentCol),
                    dataIndex: "component",
                    width: 240,
                    readOnly: true
                },
                {
                    title: compatTable.customItem(headerVersionCol),
                    dataIndex: "version",
                    width: 240,
                    readOnly: true
                },
                {
                    title: compatTable.customItem(headerScopeCol),
                    dataIndex: "scope",
                    width: 340,
                    readOnly: true
                }
            ]
            dataSource: [
                { component: page.t("桌面兼容基线"), version: "1.0.0-rc1", scope: "Windows / Linux / macOS" },
                { component: page.t("项目模板"), version: page.service ? page.service.templateVersion : "1.2.1", scope: page.service ? page.service.templateId : "sim.baseline_ring" },
                { component: page.t("管理 API"), version: "1.0", scope: "kzb_f407" },
                { component: page.t("配置模型"), version: "p04.config.draft1", scope: page.t("模拟模板") },
                { component: page.t("流程 IR"), version: "WF02-v1", scope: page.t("本地仿真") }
            ]
        }
    }

    SectionCard {
        id: trustCatalogCard
        objectName: "trustCatalogCard"
        Layout.fillWidth: true
        Layout.maximumWidth: 920
        visible: page.service ? page.service.unlocked : false
        title: page.t("信任目录")
        iconSource: FluentIcons.Shield

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

            FluButton {
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

        NoticeBanner {
            Layout.fillWidth: true
            visible: page.showTrustDetails
            kind: "warning"
            text: page.t("真实硬件绑定、板卡管理接口与生产签发根当前属于无法确认项，本模块仅运行本地工作站仿真（SIM-ROOT-01）。")
        }
    }
}
