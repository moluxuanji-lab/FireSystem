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

    spacing: 16
    leftPadding: 28
    rightPadding: 28
    topPadding: 22
    bottomPadding: 28

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
        Layout.maximumWidth: 1120
        visible: page.service ? page.service.canManage : false
        title: page.t("本机账号")
        iconSource: FluentIcons.People
        headerAction: Component {
            FluFilledButton {
                objectName: "addAccountBtn"
                text: page.t("新增账号")
                onClicked: page.startCreate()
            }
        }

        FluTextBox {
            id: accountSearchBox
            objectName: "accountSearchBox"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            placeholderText: page.t("搜索账号或角色")
            iconSource: FluentIcons.Search
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
            border.color: FluTheme.dividerColor
            color: FluTheme.frameColor

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Header row
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: FluTheme.dark ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.03)
                    radius: 6

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12

                        FluText { Layout.preferredWidth: 180; Layout.fillWidth: true; text: page.t("账号"); font: FluTextStyle.BodyStrong }
                        FluText { Layout.preferredWidth: 140; text: page.t("角色"); font: FluTextStyle.BodyStrong }
                        FluText { Layout.preferredWidth: 140; text: page.t("状态"); font: FluTextStyle.BodyStrong }
                        FluText { Layout.preferredWidth: 150; text: page.t("操作"); font: FluTextStyle.BodyStrong }
                    }
                }

                Repeater {
                    model: page.service ? page.service.accountsModel : []

                    delegate: Rectangle {
                        id: accountRow
                        required property var modelData
                        required property int index

                        Layout.fillWidth: true
                        Layout.preferredHeight: 46
                        color: (accountRow.index % 2 === 1)
                            ? (FluTheme.dark ? Qt.rgba(1, 1, 1, 0.02) : Qt.rgba(0, 0, 0, 0.015))
                            : "transparent"

                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 1
                            color: FluTheme.dividerColor
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            FluText {
                                Layout.preferredWidth: 180
                                Layout.fillWidth: true
                                text: String(accountRow.modelData.id || "")
                                font: Qt.font({ family: "Consolas", pixelSize: 13 })
                                elide: Text.ElideRight
                            }

                            FluText {
                                Layout.preferredWidth: 140
                                text: page.roleLabel(accountRow.modelData.role)
                                font: FluTextStyle.Body
                            }

                            Item {
                                Layout.preferredWidth: 140
                                Layout.fillHeight: true
                                StatusChip {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: accountRow.modelData.pending ? page.t("待设置密码") : (accountRow.modelData.enabled ? page.t("启用") : page.t("停用"))
                                    tone: accountRow.modelData.pending ? "muted" : (accountRow.modelData.enabled ? "accent" : "muted")
                                }
                            }

                            RowLayout {
                                Layout.preferredWidth: 150
                                spacing: 8

                                FluText {
                                    visible: accountRow.modelData.id === "debug" && (!page.service || page.service.currentAccountId !== "debug")
                                    text: page.t("只读")
                                    font: FluTextStyle.Caption
                                    textColor: FluTheme.fontSecondaryColor
                                }

                                FluTextButton {
                                    visible: !(accountRow.modelData.id === "debug" && (!page.service || page.service.currentAccountId !== "debug"))
                                    text: page.t("编辑")
                                    padding: 4
                                    onClicked: page.startEdit(accountRow.modelData)
                                }

                                FluTextButton {
                                    visible: !(accountRow.modelData.id === "debug" && (!page.service || page.service.currentAccountId !== "debug"))
                                        && (!page.service || accountRow.modelData.id !== page.service.currentAccountId)
                                    text: page.t("删除")
                                    padding: 4
                                    textColor: FluTheme.dark ? FluColors.Red.lighter : FluColors.Red.dark
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
                    textColor: FluTheme.fontSecondaryColor
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
            border.color: FluTheme.dividerColor
            color: FluTheme.dark ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.025)

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
                        FluComboBox {
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

                    FluFilledButton {
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

                    FluButton {
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
            border.color: FluTheme.dividerColor
            color: FluTheme.dark ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.025)

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

                    FluFilledButton {
                        id: confirmDeleteAccountBtn
                        objectName: "confirmDeleteAccountBtn"
                        text: page.t("确认删除")
                        normalColor: FluColors.Red.dark
                        hoverColor: FluColors.Red.normal
                        pressedColor: FluColors.Red.darker
                        onClicked: {
                            if (page.service && page.service.deleteAccount(page.deletingId)) {
                                page.clearEditor()
                            }
                        }
                    }

                    FluButton {
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
        Layout.maximumWidth: 1120
        visible: page.service ? (page.service.signedIn && !page.service.canManage) : false
        title: page.t("当前账号")
        iconSource: FluentIcons.Contact

        KeyValueList {
            Layout.fillWidth: true
            items: [
                { label: page.t("当前账号"), value: page.service ? page.service.currentAccountId : "", monospace: true },
                { label: page.t("角色"), value: page.roleLabel(page.service ? page.service.currentAccountRole : "") }
            ]
        }

        FluButton {
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
        iconSource: FluentIcons.Lock

        KeyValueList {
            Layout.fillWidth: true
            items: [
                { label: page.t("账号"), value: page.service ? page.service.currentAccountId : "", monospace: true },
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
            FluFilledButton {
                objectName: "saveOwnPasswordBtn"
                text: page.t("保存新密码")
                onClicked: {
                    if (page.service && page.service.changeOwnPassword(page.ownOldPassword, page.ownNewPassword, page.ownConfirmPassword)) {
                        page.clearEditor()
                    }
                }
            }
            FluButton {
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
        Layout.maximumWidth: 1120
        visible: page.service ? !page.service.signedIn : true

        FluFilledButton {
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
        Layout.maximumWidth: 1120
        visible: page.service ? page.service.canRecover : false
        columns: (page.width >= 760 && page.service && page.service.canIssue) ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        SectionCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            title: page.t("管理员离线恢复")
            iconSource: FluentIcons.Permissions

            KeyValueList {
                Layout.fillWidth: true
                items: [
                    { label: page.t("电脑名称"), value: page.service ? page.service.computerName : "" },
                    { label: page.t("账号库代次"), value: page.service ? String(page.service.accountGeneration) : "0", monospace: true },
                    {
                        label: page.t("当前请求"),
                        value: (page.service && page.service.recoveryRequestId !== "")
                            ? page.service.recoveryRequestId
                            : page.t("尚未生成"),
                        monospace: page.service ? page.service.recoveryRequestId !== "" : false
                    }
                ]
            }

            RowLayout {
                Layout.topMargin: 4
                spacing: 10

                FluFilledButton {
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

                FluButton {
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

                FluFilledButton {
                    objectName: "importRecoveryPrimaryBtn"
                    visible: page.service ? page.service.recoveryRequestId !== "" : false
                    text: page.t("导入恢复文件")
                    onClicked: importRecoveryDialog.open()
                }

                FluButton {
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
            iconSource: FluentIcons.Shield

            FluText {
                Layout.fillWidth: true
                visible: !page.hasRecoveryReqToIssue
                text: page.t("先导入目标电脑导出的请求文件。")
                font: FluTextStyle.Body
                textColor: FluTheme.fontSecondaryColor
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
                        value: page.service.issuedRecoveryRequest.id,
                        monospace: true
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

                FluButton {
                    objectName: "importRecoveryReqBtn"
                    text: page.t("导入请求文件")
                    onClicked: importRecoveryReqForIssueDialog.open()
                }

                FluFilledButton {
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
