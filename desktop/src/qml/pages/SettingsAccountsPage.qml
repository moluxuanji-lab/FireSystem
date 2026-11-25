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
    readonly property bool hasRecoveryReqToIssue: service && service.issuedRecoveryRequest && service.issuedRecoveryRequest.id !== undefined && service.issuedRecoveryRequest.id !== ""

    // Editor state: "" | "create" | "edit" | "delete" | "own-password"
    property string editorMode: ""
    property string editingOriginalId: ""
    property bool editingIsPending: false
    property string formAccountId: ""
    property string formRole: "operator"
    property bool formEnabled: true
    property bool formResetPassword: false
    property string formPassword: ""
    property string deletingId: ""

    property string ownOldPassword: ""
    property string ownNewPassword: ""
    property string ownConfirmPassword: ""

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    function roleLabel(role) {
        if (role === "admin") return t("管理员")
        if (role === "debugger") return t("调试员")
        return t("操作员")
    }

    function clearEditor() {
        editorMode = ""
        editingOriginalId = ""
        editingIsPending = false
        formAccountId = ""
        formRole = "operator"
        formEnabled = true
        formResetPassword = false
        formPassword = ""
        deletingId = ""
        ownOldPassword = ""
        ownNewPassword = ""
        ownConfirmPassword = ""
    }

    function startCreate() {
        clearEditor()
        editorMode = "create"
        formRole = "operator"
        formEnabled = true
    }

    function startEdit(row) {
        clearEditor()
        if (page.service && row.id === page.service.currentAccountId) {
            editorMode = "own-password"
            return
        }
        editorMode = "edit"
        editingOriginalId = row.id
        editingIsPending = !!row.pending
        formAccountId = row.id
        formRole = row.role
        formEnabled = !!row.enabled
    }

    function startDelete(row) {
        clearEditor()
        editorMode = "delete"
        deletingId = row.id
    }

    function syncRoleCombo() {
        editAccountRoleCombo.currentIndex = (page.formRole === "admin") ? 1 : 0
    }

    Component.onCompleted: syncRoleCombo()
    onFormRoleChanged: syncRoleCombo()

    FileDialog {
        id: exportRecoveryRequestDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.exportRecoveryRequestToFile(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: importRecoveryDialog
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.importRecoveryFromFile(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: importRecoveryReqForIssueDialog
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.importRecoveryRequestForIssue(selectedFile.toString())
            }
        }
    }

    FileDialog {
        id: exportIssuedRecoveryDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: {
            if (page.service) {
                page.service.exportIssuedRecoveryToFile(selectedFile.toString())
            }
        }
    }

    PageHeader {
        service: page.service
        parentLabel: page.t("设置")
        parentPage: "settings"
        pageTitle: page.t("账号")
    }

    // Case 1: Signed in as manager (admin or debugger) & licensed
    SectionCard {
        id: managedAccountsCard
        objectName: "managedAccountsCard"
        Layout.fillWidth: true
        visible: page.service ? page.service.canManage : false
        title: page.t("本机账号")
        glyph: "users"
        headerAction: Component {
            WorkbenchButton {
                primary: true
                objectName: "addAccountBtn"
                text: page.t("新增账号")
                onClicked: page.startCreate()
            }
        }

        FluTextBox {
            id: accountSearchBox
            objectName: "accountSearchBox"
            Layout.fillWidth: true
            placeholderText: page.t("搜索账号或角色")
            font.pixelSize: 14
            normalColor: WorkbenchStyle.ink
            placeholderNormalColor: WorkbenchStyle.muted
            background: Rectangle {
                radius: 5
                color: WorkbenchStyle.panel
                border.color: accountSearchBox.activeFocus ? WorkbenchStyle.accent : WorkbenchStyle.line
            }
            text: page.service ? page.service.accountSearchQuery : ""
            onTextChanged: {
                if (page.service && page.service.accountSearchQuery !== text) {
                    page.service.accountSearchQuery = text
                }
            }
        }

        FluFrame {
            Layout.fillWidth: true
            radius: 6
            border.color: WorkbenchStyle.line
            color: WorkbenchStyle.panel
            border.width: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Header row
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: WorkbenchStyle.surface

                    GridLayout {
                        columns: 4
                        uniformCellWidths: true
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        columnSpacing: 12

                        Repeater {
                            model: ["账号", "角色", "状态", "操作"]
                            delegate: Text {
                                required property string modelData
                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                text: page.t(modelData)
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                                color: WorkbenchStyle.muted
                            }
                        }
                    }
                }

                Repeater {
                    model: page.service ? page.service.accountsModel : []

                    delegate: Rectangle {
                        id: accountRow
                        required property var modelData

                        Layout.fillWidth: true
                        Layout.preferredHeight: 43
                        color: WorkbenchStyle.panel

                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 1
                            color: WorkbenchStyle.line
                        }

                        GridLayout {
                            columns: 4
                            uniformCellWidths: true
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            columnSpacing: 12

                            FluText {
                                Layout.preferredWidth: 1
                                Layout.fillWidth: true
                                text: String(accountRow.modelData.id || "")
                                font.pixelSize: 14
                                textColor: WorkbenchStyle.ink
                                elide: Text.ElideRight
                            }

                            FluText {
                                Layout.preferredWidth: 1
                                Layout.fillWidth: true
                                text: page.roleLabel(accountRow.modelData.role)
                                font: FluTextStyle.Body
                            }

                            Item {
                                Layout.preferredWidth: 1
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                StatusChip {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: accountRow.modelData.pending ? page.t("待设置密码") : (accountRow.modelData.enabled ? page.t("启用") : page.t("停用"))
                                    tone: accountRow.modelData.enabled ? "accent" : "muted"
                                }
                            }

                            Row {
                                Layout.preferredWidth: 1
                                Layout.fillWidth: true
                                spacing: 8

                                FluText {
                                    visible: accountRow.modelData.id === "debug" && (!page.service || page.service.currentAccountId !== "debug")
                                    text: page.t("只读")
                                    font: FluTextStyle.Caption
                                    textColor: WorkbenchStyle.muted
                                }

                                FluTextButton {
                                    visible: !(accountRow.modelData.id === "debug" && (!page.service || page.service.currentAccountId !== "debug"))
                                    text: page.t("编辑")
                                    textColor: WorkbenchStyle.accent
                                    padding: 0
                                    onClicked: page.startEdit(accountRow.modelData)
                                }

                                FluTextButton {
                                    visible: !(accountRow.modelData.id === "debug" && (!page.service || page.service.currentAccountId !== "debug"))
                                        && (!page.service || accountRow.modelData.id !== page.service.currentAccountId)
                                    text: page.t("删除")
                                    textColor: WorkbenchStyle.accent
                                    padding: 0
                                    onClicked: page.startDelete(accountRow.modelData)
                                }
                            }
                        }
                    }
                }

                FluText {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    visible: page.service ? page.service.accountsModel.length === 0 : true
                    text: page.t("没有匹配的账号。")
                    font: FluTextStyle.Body
                    textColor: WorkbenchStyle.muted
                }
            }
        }

        // Inline Create / Edit account form
        FluFrame {
            id: accountEditorBox
            objectName: "accountEditorBox"
            Layout.fillWidth: true
            visible: page.editorMode === "create" || page.editorMode === "edit"
            padding: 16
            radius: 8
            border.color: WorkbenchStyle.line
            color: WorkbenchStyle.surface

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                FluText {
                    text: page.editorMode === "create" ? page.t("新增本机账号") : page.t("编辑本机账号")
                    font: FluTextStyle.Subtitle
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: page.width >= 680 ? 2 : 1
                    columnSpacing: 14
                    rowSpacing: 10

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        FluText { text: page.t("账号"); font: FluTextStyle.BodyStrong }
                        FluTextBox {
                            id: editAccountIdInput
                            objectName: "editAccountIdInput"
                            Layout.fillWidth: true
                            readOnly: page.editorMode === "edit"
                            text: page.formAccountId
                            onTextChanged: page.formAccountId = text
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        FluText { text: page.t("角色"); font: FluTextStyle.BodyStrong }
                        WorkbenchComboBox {
                            id: editAccountRoleCombo
                            objectName: "editAccountRoleCombo"
                            Layout.fillWidth: true
                            readonly property var roleOpts: [
                                { value: "operator", label: page.t("操作员") },
                                { value: "admin", label: page.t("管理员") }
                            ]
                            model: roleOpts
                            textRole: "label"
                            valueRole: "value"
                            onModelChanged: page.syncRoleCombo()
                            onActivated: function(index) {
                                page.formRole = (index === 1) ? "admin" : "operator"
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        FluCheckBox {
                            id: resetPasswordCheck
                            objectName: "resetPasswordCheck"
                            visible: page.editorMode === "edit" && !page.editingIsPending
                            text: page.t("重设密码")
                            checked: page.formResetPassword
                            onClicked: page.formResetPassword = checked
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: page.editorMode === "create" || page.editingIsPending || page.formResetPassword
                            spacing: 4

                            FluText {
                                text: page.editingIsPending ? page.t("设置密码") : page.t("密码（可留空）")
                                font: FluTextStyle.BodyStrong
                            }

                            FluPasswordBox {
                                id: editAccountPasswordInput
                                objectName: "editAccountPasswordInput"
                                Layout.fillWidth: true
                                text: page.formPassword
                                onTextChanged: page.formPassword = text
                            }
                        }
                    }

                    FluCheckBox {
                        id: accountEnabledCheck
                        objectName: "accountEnabledCheck"
                        Layout.alignment: Qt.AlignBottom
                        text: page.t("启用此账号")
                        checked: page.formEnabled
                        onClicked: page.formEnabled = checked
                    }
                }

                RowLayout {
                    spacing: 10

                    WorkbenchButton {
                        primary: true
                        id: saveAccountBtn
                        objectName: "saveAccountBtn"
                        text: page.t("保存账号")
                        onClicked: {
                            if (!page.service) return
                            const hasPassword = (page.editorMode === "create")
                                || page.editingIsPending
                                || page.formResetPassword
                            const ok = page.service.saveAccount(
                                page.editorMode === "create" ? "" : page.editingOriginalId,
                                page.formAccountId,
                                page.formRole,
                                page.formEnabled,
                                hasPassword,
                                page.formPassword
                            )
                            if (ok) {
                                page.clearEditor()
                            }
                        }
                    }

                    WorkbenchButton {
                        text: page.t("取消")
                        onClicked: page.clearEditor()
                    }
                }
            }
        }

        // Inline Delete confirmation
        FluFrame {
            id: deleteAccountBox
            objectName: "deleteAccountBox"
            Layout.fillWidth: true
            visible: page.editorMode === "delete" && page.deletingId !== ""
            padding: 16
            radius: 8
            border.color: WorkbenchStyle.line
            color: WorkbenchStyle.surface

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                FluText { text: page.t("删除账号"); font: FluTextStyle.Subtitle }
                FluText {
                    text: page.deletingId
                    font: Qt.font({ family: "Consolas", pixelSize: 13 })
                }

                RowLayout {
                    spacing: 10

                    WorkbenchButton {
                        id: confirmDeleteAccountBtn
                        objectName: "confirmDeleteAccountBtn"
                        text: page.t("确认删除")
                        danger: true
                        onClicked: {
                            if (page.service && page.service.deleteAccount(page.deletingId)) {
                                page.clearEditor()
                            }
                        }
                    }

                    WorkbenchButton {
                        text: page.t("取消")
                        onClicked: page.clearEditor()
                    }
                }
            }
        }
    }

    // Case 2: Signed in non-manager (operator or unlicensed manager)
    SectionCard {
        id: operatorAccountCard
        objectName: "operatorAccountCard"
        Layout.fillWidth: true
        visible: page.service ? (page.service.signedIn && !page.service.canManage && page.editorMode !== "own-password") : false

        KeyValueList {
            Layout.fillWidth: true
            items: [
                { label: page.t("当前账号"), value: page.service ? page.service.currentAccountId : "" },
                { label: page.t("角色"), value: page.roleLabel(page.service ? page.service.currentAccountRole : "") }
            ]
        }

        WorkbenchButton {
            objectName: "openOwnPasswordBtn"
            visible: page.editorMode !== "own-password"
            text: page.t("修改自己的密码")
            onClicked: {
                page.clearEditor()
                page.editorMode = "own-password"
            }
        }
    }

    // Shared Own-Password Change Form
    SectionCard {
        id: ownPasswordCard
        objectName: "ownPasswordCard"
        Layout.fillWidth: true
        Layout.maximumWidth: 560
        visible: page.editorMode === "own-password" && (page.service ? page.service.signedIn : false)
        title: page.t("修改自己的密码")
        glyph: "key"

        KeyValueList {
            Layout.fillWidth: true
            items: [
                { label: page.t("账号"), value: page.service ? page.service.currentAccountId : "" },
                { label: page.t("角色"), value: page.roleLabel(page.service ? page.service.currentAccountRole : "") }
            ]
        }

        FluText { text: page.t("当前密码"); font: FluTextStyle.BodyStrong }
        FluPasswordBox {
            objectName: "ownOldPasswordInput"
            Layout.fillWidth: true
            text: page.ownOldPassword
            onTextChanged: page.ownOldPassword = text
        }

        FluText { text: page.t("新密码"); font: FluTextStyle.BodyStrong }
        FluPasswordBox {
            objectName: "ownNewPasswordInput"
            Layout.fillWidth: true
            text: page.ownNewPassword
            onTextChanged: page.ownNewPassword = text
        }

        FluText { text: page.t("确认新密码"); font: FluTextStyle.BodyStrong }
        FluPasswordBox {
            objectName: "ownConfirmPasswordInput"
            Layout.fillWidth: true
            text: page.ownConfirmPassword
            onTextChanged: page.ownConfirmPassword = text
        }

        RowLayout {
            spacing: 10
            WorkbenchButton {
                primary: true
                objectName: "saveOwnPasswordBtn"
                text: page.t("保存新密码")
                onClicked: {
                    if (page.service && page.service.changeOwnPassword(page.ownOldPassword, page.ownNewPassword, page.ownConfirmPassword)) {
                        page.clearEditor()
                    }
                }
            }
            WorkbenchButton {
                text: page.t("取消")
                onClicked: page.clearEditor()
            }
        }
    }

    // Case 3: Signed out
    SectionCard {
        id: signedOutAccountCard
        objectName: "signedOutAccountCard"
        Layout.fillWidth: true
        visible: page.service ? !page.service.signedIn : true

        WorkbenchButton {
            primary: true
            objectName: "goLoginFromAccountsBtn"
            text: page.t("登录")
            onClicked: {
                if (page.service) {
                    page.service.navigateTo("login")
                }
            }
        }
    }

    // Conditional Recovery Grid (when canRecover)
    GridLayout {
        id: recoveryGrid
        objectName: "recoveryGrid"
        Layout.fillWidth: true
        visible: page.service ? page.service.canRecover : false
        uniformCellWidths: true
        columns: (width > 720 && page.service && page.service.canIssue) ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        SectionCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            title: page.t("管理员离线恢复")
            glyph: "key"

            KeyValueList {
                Layout.fillWidth: true
                items: [
                    { label: page.t("电脑名称"), value: page.service ? page.service.computerName : "" },
                    { label: page.t("账号库代次"), value: page.service ? String(page.service.accountGeneration) : "0" },
                    {
                        label: page.t("当前请求"),
                        value: (page.service && page.service.recoveryRequestId !== "")
                            ? page.service.recoveryRequestId
                            : page.t("尚未生成")
                    }
                ]
            }

            RowLayout {
                Layout.topMargin: 4
                spacing: 10

                WorkbenchButton {
                    primary: true
                    objectName: "exportRecoveryReqPrimaryBtn"
                    visible: !(page.service && page.service.recoveryRequestId !== "")
                    text: page.t("导出请求文件")
                    onClicked: {
                        if (page.service) {
                            exportRecoveryRequestDialog.currentFile = page.service.suggestedExportFileUrl(
                                page.service.suggestedExportFileName("recovery-request")
                            )
                            exportRecoveryRequestDialog.open()
                        }
                    }
                }

                WorkbenchButton {
                    objectName: "exportRecoveryReqSecondaryBtn"
                    visible: page.service ? page.service.recoveryRequestId !== "" : false
                    text: page.t("导出请求文件")
                    onClicked: {
                        if (page.service) {
                            exportRecoveryRequestDialog.currentFile = page.service.suggestedExportFileUrl(
                                page.service.suggestedExportFileName("recovery-request")
                            )
                            exportRecoveryRequestDialog.open()
                        }
                    }
                }

                WorkbenchButton {
                    primary: true
                    objectName: "importRecoveryPrimaryBtn"
                    visible: page.service ? page.service.recoveryRequestId !== "" : false
                    text: page.t("导入恢复文件")
                    onClicked: importRecoveryDialog.open()
                }

                WorkbenchButton {
                    objectName: "importRecoveryBtn"
                    visible: !(page.service && page.service.recoveryRequestId !== "")
                    text: page.t("导入恢复文件")
                    disabled: true
                }
            }
        }

        SectionCard {
            id: recoveryIssuerCard
            objectName: "recoveryIssuerCard"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            visible: page.service ? page.service.canIssue : false
            title: page.t("签发恢复文件")
            glyph: "shield"

            FluText {
                Layout.fillWidth: true
                visible: !page.hasRecoveryReqToIssue
                text: page.t("先导入目标电脑导出的请求文件。")
                font: FluTextStyle.Body
                textColor: WorkbenchStyle.muted
                wrapMode: Text.WordWrap
            }

            KeyValueList {
                Layout.fillWidth: true
                visible: page.hasRecoveryReqToIssue
                items: page.hasRecoveryReqToIssue ? [
                    {
                        label: page.t("目标电脑"),
                        value: (page.service.issuedRecoveryRequest.computerName && page.service.issuedRecoveryRequest.computerName !== "")
                            ? page.service.issuedRecoveryRequest.computerName
                            : page.t("未获取电脑名称")
                    },
                    {
                        label: page.t("请求编号"),
                        value: page.service.issuedRecoveryRequest.id
                    }
                ] : []
            }

            FluText {
                Layout.fillWidth: true
                visible: page.service ? page.service.issuedRecoveryError !== "" : false
                text: page.service ? page.service.issuedRecoveryError : ""
                font: FluTextStyle.Body
                textColor: FluTheme.dark ? FluColors.Red.lighter : FluColors.Red.dark
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.topMargin: 4
                spacing: 10

                WorkbenchButton {
                    primary: !page.hasRecoveryReqToIssue
                    objectName: "importRecoveryReqBtn"
                    text: page.t("导入请求文件")
                    onClicked: importRecoveryReqForIssueDialog.open()
                }

                WorkbenchButton {
                    primary: page.hasRecoveryReqToIssue
                    objectName: "exportIssuedRecoveryBtn"
                    text: page.t("导出恢复文件")
                    disabled: !page.hasRecoveryReqToIssue
                    onClicked: {
                        if (page.service) {
                            exportIssuedRecoveryDialog.currentFile = page.service.suggestedExportFileUrl(
                                page.service.suggestedExportFileName("recovery")
                            )
                            exportIssuedRecoveryDialog.open()
                        }
                    }
                }
            }
        }
    }
}
