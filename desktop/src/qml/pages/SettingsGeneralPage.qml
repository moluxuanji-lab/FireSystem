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

    function t(key, arg0) {
        if (!i18n) return key
        return i18n.t(key, i18n.revision, arg0 === undefined ? "" : String(arg0))
    }

    function roleLabel(role) {
        if (role === "admin") return t("管理员")
        if (role === "debugger") return t("调试员")
        return t("操作员")
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
        pageTitle: page.t("通用")
    }

    SectionCard {
        Layout.fillWidth: true
        Layout.maximumWidth: 920

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            FluText {
                Layout.fillWidth: true
                text: page.t("语言")
                font: FluTextStyle.BodyStrong
            }

            FluComboBox {
                id: languageCombo
                objectName: "languageCombo"
                Layout.preferredWidth: 240
                model: page.i18n ? page.i18n.languageOptions : []
                textRole: "label"
                valueRole: "value"
                currentIndex: {
                    if (!page.service || !page.i18n) return 0
                    const opts = page.i18n.languageOptions
                    for (let i = 0; i < opts.length; ++i) {
                        if (opts[i].value === page.service.lang) return i
                    }
                    return 0
                }
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

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: FluTheme.dividerColor
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            FluText {
                Layout.fillWidth: true
                text: page.t("界面主题")
                font: FluTextStyle.BodyStrong
            }

            FluComboBox {
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
                currentIndex: {
                    if (!page.service) return 0
                    for (let i = 0; i < themeModel.length; ++i) {
                        if (themeModel[i].value === page.service.theme) return i
                    }
                    return 0
                }
                onActivated: function(index) {
                    if (page.service && index >= 0 && index < themeModel.length) {
                        page.service.setTheme(themeModel[index].value)
                    }
                }
            }
        }
    }
}
