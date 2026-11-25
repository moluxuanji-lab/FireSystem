pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
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

    function syncLanguageCombo() {
        if (!page.service || !page.i18n) {
            languageCombo.currentIndex = 0
            return
        }
        const opts = page.i18n.languageOptions
        for (let i = 0; i < opts.length; ++i) {
            if (opts[i].value === page.service.lang) {
                languageCombo.currentIndex = i
                return
            }
        }
        languageCombo.currentIndex = 0
    }

    function syncThemeCombo() {
        if (!page.service) {
            themeCombo.currentIndex = 0
            return
        }
        const opts = themeCombo.themeModel
        for (let i = 0; i < opts.length; ++i) {
            if (opts[i].value === page.service.theme) {
                themeCombo.currentIndex = i
                return
            }
        }
        themeCombo.currentIndex = 0
    }

    Component.onCompleted: {
        syncLanguageCombo()
        syncThemeCombo()
    }
    onServiceChanged: {
        syncLanguageCombo()
        syncThemeCombo()
    }

    Connections {
        target: page.service
        function onStateChanged() {
            page.syncLanguageCombo()
            page.syncThemeCombo()
        }
    }

    PageHeader {
        service: page.service
        parentLabel: page.t("设置")
        parentPage: "settings"
        pageTitle: page.t("通用")
    }

    SectionCard {
        Layout.fillWidth: true

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            FluText {
                Layout.fillWidth: true
                text: page.t("语言")
                font.pixelSize: 15
                textColor: WorkbenchStyle.ink
            }

            WorkbenchComboBox {
                id: languageCombo
                objectName: "languageCombo"
                Layout.preferredWidth: 240
                model: page.i18n ? page.i18n.languageOptions : []
                textRole: "label"
                valueRole: "value"
                onModelChanged: page.syncLanguageCombo()
                onActivated: function(index) {
                    if (page.service && page.i18n) {
                        const opts = page.i18n.languageOptions
                        if (index >= 0 && index < opts.length) {
                            page.service.setLanguage(opts[index].value)
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            FluText {
                Layout.fillWidth: true
                text: page.t("界面主题")
                font.pixelSize: 15
                textColor: WorkbenchStyle.ink
            }

            WorkbenchComboBox {
                id: themeCombo
                objectName: "themeCombo"
                Layout.preferredWidth: 240
                readonly property var themeModel: [
                    { value: "system", label: page.t("跟随系统") },
                    { value: "light", label: page.t("浅色") },
                    { value: "dark", label: page.t("深色") }
                ]
                model: themeModel
                textRole: "label"
                valueRole: "value"
                onModelChanged: page.syncThemeCombo()
                onActivated: function(index) {
                    if (page.service && index >= 0 && index < themeModel.length) {
                        page.service.setTheme(themeModel[index].value)
                    }
                }
            }
        }
    }
}
