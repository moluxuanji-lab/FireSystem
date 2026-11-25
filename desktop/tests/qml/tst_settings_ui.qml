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
        svc.navigateTo("settings")
        waitForRendering(win.contentItem)
        const navTemplatesUnlocked = findChild(win.contentItem, "navCardTemplates")
        const transferCardUnlocked = findChild(win.contentItem, "settingsTransferCard")
        verify(navTemplatesUnlocked !== null && navTemplatesUnlocked.visible)
        verify(transferCardUnlocked !== null && transferCardUnlocked.visible)

        // Trust catalog visible in About page when unlocked
        svc.navigateTo("settings-about")
        waitForRendering(win.contentItem)
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
        const pwdInput = findChild(win.contentItem, "editAccountPasswordInput")
        const saveBtn = findChild(win.contentItem, "saveAccountBtn")
        verify(idInput !== null && pwdInput !== null && saveBtn !== null)
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
