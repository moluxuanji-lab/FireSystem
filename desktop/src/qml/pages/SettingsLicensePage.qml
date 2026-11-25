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
    readonly property bool eligible: service ? (!service.licensed || service.canManage) : true
    readonly property bool isManagerRole: service ? (service.signedIn && (service.currentAccountRole === "admin" || service.currentAccountRole === "debugger")) : false
    readonly property bool hasReqToIssue: service && service.issuedLicenseRequest && service.issuedLicenseRequest.id !== undefined && service.issuedLicenseRequest.id !== ""

    property bool confirmClear: false
    property string issueMode: "years"
    property int issueYears: 1
    property date issueDate: new Date()

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    function formattedIssueDate() {
        return Qt.formatDate(issueDate, "yyyy-MM-dd")
    }

    function syncIssueModeCombo() {
        issueModeCombo.currentIndex = (page.issueMode === "date") ? 1 : 0
    }

    Component.onCompleted: {
        const d = new Date()
        d.setFullYear(d.getFullYear() + 1)
        issueDate = d
        syncIssueModeCombo()
    }
    onIssueModeChanged: syncIssueModeCombo()

    FileDialog {
        id: exportLicenseRequestDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.exportLicenseRequestToFile(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: importLicenseDialog
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.importLicenseFromFile(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: importRequestForIssueDialog
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.importLicenseRequestForIssue(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: exportIssuedLicenseDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.exportIssuedLicenseToFile(
                    selectedFile.toString(),
                    page.issueMode,
                    page.issueYears,
                    page.formattedIssueDate()
                )
            }
        }
    }

    PageHeader {
        service: page.service
        parentLabel: page.t("设置")
        parentPage: "settings"
        pageTitle: page.t("许可证")
    }

    GridLayout {
        Layout.fillWidth: true
        uniformCellWidths: true
        columns: (width > 720 && page.service && page.service.canIssue) ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        SectionCard {
            id: licenseStatusCard
            objectName: "licenseStatusCard"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            title: page.t("许可文件")
            glyph: "key"

            KeyValueList {
                Layout.fillWidth: true
                items: {
                    const list = [
                        { label: page.t("电脑名称"), value: page.service ? page.service.computerName : "" },
                        { label: page.t("当前状态"), value: (page.service && page.service.licensed) ? page.t("有效") : page.t("已失效") }
                    ]
                    if (page.service && page.service.licenseNotAfter !== "") {
                        list.push({ label: page.t("有效期"), value: page.service.licenseNotAfter })
                    }
                    if (page.service && page.service.licenseRequestId !== "") {
                        list.push({ label: page.t("当前请求"), value: page.service.licenseRequestId })
                    }
                    return list
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: (width <= 380) ? 1 : page.isManagerRole ? 3 : 2
                uniformCellWidths: true
                columnSpacing: 8
                rowSpacing: 8
                WorkbenchButton {
                    objectName: "exportLicenseReqBtn"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    primary: page.service && !page.service.licenseRequestId
                    text: page.t("导出请求文件")
                    disabled: !page.eligible
                    onClicked: {
                        if (page.service) {
                            exportLicenseRequestDialog.currentFile = page.service.suggestedExportFileUrl(
                                page.service.suggestedExportFileName("license-request")
                            )
                            exportLicenseRequestDialog.open()
                        }
                    }
                }
                WorkbenchButton {
                    objectName: "importLicenseBtn"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    primary: page.service && page.service.licenseRequestId !== ""
                    text: page.t("导入许可文件")
                    disabled: !page.eligible
                    onClicked: importLicenseDialog.open()
                }
                WorkbenchButton {
                    objectName: "clearLicenseBtn"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: page.isManagerRole
                    danger: true
                    text: page.t("清除许可")
                    disabled: !(page.service && page.service.licensed)
                    onClicked: page.confirmClear = true
                }
            }

            FluFrame {
                id: clearLicenseConfirmBox
                objectName: "clearLicenseConfirmBox"
                Layout.fillWidth: true
                visible: page.confirmClear && page.isManagerRole && (page.service ? page.service.licensed : false)
                padding: 16
                radius: 8
                border.color: WorkbenchStyle.line
                color: WorkbenchStyle.surface

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    FluText {
                        Layout.fillWidth: true
                        text: page.t("清除后，工作站将进入许可失效状态。重新启用需要申请新许可。")
                        font: FluTextStyle.Body
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        spacing: 10

                        WorkbenchButton {
                            id: confirmClearLicenseBtn
                            objectName: "confirmClearLicenseBtn"
                            text: page.t("确认清除")
                            danger: true
                            onClicked: {
                                if (page.service) {
                                    page.service.clearLicense()
                                    page.confirmClear = false
                                }
                            }
                        }

                        WorkbenchButton {
                            text: page.t("取消")
                            onClicked: page.confirmClear = false
                        }
                    }
                }
            }

            NoticeBanner {
                Layout.fillWidth: true
                visible: !page.eligible
                kind: "info"
                text: page.t("许可有效时，由管理员或调试员管理许可文件。")
            }
        }

        SectionCard {
            id: licenseIssuerCard
            objectName: "licenseIssuerCard"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            visible: page.service ? page.service.canIssue : false
            title: page.t("签发许可文件")
            glyph: "shield"

            FluText {
                Layout.fillWidth: true
                visible: !page.hasReqToIssue
                text: page.t("先导入目标电脑导出的请求文件。")
                font: FluTextStyle.Body
                textColor: WorkbenchStyle.muted
                wrapMode: Text.WordWrap
            }

            KeyValueList {
                Layout.fillWidth: true
                visible: page.hasReqToIssue
                items: page.hasReqToIssue ? [
                    {
                        label: page.t("目标电脑"),
                        value: (page.service.issuedLicenseRequest.computerName && page.service.issuedLicenseRequest.computerName !== "")
                            ? page.service.issuedLicenseRequest.computerName
                            : page.t("未获取电脑名称")
                    },
                    {
                        label: page.t("请求编号"),
                        value: page.service.issuedLicenseRequest.id
                    }
                ] : []
            }

            FluText {
                id: issueLicenseErrorText
                objectName: "issueLicenseErrorText"
                Layout.fillWidth: true
                visible: page.service ? page.service.issuedLicenseError !== "" : false
                text: page.service ? page.service.issuedLicenseError : ""
                font: FluTextStyle.Body
                textColor: FluTheme.dark ? FluColors.Red.lighter : FluColors.Red.dark
                wrapMode: Text.WordWrap
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: page.hasReqToIssue
                spacing: 10

                FluText {
                    text: page.t("有效期")
                    font: FluTextStyle.BodyStrong
                }

                WorkbenchComboBox {
                    id: issueModeCombo
                    objectName: "issueModeCombo"
                    Layout.fillWidth: true
                    Layout.maximumWidth: 260
                    readonly property var modes: [
                        { value: "years", label: page.t("按年数") },
                        { value: "date", label: page.t("按截止日期") }
                    ]
                    model: modes
                    textRole: "label"
                    valueRole: "value"
                    onModelChanged: page.syncIssueModeCombo()
                    onActivated: function(index) {
                        page.issueMode = (index === 1) ? "date" : "years"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: page.issueMode === "years"
                    spacing: 6

                    FluText {
                        text: page.t("有效年数")
                        font: FluTextStyle.BodyStrong
                    }

                    FluSpinBox {
                        id: issueYearsSpin
                        objectName: "issueYearsSpin"
                        from: 1
                        to: page.service ? page.service.maxUtcYearOffset() : 100
                        value: page.issueYears
                        editable: true
                        onValueChanged: page.issueYears = value
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: page.issueMode === "date"
                    spacing: 6

                    FluText {
                        text: page.t("截止日期（UTC）")
                        font: FluTextStyle.BodyStrong
                    }

                    FluCalendarPicker {
                        id: issueDatePicker
                        objectName: "issueDatePicker"
                        from: new Date()
                        to: new Date(9999, 11, 31)
                        current: page.issueDate
                        onAccepted: {
                            if (current) {
                                page.issueDate = current
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.topMargin: 4
                spacing: 10

                WorkbenchButton {
                    primary: !page.hasReqToIssue
                    id: importLicenseReqBtn
                    objectName: "importLicenseReqBtn"
                    text: page.t("导入请求文件")
                    onClicked: importRequestForIssueDialog.open()
                }

                WorkbenchButton {
                    primary: page.hasReqToIssue
                    id: exportIssuedLicenseBtn
                    objectName: "exportIssuedLicenseBtn"
                    text: page.t("导出许可文件")
                    disabled: !page.hasReqToIssue
                    onClicked: {
                        if (page.service) {
                            exportIssuedLicenseDialog.currentFile = page.service.suggestedExportFileUrl(
                                page.service.suggestedExportFileName("license")
                            )
                            exportIssuedLicenseDialog.open()
                        }
                    }
                }
            }
        }
    }
}
