#include <QtTest>

#include <QCryptographicHash>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPasswordDigestor>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimeZone>

#include "core/AccountManager.h"
#include "core/I18nCatalog.h"
#include "core/OfflineFileManager.h"
#include "core/PreferencesManager.h"
#include "core/SettingsService.h"
#include "core/TemplateReviewManager.h"

using namespace FireSystem;

class TestSettingsCore : public QObject
{
    Q_OBJECT

private slots:
    void testPbkdf2Sha256_600000Rounds();
    void testAccountCrudRbacAndSelfProtection();
    void testLastAdminOfflineRecovery();
    void testLicenseWorkflowAndExpiry();
    void testSettingsExportImportV1AndLegacyV2();
    void testTemplateReviewAndI18n6LanguagesAndTheme();
    void testAccessGating();
};

void TestSettingsCore::testPbkdf2Sha256_600000Rounds()
{
    QCOMPARE(AccountManager::kDefaultPbkdf2Rounds, 600000);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AccountManager mgr(dir.path());
    QCOMPARE(mgr.pbkdf2Rounds(), 600000);

    QString err;
    QVERIFY2(mgr.initialize(&err), qPrintable(err));
    QVERIFY(mgr.isInitialized());
    QCOMPARE(mgr.generation(), 1);

    // Preset logins: debug / "lince", admin / ""
    QVERIFY2(mgr.login(QStringLiteral("debug"), QStringLiteral("lince"), &err), qPrintable(err));
    QVERIFY(mgr.current().has_value());
    QCOMPARE(mgr.current()->id, QStringLiteral("debug"));
    QCOMPARE(mgr.current()->role, QStringLiteral("debugger"));

    QVERIFY(!mgr.login(QStringLiteral("debug"), QStringLiteral("wrong"), &err));
    QCOMPARE(err, QStringLiteral("账号或密码错误"));

    QVERIFY2(mgr.login(QStringLiteral("admin"), QString(), &err), qPrintable(err));
    QVERIFY(mgr.current().has_value());
    QCOMPARE(mgr.current()->id, QStringLiteral("admin"));
    QCOMPARE(mgr.current()->role, QStringLiteral("admin"));

    // Verify stored schema v3 + 600,000-round PBKDF2-HMAC-SHA256 digest matches independent derivation
    QFile dbFile(mgr.storageFilePath());
    QVERIFY(dbFile.open(QIODevice::ReadOnly));
    const QJsonObject db = QJsonDocument::fromJson(dbFile.readAll()).object();
    QCOMPARE(db.value(QStringLiteral("schemaVersion")).toInt(), 3);
    QCOMPARE(db.value(QStringLiteral("presetRevision")).toInt(), 1);

    const QJsonObject debugObj = db.value(QStringLiteral("debug")).toObject();
    const QString saltHex = debugObj.value(QStringLiteral("salt")).toString();
    const QString hashHex = debugObj.value(QStringLiteral("hash")).toString();
    QCOMPARE(saltHex.size(), 32);
    QCOMPARE(hashHex.size(), 64);

    const QByteArray expectedDigest = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        QByteArrayLiteral("lince"),
        QByteArray::fromHex(saltHex.toLatin1()),
        600000,
        32).toHex();
    QCOMPARE(hashHex.toLatin1(), expectedDigest);
}

void TestSettingsCore::testAccountCrudRbacAndSelfProtection()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AccountManager mgr(dir.path(), 1000);
    QString err;
    QVERIFY(mgr.initialize(&err));
    QVERIFY(mgr.login(QStringLiteral("admin"), QString(), &err));

    // Create operator & disabled admin
    QVERIFY2(mgr.save(QString(), QStringLiteral("op1"), QStringLiteral("operator"), true, true, QStringLiteral("pass1"), &err), qPrintable(err));
    QVERIFY2(mgr.save(QString(), QStringLiteral("adm2"), QStringLiteral("admin"), false, true, QStringLiteral("pass2"), &err), qPrintable(err));
    QCOMPARE(mgr.generation(), 3);

    // Duplicate ID rejected
    QVERIFY(!mgr.save(QString(), QStringLiteral("op1"), QStringLiteral("operator"), true, true, QStringLiteral("x"), &err));
    QCOMPARE(err, QStringLiteral("此账号已存在"));

    // Reserved "debug" rejected
    QVERIFY(!mgr.save(QString(), QStringLiteral("Debug"), QStringLiteral("admin"), true, true, QStringLiteral("x"), &err));
    QCOMPARE(err, QStringLiteral("请检查账号和角色"));

    // Self-protection: admin cannot disable self, change own role, reset own password via save(), or delete self
    QVERIFY(!mgr.save(QStringLiteral("admin"), QStringLiteral("admin"), QStringLiteral("operator"), true, false, QString(), &err));
    QCOMPARE(err, QStringLiteral("请在登录页修改自己的密码"));
    QVERIFY(!mgr.save(QStringLiteral("admin"), QStringLiteral("admin"), QStringLiteral("admin"), false, false, QString(), &err));
    QCOMPARE(err, QStringLiteral("请在登录页修改自己的密码"));
    QVERIFY(!mgr.save(QStringLiteral("admin"), QStringLiteral("admin"), QStringLiteral("admin"), true, true, QStringLiteral("new"), &err));
    QCOMPARE(err, QStringLiteral("请在登录页修改自己的密码"));
    QVERIFY(!mgr.remove(QStringLiteral("admin"), &err));
    QCOMPARE(err, QStringLiteral("此账号无法删除"));
    QVERIFY(!mgr.remove(QStringLiteral("debug"), &err));
    QCOMPARE(err, QStringLiteral("此账号无法删除"));

    // Own password change requires valid current password and non-empty new password
    QVERIFY(!mgr.changePassword(QStringLiteral("wrong"), QStringLiteral("newpass"), &err));
    QCOMPARE(err, QStringLiteral("当前密码错误"));
    QVERIFY(!mgr.changePassword(QString(), QString(), &err));
    QCOMPARE(err, QStringLiteral("请输入密码"));
    QVERIFY2(mgr.changePassword(QString(), QStringLiteral("adminNew"), &err), qPrintable(err));
    QVERIFY(mgr.current().has_value());

    // Operator RBAC enforcement
    QVERIFY(mgr.login(QStringLiteral("op1"), QStringLiteral("pass1"), &err));
    QVERIFY(!AccountManager::can(QStringLiteral("operator"), QStringLiteral("accounts")));
    QVERIFY(mgr.list(&err).isEmpty());
    QCOMPARE(err, QStringLiteral("需要账号管理权限"));
    QVERIFY(!mgr.save(QString(), QStringLiteral("op2"), QStringLiteral("operator"), true, true, QStringLiteral("x"), &err));
    QCOMPARE(err, QStringLiteral("需要账号管理权限"));
    QVERIFY(!mgr.remove(QStringLiteral("adm2"), &err));
    QCOMPARE(err, QStringLiteral("需要账号管理权限"));

    // Re-login as admin with new password and remove op1
    QVERIFY(mgr.login(QStringLiteral("admin"), QStringLiteral("adminNew"), &err));
    QVERIFY2(mgr.remove(QStringLiteral("op1"), &err), qPrintable(err));
    const QVariantList rows = mgr.list(&err);
    QCOMPARE(rows.size(), 3); // debug, admin, adm2
}

void TestSettingsCore::testLastAdminOfflineRecovery()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AccountManager mgr(dir.path(), 1000);
    OfflineFileManager files(dir.path(), &mgr);
    QString err;
    QVERIFY(mgr.initialize(&err));

    // While admin is enabled and signed out, recovery is NOT allowed
    QVERIFY(!mgr.canRecover());

    // Debugger disables the only admin account
    QVERIFY(mgr.login(QStringLiteral("debug"), QStringLiteral("lince"), &err));
    QVERIFY2(mgr.save(QStringLiteral("admin"), QStringLiteral("admin"), QStringLiteral("admin"), false, false, QString(), &err), qPrintable(err));
    mgr.logout();

    // Now signed out and all non-pending admins are disabled -> canRecover() is true!
    QVERIFY(mgr.canRecover());

    const auto reqOpt = files.exportRequest(QStringLiteral("recovery"), &err);
    QVERIFY2(reqOpt.has_value(), qPrintable(err));
    const QJsonObject req = *reqOpt;
    QCOMPARE(req.value(QStringLiteral("version")).toInt(), 2);
    QCOMPARE(req.value(QStringLiteral("type")).toString(), QStringLiteral("firesystem.prototype.recovery.request"));
    QCOMPARE(req.value(QStringLiteral("generation")).toInt(), mgr.generation());

    // Idempotent until applied
    const auto req2 = files.exportRequest(QStringLiteral("recovery"), &err);
    QVERIFY(req2.has_value());
    QCOMPARE(req2->value(QStringLiteral("id")).toString(), req.value(QStringLiteral("id")).toString());

    // Disabled admin cannot sign in
    QVERIFY(!mgr.login(QStringLiteral("admin"), QString(), &err));

    // Only debugger can issue recovery files
    const QJsonObject legacyReq{{QStringLiteral("type"), QStringLiteral("firesystem.prototype.request")}};
    QVERIFY(mgr.login(QStringLiteral("debug"), QStringLiteral("lince"), &err));
    QVERIFY(!files.checkRequest(QStringLiteral("recovery"), legacyReq, &err).has_value());
    QCOMPARE(err, QStringLiteral("请在目标电脑重新导出新版请求文件。"));

    const auto licReq = files.exportRequest(QStringLiteral("license"), &err);
    QVERIFY(licReq.has_value());
    QVERIFY(!files.checkRequest(QStringLiteral("recovery"), *licReq, &err).has_value());
    QCOMPARE(err, QStringLiteral("请选择账号恢复请求文件。"));

    const auto respOpt = files.issue(QStringLiteral("recovery"), req, QStringLiteral("years"), 1, QString(), &err);
    QVERIFY2(respOpt.has_value(), qPrintable(err));
    QCOMPARE(respOpt->value(QStringLiteral("type")).toString(), QStringLiteral("firesystem.prototype.recovery"));
    QCOMPARE(respOpt->value(QStringLiteral("demo")).toBool(), true);

    mgr.logout();
    QVERIFY2(files.apply(QStringLiteral("recovery"), *respOpt, &err), qPrintable(err));
    QVERIFY(!files.localRequest(QStringLiteral("recovery")).has_value());

    // Admin is now enabled and can log in!
    QVERIFY2(mgr.login(QStringLiteral("admin"), QString(), &err), qPrintable(err));
}

void TestSettingsCore::testLicenseWorkflowAndExpiry()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AccountManager mgr(dir.path(), 1000);
    OfflineFileManager files(dir.path(), &mgr);
    QString err;
    QVERIFY(mgr.initialize(&err));

    // Fresh workstation starts unlicensed
    QVERIFY(!files.licenseStatus().licensed);

    // Unlicensed workstation can export license request even while signed out
    const auto reqOpt = files.exportRequest(QStringLiteral("license"), &err);
    QVERIFY2(reqOpt.has_value(), qPrintable(err));
    QCOMPARE(reqOpt->value(QStringLiteral("version")).toInt(), 2);
    QCOMPARE(reqOpt->value(QStringLiteral("type")).toString(), QStringLiteral("firesystem.prototype.license.request"));

    // Expiry math validation
    QVERIFY(OfflineFileManager::computeLicenseEndDate(QStringLiteral("years"), 0, QString(), &err).isEmpty());
    QCOMPARE(err, QStringLiteral("请输入有效的许可年数。"));
    QVERIFY(OfflineFileManager::computeLicenseEndDate(QStringLiteral("date"), 1, QStringLiteral("2020-01-01"), &err).isEmpty());
    QCOMPARE(err, QStringLiteral("许可有效期无效或已到期"));

    const QString futureDate = QDateTime::currentDateTimeUtc().date().addYears(2).toString(Qt::ISODate);
    QCOMPARE(OfflineFileManager::computeLicenseEndDate(QStringLiteral("date"), 1, futureDate, &err), futureDate);

    // Debugger issues license
    QVERIFY(mgr.login(QStringLiteral("debug"), QStringLiteral("lince"), &err));
    const auto respOpt = files.issue(QStringLiteral("license"), *reqOpt, QStringLiteral("years"), 1, QString(), &err);
    QVERIFY2(respOpt.has_value(), qPrintable(err));
    QCOMPARE(respOpt->value(QStringLiteral("scope")).toString(), QStringLiteral("workstation"));
    QCOMPARE(respOpt->value(QStringLiteral("demo")).toBool(), true);

    // Reject non-demo response
    QJsonObject nonDemo = *respOpt;
    nonDemo.insert(QStringLiteral("demo"), false);
    QVERIFY(!files.apply(QStringLiteral("license"), nonDemo, &err));
    QCOMPARE(err, QStringLiteral("请选择由新版请求签发的对应文件"));

    // Apply valid response
    QVERIFY2(files.apply(QStringLiteral("license"), *respOpt, &err), qPrintable(err));
    QVERIFY(files.licenseStatus().licensed);
    QCOMPARE(files.licenseStatus().notAfter, respOpt->value(QStringLiteral("notAfter")).toString());
    QVERIFY(!files.localRequest(QStringLiteral("license")).has_value());

    // Clear license as admin
    QVERIFY(mgr.login(QStringLiteral("admin"), QString(), &err));
    QVERIFY2(files.clearLicense(&err), qPrintable(err));
    QVERIFY(!files.licenseStatus().licensed);
}

void TestSettingsCore::testSettingsExportImportV1AndLegacyV2()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SettingsService svc(dir.path(), 1000);
    QVERIFY(svc.signIn(QStringLiteral("debug"), QStringLiteral("lince")).isEmpty());

    // Activate license first so canManage() == true
    const QString reqPath = dir.filePath(QStringLiteral("lic-req.json"));
    const QString licPath = dir.filePath(QStringLiteral("lic.json"));
    QVERIFY(svc.exportLicenseRequestToFile(reqPath));
    QVERIFY(svc.importLicenseRequestForIssue(reqPath));
    QVERIFY(svc.exportIssuedLicenseToFile(licPath, QStringLiteral("years"), 1, QString()));
    QVERIFY(svc.importLicenseFromFile(licPath));
    QVERIFY(svc.unlocked());
    QVERIFY(svc.canManage());

    // Create an extra operator and set language/theme
    QVERIFY(svc.saveAccount(QString(), QStringLiteral("op_exp"), QStringLiteral("operator"), true, true, QStringLiteral("oppass")));
    QVERIFY(svc.setLanguage(QStringLiteral("en")));
    QVERIFY(svc.setTheme(QStringLiteral("dark")));

    // Export settings v1
    const QString settingsPath = dir.filePath(QStringLiteral("settings-v1.json"));
    QVERIFY(svc.exportSettingsToFile(settingsPath));

    QFile exportedFile(settingsPath);
    QVERIFY(exportedFile.open(QIODevice::ReadOnly));
    const QJsonObject exportedObj = QJsonDocument::fromJson(exportedFile.readAll()).object();
    QCOMPARE(exportedObj.value(QStringLiteral("type")).toString(), QStringLiteral("firesystem.prototype.settings"));
    QCOMPARE(exportedObj.value(QStringLiteral("version")).toInt(), 1);
    QCOMPARE(exportedObj.value(QStringLiteral("appearance")).toObject().value(QStringLiteral("lang")).toString(), QStringLiteral("en"));
    QCOMPARE(exportedObj.value(QStringLiteral("appearance")).toObject().value(QStringLiteral("theme")).toString(), QStringLiteral("dark"));
    const QJsonArray exportedAccounts = exportedObj.value(QStringLiteral("accounts")).toArray();
    QCOMPARE(exportedAccounts.size(), 2);
    for (const QJsonValue &v : exportedAccounts) {
        QVERIFY(!v.toObject().contains(QStringLiteral("token")));
    }

    // Reject oversized (>256 KB) JSON import
    const QString bigPath = dir.filePath(QStringLiteral("oversized.json"));
    {
        QFile bigFile(bigPath);
        QVERIFY(bigFile.open(QIODevice::WriteOnly));
        bigFile.write(QByteArray(260 * 1024, ' '));
    }
    QVERIFY(!svc.prepareSettingsImportFromFile(bigPath));
    QVERIFY(!svc.hasPendingSettingsImport());

    // Mutate local state, then re-import v1
    QVERIFY(svc.deleteAccount(QStringLiteral("op_exp")));
    QVERIFY(svc.setLanguage(QStringLiteral("fr")));
    QVERIFY(svc.setTheme(QStringLiteral("light")));

    QVERIFY(svc.prepareSettingsImportFromFile(settingsPath));
    QVERIFY(svc.hasPendingSettingsImport());
    QVERIFY(svc.pendingImportHasAppearance());
    QCOMPARE(svc.pendingImportAccountCount(), 2);
    QVERIFY(svc.confirmSettingsImport());
    QVERIFY(!svc.signedIn());
    QCOMPARE(svc.currentPage(), QStringLiteral("login"));
    QCOMPARE(svc.lang(), QStringLiteral("en"));
    QCOMPARE(svc.theme(), QStringLiteral("dark"));

    // Legacy v2 backup import (appearance untouched)
    QVERIFY(svc.signIn(QStringLiteral("op_exp"), QStringLiteral("oppass")).isEmpty()); // verifies restored op_exp
    QVERIFY(svc.signIn(QStringLiteral("admin"), QString()).isEmpty());
    QVERIFY(svc.setLanguage(QStringLiteral("de")));

    const QString legacyPath = dir.filePath(QStringLiteral("legacy-v2.json"));
    {
        const QJsonObject legacyObj{
            {QStringLiteral("type"), QStringLiteral("firesystem.prototype.backup")},
            {QStringLiteral("version"), 2},
            {QStringLiteral("accounts"), exportedAccounts},
        };
        QFile f(legacyPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QJsonDocument(legacyObj).toJson());
    }
    QVERIFY(svc.prepareSettingsImportFromFile(legacyPath));
    QVERIFY(svc.hasPendingSettingsImport());
    QVERIFY(!svc.pendingImportHasAppearance());
    QVERIFY(svc.confirmSettingsImport());
    QCOMPARE(svc.lang(), QStringLiteral("de")); // unchanged!
}

void TestSettingsCore::testTemplateReviewAndI18n6LanguagesAndTheme()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    {
        SettingsService svc(dir.path(), 1000);
        QVERIFY(svc.signIn(QStringLiteral("admin"), QString()).isEmpty());

        // License workstation
        const QString reqPath = dir.filePath(QStringLiteral("req.json"));
        const QString licPath = dir.filePath(QStringLiteral("lic.json"));
        QVERIFY(svc.exportLicenseRequestToFile(reqPath));
        QVERIFY(svc.signIn(QStringLiteral("debug"), QStringLiteral("lince")).isEmpty());
        QVERIFY(svc.importLicenseRequestForIssue(reqPath));
        QVERIFY(svc.exportIssuedLicenseToFile(licPath, QStringLiteral("years"), 1, QString()));
        QVERIFY(svc.importLicenseFromFile(licPath));

        QCOMPARE(svc.templateId(), QStringLiteral("sim.baseline_ring"));
        QCOMPARE(svc.templateBoardId(), QStringLiteral("kzb_f407"));
        QCOMPARE(svc.templateVersion(), QStringLiteral("1.2.1"));
        QCOMPARE(svc.templateReviewStatus(), QStringLiteral("待复核"));

        QVERIFY(svc.recordTemplateReview());
        QCOMPARE(svc.templateReviewStatus(), QStringLiteral("已复核"));

        const QString manifestPath = dir.filePath(QStringLiteral("template-review.json"));
        QVERIFY(svc.exportTemplateReviewToFile(manifestPath));
        QFile f(manifestPath);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QJsonObject m = QJsonDocument::fromJson(f.readAll()).object();
        QCOMPARE(m.value(QStringLiteral("demo")).toBool(), true);
        QCOMPARE(m.value(QStringLiteral("deployable")).toBool(), false);
        QCOMPARE(m.value(QStringLiteral("template")).toString(), QStringLiteral("sim.baseline_ring"));
        QCOMPARE(m.value(QStringLiteral("boardId")).toString(), QStringLiteral("kzb_f407"));
        QCOMPARE(m.value(QStringLiteral("review")).toString(), QStringLiteral("已复核"));

        QVERIFY(svc.setLanguage(QStringLiteral("ru")));
        QVERIFY(svc.setTheme(QStringLiteral("dark")));
    }

    // Persistence check across new SettingsService instance
    {
        SettingsService svc2(dir.path(), 1000);
        QCOMPARE(svc2.lang(), QStringLiteral("ru"));
        QCOMPARE(svc2.theme(), QStringLiteral("dark"));
        QCOMPARE(svc2.templateReviewStatus(), QStringLiteral("已复核"));
    }

    // Verify all 6 languages translate core strings & {0} placeholder
    I18nCatalog i18n;
    const QStringList langs = {
        QStringLiteral("zh-CN"), QStringLiteral("en"), QStringLiteral("ru"),
        QStringLiteral("de"), QStringLiteral("fr"), QStringLiteral("es")
    };
    QSet<QString> translations;
    for (const QString &l : langs) {
        QVERIFY(i18n.setLang(l));
        const QString translated = i18n.tr(QStringLiteral("设置"));
        QVERIFY(!translated.isEmpty());
        translations.insert(translated);
        const QString withArg = i18n.tr(QStringLiteral("将用文件中的 {0} 个账号替换本机的操作员和管理员账号，并退出登录。"), QStringLiteral("3"));
        QVERIFY(withArg.contains(QLatin1String("3")));
    }
    QCOMPARE(translations.size(), 6);
}

void TestSettingsCore::testAccessGating()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SettingsService svc(dir.path(), 1000);
    QVERIFY(!svc.licensed());
    QVERIFY(!svc.signedIn());
    QVERIFY(!svc.unlocked());
    QCOMPARE(svc.currentPage(), QStringLiteral("settings-license"));

    // Attempting to navigate to settings-templates while locked redirects to accessLanding()
    QSignalSpy toastSpy(&svc, &SettingsService::toastRequested);
    svc.navigateTo(QStringLiteral("settings-templates"));
    QCOMPARE(svc.currentPage(), QStringLiteral("settings-license"));
    QCOMPARE(toastSpy.count(), 1);

    // Other settings pages are available even when locked
    svc.navigateTo(QStringLiteral("settings"));
    QCOMPARE(svc.currentPage(), QStringLiteral("settings"));
    svc.navigateTo(QStringLiteral("settings-general"));
    QCOMPARE(svc.currentPage(), QStringLiteral("settings-general"));
    svc.navigateTo(QStringLiteral("settings-about"));
    QCOMPARE(svc.currentPage(), QStringLiteral("settings-about"));
    svc.navigateTo(QStringLiteral("settings-accounts"));
    QCOMPARE(svc.currentPage(), QStringLiteral("settings-accounts"));
}

QTEST_MAIN(TestSettingsCore)
#include "tst_settings_core.moc"
