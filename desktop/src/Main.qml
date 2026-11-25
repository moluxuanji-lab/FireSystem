pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import FluentUI
import "qml/components"
import "qml/pages"

ApplicationWindow {
    id: window

    property var service: null
    readonly property var i18n: service ? service.i18n : null
    readonly property string currentPage: service ? service.currentPage : "settings"
    readonly property bool narrow: width <= 760
    property bool compact: false
    property bool drawerOpen: false
    property string lastToastMessage: ""
    property string lastToastKind: ""

    function tr(key) {
        return i18n ? i18n.t(key, i18n.revision) : key
    }

    function navigate(key) {
        drawerOpen = false
        if (service) service.navigateTo(key)
    }

    function syncTheme() {
        if (!service) return
        FluTheme.darkMode = service.theme === "dark" ? FluThemeType.Dark
            : service.theme === "light" ? FluThemeType.Light : FluThemeType.System
    }

    title: "FireSystem · " + tr("设置")
    width: 1120
    height: 740
    minimumWidth: 720
    minimumHeight: 500
    visible: true
    color: WorkbenchStyle.surface
    onServiceChanged: syncTheme()
    onCurrentPageChanged: drawerOpen = false
    onNarrowChanged: drawerOpen = false
    Component.onCompleted: {
        FluApp.init(window)
        FluTheme.accentColor = FluColors.createAccentColor("#2563b1")
        syncTheme()
    }

    Connections {
        target: window.service
        function onStateChanged() { window.syncTheme() }
        function onToastRequested(message, kind) {
            window.lastToastMessage = message
            window.lastToastKind = kind
            toastTimeout.restart()
        }
    }

    WorkbenchHeader {
        id: header
        width: parent.width
        height: implicitHeight
        service: window.service
        narrow: window.narrow
        onNavigate: key => window.navigate(key)
        onToggleNavigation: {
            if (window.narrow) window.drawerOpen = !window.drawerOpen
            else window.compact = !window.compact
        }
    }

    Item {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Loader {
            id: pageLoader
            objectName: "pageLoader"
            anchors.fill: parent
            anchors.leftMargin: window.narrow ? 0 : sidebar.width
            sourceComponent: {
                switch (window.currentPage) {
                case "login": return login
                case "settings": return settings
                case "settings-accounts": return accounts
                case "settings-license": return license
                case "settings-templates": return templates
                case "settings-general": return general
                case "settings-about": return about
                default: return placeholder
                }
            }
        }
        Rectangle {
            anchors.fill: parent
            visible: window.narrow && window.drawerOpen
            color: "#40142336"
            MouseArea { anchors.fill: parent; onClicked: window.drawerOpen = false }
        }
        WorkbenchSidebar {
            id: sidebar
            objectName: "mainNavView"
            service: window.service
            height: parent.height
            visible: !window.narrow || window.drawerOpen
            compact: !window.narrow && window.compact
            width: window.narrow ? 210 : compact ? 62 : window.width <= 1000 ? 160
                : window.width <= 1250 ? 185 : window.width < 1550 ? 212 : 233
            onNavigate: key => window.navigate(key)
        }
    }

    Timer { id: toastTimeout; interval: 3200 }
    Rectangle {
        visible: toastTimeout.running
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 22
        width: Math.min(window.width - 32, toastText.implicitWidth + 34)
        height: toastText.implicitHeight + 20
        radius: 6
        color: WorkbenchStyle.ink
        Text {
            id: toastText
            width: parent.width - 34
            anchors.centerIn: parent
            text: window.lastToastMessage
            font.pixelSize: 14
            color: WorkbenchStyle.panel
            wrapMode: Text.Wrap
        }
    }

    Component { id: login; LoginPage { service: window.service } }
    Component { id: settings; SettingsPage { service: window.service } }
    Component { id: accounts; SettingsAccountsPage { service: window.service } }
    Component { id: license; SettingsLicensePage { service: window.service } }
    Component { id: templates; SettingsTemplatesPage { service: window.service } }
    Component { id: general; SettingsGeneralPage { service: window.service } }
    Component { id: about; SettingsAboutPage { service: window.service } }
    Component {
        id: placeholder
        WorkbenchPage {
            id: placeholderPage
            objectName: "placeholderPage"
            readonly property var pages: window.service ? window.service.navigationPages : []
            readonly property var route: pages.find(p => p.id === window.currentPage)
            readonly property var section: route ? pages.find(p => p.id === route.section) : null
            PageHeader {
                service: window.service
                pageTitle: window.tr(placeholderPage.route ? placeholderPage.route.label : "")
                parentLabel: window.tr(placeholderPage.section ? placeholderPage.section.label : "")
                parentPage: placeholderPage.section ? placeholderPage.section.id : ""
            }
        }
    }
}
