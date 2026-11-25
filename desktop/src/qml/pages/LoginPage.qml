pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import FluentUI
import "../components"

WorkbenchPage {
    id: page

    property var service: null

    readonly property var i18n: service ? service.i18n : null
    readonly property bool switching: service ? service.signedIn : false
    property string errorMessage: ""

    function t(key) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision)
    }

    function submitLogin() {
        if (!page.service) return
        page.errorMessage = page.service.signIn(accountInput.text, passwordInput.text)
        if (page.errorMessage === "") {
            passwordInput.text = ""
        }
    }

    PageHeader {
        service: page.service
        pageTitle: page.switching ? page.t("切换账号") : page.t("登录")
    }

    SectionCard {
        Layout.fillWidth: true
        Layout.maximumWidth: 440

        FluText {
            text: page.t("账号")
            font: FluTextStyle.BodyStrong
        }

        FluTextBox {
            id: accountInput
            objectName: "loginAccountInput"
            Layout.fillWidth: true
            onTextChanged: page.errorMessage = ""
            onCommit: page.submitLogin()
        }

        FluText {
            text: page.t("密码")
            font: FluTextStyle.BodyStrong
        }

        FluPasswordBox {
            id: passwordInput
            objectName: "loginPasswordInput"
            Layout.fillWidth: true
            onTextChanged: page.errorMessage = ""
            onCommit: page.submitLogin()
        }

        FluText {
            Layout.fillWidth: true
            text: page.t("首次使用 admin 登录时，密码留空。")
            font: FluTextStyle.Caption
            textColor: WorkbenchStyle.muted
            wrapMode: Text.WordWrap
        }

        FluText {
            id: loginErrorText
            objectName: "loginErrorText"
            Layout.fillWidth: true
            visible: page.errorMessage !== ""
            text: page.errorMessage
            font: FluTextStyle.Body
            textColor: FluTheme.dark ? FluColors.Red.lighter : FluColors.Red.dark
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            WorkbenchButton {
                primary: true
                id: loginSubmitButton
                objectName: "loginSubmitButton"
                text: page.switching ? page.t("切换账号") : page.t("登录")
                onClicked: page.submitLogin()
            }

            WorkbenchButton {
                objectName: "loginCancelButton"
                visible: page.switching
                text: page.t("取消")
                onClicked: {
                    if (page.service) {
                        page.service.navigateTo("settings-accounts")
                    }
                }
            }
        }
    }
}
