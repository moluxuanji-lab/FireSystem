pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import FluentUI
import "qml/components"

FluWindow {
    id: window

    property var service: null
    readonly property var i18n: service ? service.i18n : null
    property bool _syncingNav: false
    property string lastToastMessage: ""
    property string lastToastKind: ""

    function tr(key, arg0) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision, arg0 === undefined ? "" : String(arg0))
    }

    function roleLabel(role) {
        if (role === "admin") return tr("管理员")
        if (role === "debugger") return tr("调试员")
        if (role === "operator") return tr("操作员")
        return ""
    }

    function pageUrl(pageKey) {
        switch (pageKey) {
        case "login": return "qrc:/qt/qml/FireSystem/UI/qml/pages/LoginPage.qml"
        case "settings": return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsPage.qml"
        case "settings-accounts": return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsAccountsPage.qml"
        case "settings-license": return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsLicensePage.qml"
        case "settings-templates": return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsTemplatesPage.qml"
        case "settings-general": return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsGeneralPage.qml"
        case "settings-about": return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsAboutPage.qml"
        default: return "qrc:/qt/qml/FireSystem/UI/qml/pages/SettingsPage.qml"
        }
    }

    function syncThemeFromService() {
        if (!service) return
        if (service.theme === "light") {
            FluTheme.darkMode = FluThemeType.Light
        } else if (service.theme === "dark") {
            FluTheme.darkMode = FluThemeType.Dark
        } else {
            FluTheme.darkMode = FluThemeType.System
        }
    }

    function syncPageFromService() {
        if (!service) return
        const targetPage = service.currentPage
        const targetUrl = pageUrl(targetPage)
        _syncingNav = true
        const items = nav_view.getItems()
        for (let i = 0; i < items.length; ++i) {
            const item = items[i]
            if (item && item.extra === targetPage) {
                if (nav_view.getCurrentIndex() !== i) {
                    nav_view.startPageByItem(item)
                }
                break
            }
        }
        _syncingNav = false
        if (nav_view.getCurrentUrl() !== targetUrl) {
            nav_view.push(targetUrl, { service: window.service, hostWindow: window })
        }
    }

    title: "FireSystem · " + tr("设置")
    width: 1120
    height: 740
    minimumWidth: 720
    minimumHeight: 500
    visible: true

    onServiceChanged: {
        syncThemeFromService()
        syncPageFromService()
    }

    Component.onCompleted: {
        FluApp.init(window)
        FluTheme.accentColor = FluColors.createAccentColor(Qt.rgba(37 / 255, 99 / 255, 177 / 255, 1))
        FluTheme.animationEnabled = true
        syncThemeFromService()
        syncPageFromService()
    }

    Connections {
        target: window.service
        function onStateChanged() {
            window.syncThemeFromService()
            window.syncPageFromService()
        }
        function onToastRequested(message, kind) {
            window.lastToastMessage = message
            window.lastToastKind = kind
            if (kind === "error") {
                window.showError(message)
            } else if (kind === "warning") {
                window.showWarning(message)
            } else if (kind === "info") {
                window.showInfo(message)
            } else {
                window.showSuccess(message)
            }
        }
    }

    appBar: FluAppBar {
        title: window.title
        showDark: true
        darkClickListener: function() {
            if (!window.service) return
            window.service.setTheme(FluTheme.dark ? "light" : "dark")
        }
    }

    FluNavigationView {
        id: nav_view
        objectName: "mainNavView"
        anchors.fill: parent
        anchors.topMargin: window.appBar.height
        pageMode: FluNavigationViewType.NoStack
        cellWidth: 224
        navCompactWidth: 62
        title: window.tr("轨道消防上位机")

        items: FluObject {
            FluPaneItem {
                extra: "login"
                title: window.tr("登录")
                icon: FluentIcons.Contact
                url: window.pageUrl("login")
                visible: window.service ? !window.service.signedIn : true
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("login")
                    }
                }
            }

            FluPaneItemHeader {
                title: window.tr("设置")
            }

            FluPaneItem {
                extra: "settings"
                title: window.tr("设置")
                icon: FluentIcons.Settings
                url: window.pageUrl("settings")
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("settings")
                    }
                }
            }

            FluPaneItem {
                extra: "settings-accounts"
                title: window.tr("账号")
                icon: FluentIcons.People
                url: window.pageUrl("settings-accounts")
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("settings-accounts")
                    }
                }
            }

            FluPaneItem {
                extra: "settings-license"
                title: window.tr("许可证")
                icon: FluentIcons.Permissions
                url: window.pageUrl("settings-license")
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("settings-license")
                    }
                }
            }

            FluPaneItem {
                extra: "settings-templates"
                title: window.tr("项目模板")
                icon: FluentIcons.Folder
                url: window.pageUrl("settings-templates")
                visible: window.service ? window.service.unlocked : false
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("settings-templates")
                    }
                }
            }

            FluPaneItem {
                extra: "settings-general"
                title: window.tr("通用")
                icon: FluentIcons.Design
                url: window.pageUrl("settings-general")
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("settings-general")
                    }
                }
            }

            FluPaneItem {
                extra: "settings-about"
                title: window.tr("关于")
                icon: FluentIcons.Info
                url: window.pageUrl("settings-about")
                onTap: {
                    if (!window._syncingNav && window.service) {
                        window.service.navigateTo("settings-about")
                    }
                }
            }
        }

        footerItems: FluObject {
            FluPaneItem {
                title: window.tr("本地仿真") + " · SIMULATION"
                icon: FluentIcons.Shield
                disabled: true
                onTapListener: function() {}
            }
        }

        actionItem: Component {
            RowLayout {
                spacing: 10
                Layout.rightMargin: 8

                StatusChip {
                    text: (window.service && window.service.licensed)
                        ? window.tr("有效")
                        : window.tr("已失效")
                    tone: (window.service && window.service.licensed) ? "good" : "muted"
                }

                FluButton {
                    objectName: "headerLoginBtn"
                    visible: window.service ? !window.service.signedIn : true
                    text: window.tr("登录")
                    onClicked: {
                        if (window.service) {
                            window.service.navigateTo("login")
                        }
                    }
                }

                FluDropDownButton {
                    id: profileDropdown
                    objectName: "headerProfileDropdown"
                    visible: window.service ? window.service.signedIn : false
                    implicitWidth: Math.max(168, contentText.implicitWidth + 56)
                    text: ""

                    FluText {
                        id: contentText
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        text: window.service
                            ? (window.service.currentAccountId + " · " + window.roleLabel(window.service.currentAccountRole))
                            : ""
                        font: FluTextStyle.Caption
                        elide: Text.ElideRight
                        width: Math.max(80, parent.width - 42)
                    }

                    FluMenuItem {
                        text: window.tr("切换账号")
                        onClicked: {
                            if (window.service) {
                                window.service.navigateTo("login")
                            }
                        }
                    }
                    FluMenuItem {
                        text: window.tr("账号设置")
                        onClicked: {
                            if (window.service) {
                                window.service.navigateTo("settings-accounts")
                            }
                        }
                    }
                    FluMenuItem {
                        text: window.tr("退出登录")
                        onClicked: {
                            if (window.service) {
                                window.service.signOut()
                            }
                        }
                    }
                }
            }
        }
    }
}
