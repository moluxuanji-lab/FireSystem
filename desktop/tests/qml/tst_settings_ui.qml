// qmllint disable unqualified
import QtQuick
import QtTest
import FluentUI
import FireSystem.UI

TestCase {
    id: root
    name: "SettingsUI"
    when: windowShown
    width: 1120
    height: 740

    Component {
        id: mainComp
        Main {}
    }

    function activateLicenseViaUi(win) {
        // Sign in as debug ("lince")
        win.service.navigateTo("login")
        waitForRendering(win.contentItem)
        const accInput = findChild(win.contentItem, "loginAccountInput")
        const pwdInput = findChild(win.contentItem, "loginPasswordInput")
        const submitBtn = findChild(win.contentItem, "loginSubmitButton")
        verify(accInput !== null)
        verify(pwdInput !== null)
        verify(submitBtn !== null)
        accInput.text = "debug"
        pwdInput.text = "lince"
        submitBtn.clicked()
        tryCompare(win.service, "signedIn", true)

        // Export license request, issue it, and import it
        const reqUrl = SettingsTestFactory.newTempFileUrl("ui-lic-req.json")
        const licUrl = SettingsTestFactory.newTempFileUrl("ui-lic.json")
        verify(win.service.exportLicenseRequestToFile(reqUrl))
        verify(win.service.importLicenseRequestForIssue(reqUrl))
        verify(win.service.exportIssuedLicenseToFile(licUrl, "years", 1, ""))
        verify(win.service.importLicenseFromFile(licUrl))
        tryCompare(win.service, "unlocked", true)
    }

    function test_locked_vs_unlocked_gating() {
        const svc = SettingsTestFactory.createService()
        const win = createTemporaryObject(mainComp, root, { service: svc })
        verify(win !== null)
        waitForRendering(win.contentItem)

        // Locked initially: templates card is hidden in Settings hub
        svc.navigateTo("settings")
        waitForRendering(win.contentItem)
        const navAccounts = findChild(win.contentItem, "navCardAccounts")
        const navTemplates = findChild(win.contentItem, "navCardTemplates")
        const transferCard = findChild(win.contentItem, "settingsTransferCard")
        verify(navAccounts !== null && navAccounts.visible)
        verify(navTemplates !== null && !navTemplates.visible)
        verify(transferCard !== null && !transferCard.visible)

        // Gated navigation redirects back to settings-license
        svc.navigateTo("settings-templates")
        compare(svc.currentPage, "settings-license")

        // Activate license as debug -> unlocked
        activateLicenseViaUi(win)

        // Verify header FluDropDownButton displays account id + localized role and has 3 menu items (not 4)
        const profileDropdown = findChild(win.contentItem, "headerProfileDropdown")
        verify(profileDropdown !== null && profileDropdown.visible)
        verify(profileDropdown.text.indexOf("debug") !== -1)
        verify(profileDropdown.text.indexOf("调试员") !== -1)
        compare(profileDropdown.contentData.length, 3)
        let foundMenu = null
        for (let r = 0; r < profileDropdown.resources.length; ++r) {
            const res = profileDropdown.resources[r]
            if (res && res.count !== undefined) {
                foundMenu = res
                break
            }
        }
        verify(foundMenu !== null)
        compare(foundMenu.count, 3)

        svc.setLanguage("en")
        tryCompare(profileDropdown, "text", "debug · Debugger")
        svc.setLanguage("zh-CN")
        tryCompare(profileDropdown, "text", "debug · 调试员")

        svc.navigateTo("settings")
        waitForRendering(win.contentItem)
        const navTemplatesUnlocked = findChild(win.contentItem, "navCardTemplates")
        const transferCardUnlocked = findChild(win.contentItem, "settingsTransferCard")
        verify(navTemplatesUnlocked !== null && navTemplatesUnlocked.visible)
        verify(transferCardUnlocked !== null && transferCardUnlocked.visible)

        // Trust catalog & About table columnSource stability across language switch
        svc.navigateTo("settings-about")
        waitForRendering(win.contentItem)
        const compatTable = findChild(win.contentItem, "compatTable")
        verify(compatTable !== null)
        const initialColumns = compatTable.columnSource
        compare(initialColumns.length, 3)
        svc.setLanguage("en")
        waitForRendering(win.contentItem)
        compare(compatTable.columnSource, initialColumns)
        svc.setLanguage("zh-CN")
        waitForRendering(win.contentItem)
        compare(compatTable.columnSource, initialColumns)
        const trustCard = findChild(win.contentItem, "trustCatalogCard")
        const trustBtn = findChild(win.contentItem, "trustDetailBtn")
        const trustBanner = findChild(win.contentItem, "trustDetailBanner")
        verify(trustCard !== null && trustCard.visible)
        verify(trustBanner !== null && !trustBanner.visible)
        trustBtn.clicked()
        tryCompare(trustBanner, "visible", true)

        // Clear license via UI button & confirm
        svc.navigateTo("settings-license")
        waitForRendering(win.contentItem)
        const clearBtn = findChild(win.contentItem, "clearLicenseBtn")
        verify(clearBtn !== null && clearBtn.visible && !clearBtn.disabled)
        clearBtn.clicked()
        const confirmClearBtn = findChild(win.contentItem, "confirmClearLicenseBtn")
        verify(confirmClearBtn !== null)
        confirmClearBtn.clicked()
        tryCompare(svc, "licensed", false)
        tryCompare(svc, "unlocked", false)
    }

    function test_workbench_navigation_and_placeholders() {
        const svc = SettingsTestFactory.createService()
        const win = createTemporaryObject(mainComp, root, { service: svc })
        verify(win !== null)
        waitForRendering(win.contentItem)
        const rail = findChild(win.contentItem, "mainNavView")
        const settings = findChild(rail, "nav-settings")
        mouseClick(settings)
        tryCompare(svc, "currentPage", "settings")
        mouseClick(findChild(win.contentItem, "navCardGeneral"))
        tryCompare(svc, "currentPage", "settings-general")
        verify(settings.selected)
        verify(findChild(rail, "nav-monitor") === null)

        mouseClick(findChild(rail, "nav-help"))
        tryCompare(svc, "currentPage", "help")
        const loader = findChild(win.contentItem, "pageLoader")
        compare(loader.item.objectName, "placeholderPage")
        verify(!settings.selected)
        svc.navigateTo("monitor")
        tryCompare(svc, "currentPage", "settings-license")

        activateLicenseViaUi(win)
        waitForRendering(win.contentItem)
        const monitor = findChild(rail, "nav-monitor")
        verify(monitor && monitor.visible)
        mouseClick(monitor)
        tryCompare(svc, "currentPage", "monitor")
        compare(loader.item.objectName, "placeholderPage")
        verify(monitor.selected)
        mouseClick(settings)
        tryCompare(svc, "currentPage", "settings")
        verify(findChild(loader.item, "navCardTemplates").visible)

        win.width = 720
        tryCompare(rail, "visible", false)
        waitForRendering(win.contentItem)
        mouseClick(findChild(win.contentItem, "toggleNavigationButton"))
        tryCompare(rail, "visible", true)
        waitForRendering(rail)
        mouseClick(findChild(rail, "nav-help"))
        tryCompare(svc, "currentPage", "help")
        tryCompare(rail, "visible", false)
    }

    function test_live_6_language_and_3_theme_switching() {
        const svc = SettingsTestFactory.createService()
        const win = createTemporaryObject(mainComp, root, { service: svc })
        verify(win !== null)
        svc.navigateTo("settings-general")
        waitForRendering(win.contentItem)

        const langCombo = findChild(win.contentItem, "languageCombo")
        const themeCombo = findChild(win.contentItem, "themeCombo")
        verify(langCombo !== null)
        verify(themeCombo !== null)

        // Switch through all 6 languages live
        const expectedTitles = {
            "zh-CN": "FireSystem · 设置",
            "en": "FireSystem · Settings",
            "ru": "FireSystem · Настройки",
            "de": "FireSystem · Einstellungen",
            "fr": "FireSystem · Paramètres",
            "es": "FireSystem · Configuración"
        }
        const langs = ["zh-CN", "en", "ru", "de", "fr", "es"]
        for (let i = 0; i < langs.length; ++i) {
            svc.setLanguage(langs[i])
            tryCompare(win, "title", expectedTitles[langs[i]])
        }

        // Switch themes
        svc.setTheme("dark")
        tryCompare(FluTheme, "darkMode", FluThemeType.Dark)
        svc.setTheme("light")
        tryCompare(FluTheme, "darkMode", FluThemeType.Light)
        svc.setTheme("system")
        tryCompare(FluTheme, "darkMode", FluThemeType.System)

        // Interactive combo selection followed by programmatic updates must keep currentIndex in sync
        themeCombo.currentIndex = 2
        themeCombo.activated(2)
        compare(svc.theme, "dark")
        compare(themeCombo.currentIndex, 2)
        svc.setTheme("light")
        tryCompare(themeCombo, "currentIndex", 1)

        langCombo.currentIndex = 1 // "en"
        langCombo.activated(1)
        compare(svc.lang, "en")
        compare(langCombo.currentIndex, 1)
        svc.setLanguage("de") // index 3
        tryCompare(langCombo, "currentIndex", 3)

        // Switching language while theme is "dark" preserves themeCombo.currentIndex
        svc.setTheme("dark")
        tryCompare(themeCombo, "currentIndex", 2)
        svc.setLanguage("zh-CN")
        tryCompare(themeCombo, "currentIndex", 2)
    }

    function test_accounts_crud_and_templates_review() {
        const svc = SettingsTestFactory.createService()
        const win = createTemporaryObject(mainComp, root, { service: svc })
        verify(win !== null)
        activateLicenseViaUi(win)

        // Navigate to Accounts and create an operator via UI form
        svc.navigateTo("settings-accounts")
        waitForRendering(win.contentItem)
        const addBtn = findChild(win.contentItem, "addAccountBtn")
        verify(addBtn !== null)
        addBtn.clicked()

        const idInput = findChild(win.contentItem, "editAccountIdInput")
        const roleCombo = findChild(win.contentItem, "editAccountRoleCombo")
        const pwdInput = findChild(win.contentItem, "editAccountPasswordInput")
        const saveBtn = findChild(win.contentItem, "saveAccountBtn")
        verify(idInput !== null && roleCombo !== null && pwdInput !== null && saveBtn !== null)

        // Interact with roleCombo (select admin, then back to operator via programmatic form reset)
        roleCombo.currentIndex = 1
        roleCombo.activated(1)
        compare(roleCombo.currentIndex, 1)
        addBtn.clicked()
        tryCompare(roleCombo, "currentIndex", 0)

        idInput.text = "ui_op1"
        pwdInput.text = "opsecret"
        saveBtn.clicked()

        tryCompare(svc.accountsModel, "length", 3)

        // Search filter
        const searchBox = findChild(win.contentItem, "accountSearchBox")
        verify(searchBox !== null)
        searchBox.text = "ui_op1"
        tryCompare(svc.accountsModel, "length", 1)
        searchBox.text = ""
        tryCompare(svc.accountsModel, "length", 3)

        // Templates review
        svc.navigateTo("settings-templates")
        waitForRendering(win.contentItem)
        const reviewBtn = findChild(win.contentItem, "recordReviewBtn")
        verify(reviewBtn !== null && !reviewBtn.disabled)
        reviewBtn.clicked()
        tryCompare(svc, "templateReviewStatus", "已复核")
        tryCompare(reviewBtn, "disabled", true)
    }
}
