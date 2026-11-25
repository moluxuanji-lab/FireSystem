#include "SettingsService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimeZone>
#include <QUrl>

namespace FireSystem {

SettingsService::SettingsService(const QString &storageDir, int pbkdf2Rounds, QObject *parent)
    : QObject(parent)
    , m_storageDir(storageDir)
    , m_accounts(storageDir, pbkdf2Rounds)
    , m_files(storageDir, &m_accounts)
    , m_prefs(storageDir)
    , m_templates(storageDir)
{
    const PreferencesData prefs = m_prefs.load();
    m_i18n.setLang(prefs.lang);
    m_theme = prefs.theme;

    QString initErr;
    m_accounts.initialize(&initErr);

    m_currentPage = accessLanding();
}

namespace {

// The sidebar, settings directory and route guard use the same page hierarchy.
struct PageEntry {
    const char *id;
    const char *label;
    const char *glyph;
    const char *section;
};

constexpr PageEntry pages[] = {
    {"operations", "运行监控", "pulse", ""},
    {"overview", "运行总览", "grid", "operations"},
    {"monitor", "设备监控", "pulse", "operations"},
    {"alarms", "告警中心", "bell", "operations"},
    {"engineering", "工程配置", "sliders", ""},
    {"config", "参数配置", "sliders", "engineering"},
    {"protocol", "协议设计", "network", "engineering"},
    {"workflow", "流程设计", "flow", "engineering"},
    {"maintenance", "维护工具", "settings", ""},
    {"controls", "控制与气瓶", "bottle", "maintenance"},
    {"upgrade", "固件升级", "up", "maintenance"},
    {"logs", "日志与审计", "log", "maintenance"},
    {"terminal", "终端工具", "terminal", "maintenance"},
    {"settings", "设置", "settings", ""},
    {"settings-accounts", "账号", "user", "settings"},
    {"settings-license", "许可证", "key", "settings"},
    {"settings-templates", "项目模板", "folder", "settings"},
    {"settings-general", "通用", "sliders", "settings"},
    {"settings-about", "关于", "help", "settings"},
    {"help", "帮助", "help", ""},
    {"login", "登录", "user", ""},
    {"projects", "项目选择", "folder", "engineering"},
};

bool isKnownPage(const QString &id)
{
    for (const auto &page : pages) {
        if (id == QLatin1String(page.id)) return true;
    }
    return false;
}

} // namespace

QVariantList SettingsService::navigationPages() const
{
    QVariantList entries;
    for (const auto &page : pages) {
        entries.append(QVariantMap{
            {QStringLiteral("id"), QString::fromLatin1(page.id)},
            {QStringLiteral("label"), QString::fromUtf8(page.label)},
            {QStringLiteral("glyph"), QString::fromLatin1(page.glyph)},
            {QStringLiteral("section"), QString::fromLatin1(page.section)}
        });
    }
    return entries;
}

bool SettingsService::signedIn() const
{
    return m_accounts.current().has_value();
}

bool SettingsService::licensed() const
{
    return m_files.licenseStatus().licensed;
}

bool SettingsService::unlocked() const
{
    return licensed() && signedIn();
}

bool SettingsService::canManage() const
{
    if (!unlocked()) return false;
    const auto actor = m_accounts.current();
    return actor.has_value() && AccountManager::can(actor->role, QStringLiteral("accounts"));
}

bool SettingsService::canIssue() const
{
    const auto actor = m_accounts.current();
    return actor.has_value() && AccountManager::can(actor->role, QStringLiteral("issueFiles"));
}

bool SettingsService::canRecover() const
{
    return m_accounts.canRecover();
}

QString SettingsService::currentAccountId() const
{
    const auto actor = m_accounts.current();
    return actor.has_value() ? actor->id : QString();
}

QString SettingsService::currentAccountRole() const
{
    const auto actor = m_accounts.current();
    return actor.has_value() ? actor->role : QString();
}

QString SettingsService::computerName() const
{
    return m_files.computerName();
}

QString SettingsService::licenseNotAfter() const
{
    return m_files.licenseStatus().notAfter;
}

QString SettingsService::licenseRequestId() const
{
    const auto req = m_files.localRequest(QStringLiteral("license"));
    return req.has_value() ? req->value(QStringLiteral("id")).toString() : QString();
}

QString SettingsService::recoveryRequestId() const
{
    const auto req = m_files.localRequest(QStringLiteral("recovery"));
    return req.has_value() ? req->value(QStringLiteral("id")).toString() : QString();
}

int SettingsService::accountGeneration() const
{
    return m_accounts.generation();
}

QString SettingsService::lang() const
{
    return m_i18n.lang();
}

QString SettingsService::theme() const
{
    return m_theme;
}

void SettingsService::setAccountSearchQuery(const QString &query)
{
    if (m_accountSearchQuery == query) return;
    m_accountSearchQuery = query;
    emit stateChanged();
}

QVariantList SettingsService::accountsModel() const
{
    if (!canManage()) {
        return {};
    }
    QString err;
    const QVariantList all = m_accounts.list(&err);
    if (m_accountSearchQuery.trimmed().isEmpty()) {
        return all;
    }
    const QString needle = m_accountSearchQuery.toLower();
    QVariantList filtered;
    for (const QVariant &v : all) {
        const QVariantMap row = v.toMap();
        const QString id = row.value(QStringLiteral("id")).toString();
        const QString role = row.value(QStringLiteral("role")).toString();
        QString zhRole = QStringLiteral("操作员");
        if (role == QLatin1String("admin")) zhRole = QStringLiteral("管理员");
        else if (role == QLatin1String("debugger")) zhRole = QStringLiteral("调试员");
        const QString localizedRole = m_i18n.tr(zhRole);
        const QString hay = (id + QLatin1Char(' ') + zhRole + QLatin1Char(' ') + localizedRole).toLower();
        if (hay.contains(needle)) {
            filtered.append(row);
        }
    }
    return filtered;
}

QString SettingsService::templateId() const { return m_templates.templateId(); }
QString SettingsService::templateBoardId() const { return m_templates.boardId(); }
QString SettingsService::templateVersion() const { return m_templates.version(); }
QString SettingsService::templateReviewStatus() const { return m_templates.reviewStatus(); }

bool SettingsService::pendingImportHasAppearance() const
{
    return m_pendingSettings.has_value() && m_pendingSettings->hasAppearance;
}

int SettingsService::pendingImportAccountCount() const
{
    return m_pendingSettings.has_value() ? m_pendingSettings->accounts.size() : 0;
}

QVariantMap SettingsService::issuedLicenseRequest() const
{
    return m_issuedLicenseReq.has_value() ? m_issuedLicenseReq->toVariantMap() : QVariantMap{};
}

QVariantMap SettingsService::issuedRecoveryRequest() const
{
    return m_issuedRecoveryReq.has_value() ? m_issuedRecoveryReq->toVariantMap() : QVariantMap{};
}

bool SettingsService::pageAvailable(const QString &page) const
{
    if (!isKnownPage(page)) {
        return false;
    }
    if (unlocked()) {
        return true;
    }
    return page == QLatin1String("help")
        || page == QLatin1String("login")
        || page == QLatin1String("settings")
        || page == QLatin1String("settings-accounts")
        || page == QLatin1String("settings-license")
        || page == QLatin1String("settings-general")
        || page == QLatin1String("settings-about");
}

QString SettingsService::accessLanding() const
{
    return licensed() ? QStringLiteral("login") : QStringLiteral("settings-license");
}

void SettingsService::navigateTo(const QString &page)
{
    QString target = page;
    if (!isKnownPage(target)) {
        target = QStringLiteral("settings");
    }
    if (!pageAvailable(target)) {
        target = accessLanding();
        emitError(licensed() ? QStringLiteral("请先登录") : QStringLiteral("请激活模拟许可"));
    }
    if (target != m_currentPage) {
        m_pendingSettings.reset();
        m_currentPage = target;
    }
    emit stateChanged();
}

void SettingsService::refreshAccessAndClearTransients()
{
    m_pendingSettings.reset();
    m_accountSearchQuery.clear();
    m_issuedLicenseReq.reset();
    m_issuedLicenseError.clear();
    m_issuedRecoveryReq.reset();
    m_issuedRecoveryError.clear();
    if (!unlocked() && !pageAvailable(m_currentPage)) {
        m_currentPage = accessLanding();
    }
    emit stateChanged();
}

void SettingsService::emitError(const QString &zhError)
{
    m_lastError = m_i18n.tr(zhError);
    emit toastRequested(m_lastError, QStringLiteral("error"));
}

void SettingsService::emitSuccess(const QString &zhMsg)
{
    m_lastError.clear();
    emit toastRequested(m_i18n.tr(zhMsg), QStringLiteral("success"));
}

QString SettingsService::signIn(const QString &id, const QString &password)
{
    QString err;
    if (!m_accounts.login(id.trimmed(), password, &err)) {
        m_lastError = m_i18n.tr(err);
        emit stateChanged();
        return m_lastError;
    }
    m_lastError.clear();
    const auto actor = m_accounts.current();
    refreshAccessAndClearTransients();
    if (!licensed()) {
        m_currentPage = QStringLiteral("settings-license");
    } else if (actor.has_value() && actor->role == QLatin1String("admin")) {
        m_currentPage = QStringLiteral("settings-accounts");
    } else {
        m_currentPage = QStringLiteral("settings");
    }
    emit stateChanged();
    return {};
}

void SettingsService::signOut()
{
    m_accounts.logout();
    refreshAccessAndClearTransients();
    m_currentPage = QStringLiteral("login");
    emit stateChanged();
}

bool SettingsService::saveAccount(const QString &originalId,
                                  const QString &id,
                                  const QString &role,
                                  bool enabled,
                                  bool hasPassword,
                                  const QString &password)
{
    if (!canManage()) {
        emitError(QStringLiteral("需要账号管理权限"));
        return false;
    }
    QString err;
    if (!m_accounts.save(originalId, id.trimmed(), role, enabled, hasPassword, password, &err)) {
        emitError(err);
        emit stateChanged();
        return false;
    }
    emitSuccess(QStringLiteral("账号已保存"));
    emit stateChanged();
    return true;
}

bool SettingsService::changeOwnPassword(const QString &oldPassword,
                                        const QString &newPassword,
                                        const QString &confirmPassword)
{
    if (newPassword != confirmPassword) {
        emitError(QStringLiteral("两次输入的密码不一致"));
        return false;
    }
    QString err;
    if (!m_accounts.changePassword(oldPassword, newPassword, &err)) {
        emitError(err);
        emit stateChanged();
        return false;
    }
    emitSuccess(QStringLiteral("密码已更新"));
    emit stateChanged();
    return true;
}

bool SettingsService::deleteAccount(const QString &id)
{
    if (!canManage()) {
        emitError(QStringLiteral("需要账号管理权限"));
        return false;
    }
    QString err;
    if (!m_accounts.remove(id, &err)) {
        emitError(err);
        emit stateChanged();
        return false;
    }
    emitSuccess(QStringLiteral("账号已删除"));
    emit stateChanged();
    return true;
}

QString SettingsService::resolveLocalPath(const QString &fileUrlOrPath)
{
    if (fileUrlOrPath.isEmpty()) {
        return {};
    }
    const QUrl url(fileUrlOrPath);
    if (url.isValid() && url.isLocalFile()) {
        return url.toLocalFile();
    }
    return fileUrlOrPath;
}

std::optional<QJsonObject> SettingsService::readJsonFile(const QString &fileUrlOrPath, QString *errorOut) const
{
    const QString path = resolveLocalPath(fileUrlOrPath);
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        if (errorOut) *errorOut = QStringLiteral("无法读取文件，请重新选择本地文件。");
        return std::nullopt;
    }
    if (file.size() > kMaxJsonImportBytes) {
        if (errorOut) *errorOut = QStringLiteral("请选择小于 256 KB 的演示 JSON 文件");
        return std::nullopt;
    }
    const QByteArray bytes = file.readAll();
    if (bytes.size() > kMaxJsonImportBytes) {
        if (errorOut) *errorOut = QStringLiteral("请选择小于 256 KB 的演示 JSON 文件");
        return std::nullopt;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorOut) *errorOut = QStringLiteral("文件不是有效的 JSON");
        return std::nullopt;
    }
    return doc.object();
}

bool SettingsService::writeJsonFile(const QString &fileUrlOrPath, const QJsonObject &obj, QString *errorOut) const
{
    const QString path = resolveLocalPath(fileUrlOrPath);
    if (path.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("无法写入文件，请检查路径权限。");
        return false;
    }
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorOut) *errorOut = QStringLiteral("无法写入文件，请检查路径权限。");
        return false;
    }
    saveFile.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    if (!saveFile.commit()) {
        if (errorOut) *errorOut = QStringLiteral("无法写入文件，请检查路径权限。");
        return false;
    }
    return true;
}

bool SettingsService::exportLicenseRequestToFile(const QString &fileUrlOrPath)
{
    QString err;
    const auto req = m_files.exportRequest(QStringLiteral("license"), &err);
    if (!req.has_value()) {
        emitError(err);
        return false;
    }
    if (!writeJsonFile(fileUrlOrPath, *req, &err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("导出许可请求"));
    emit stateChanged();
    return true;
}

bool SettingsService::importLicenseFromFile(const QString &fileUrlOrPath)
{
    QString err;
    const auto obj = readJsonFile(fileUrlOrPath, &err);
    if (!obj.has_value()) {
        emitError(err);
        return false;
    }
    if (!m_files.apply(QStringLiteral("license"), *obj, &err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("工作站许可已启用"));
    refreshAccessAndClearTransients();
    return true;
}

bool SettingsService::clearLicense()
{
    if (!canManage()) {
        emitError(QStringLiteral("需要账号管理权限"));
        return false;
    }
    QString err;
    if (!m_files.clearLicense(&err)) {
        emitError(err);
        return false;
    }
    emitError(QStringLiteral("许可已失效"));
    refreshAccessAndClearTransients();
    return true;
}

bool SettingsService::importLicenseRequestForIssue(const QString &fileUrlOrPath)
{
    if (!canIssue()) {
        emitError(QStringLiteral("需要调试员权限"));
        return false;
    }
    m_issuedLicenseReq.reset();
    m_issuedLicenseError.clear();
    QString err;
    const auto obj = readJsonFile(fileUrlOrPath, &err);
    if (!obj.has_value()) {
        m_issuedLicenseError = m_i18n.tr(err);
        emit stateChanged();
        return false;
    }
    const auto checked = m_files.checkRequest(QStringLiteral("license"), *obj, &err);
    if (!checked.has_value()) {
        m_issuedLicenseError = m_i18n.tr(err);
        emit stateChanged();
        return false;
    }
    m_issuedLicenseReq = *checked;
    emit stateChanged();
    return true;
}

bool SettingsService::exportIssuedLicenseToFile(const QString &fileUrlOrPath,
                                                 const QString &mode,
                                                 int years,
                                                 const QString &notAfter)
{
    if (!canIssue() || !m_issuedLicenseReq.has_value()) {
        emitError(QStringLiteral("需要调试员权限"));
        return false;
    }
    QString err;
    const auto resp = m_files.issue(QStringLiteral("license"), *m_issuedLicenseReq, mode, years, notAfter, &err);
    if (!resp.has_value()) {
        m_issuedLicenseError = m_i18n.tr(err);
        emitError(err);
        emit stateChanged();
        return false;
    }
    if (!writeJsonFile(fileUrlOrPath, *resp, &err)) {
        emitError(err);
        return false;
    }
    m_issuedLicenseError.clear();
    emitSuccess(QStringLiteral("签发许可文件"));
    emit stateChanged();
    return true;
}

bool SettingsService::exportRecoveryRequestToFile(const QString &fileUrlOrPath)
{
    QString err;
    const auto req = m_files.exportRequest(QStringLiteral("recovery"), &err);
    if (!req.has_value()) {
        emitError(err);
        return false;
    }
    if (!writeJsonFile(fileUrlOrPath, *req, &err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("导出账号恢复请求"));
    emit stateChanged();
    return true;
}

bool SettingsService::importRecoveryFromFile(const QString &fileUrlOrPath)
{
    QString err;
    const auto obj = readJsonFile(fileUrlOrPath, &err);
    if (!obj.has_value()) {
        emitError(err);
        return false;
    }
    if (!m_files.apply(QStringLiteral("recovery"), *obj, &err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("恢复已完成，账号可重新登录"));
    emit stateChanged();
    return true;
}

bool SettingsService::importRecoveryRequestForIssue(const QString &fileUrlOrPath)
{
    if (!canIssue()) {
        emitError(QStringLiteral("需要调试员权限"));
        return false;
    }
    m_issuedRecoveryReq.reset();
    m_issuedRecoveryError.clear();
    QString err;
    const auto obj = readJsonFile(fileUrlOrPath, &err);
    if (!obj.has_value()) {
        m_issuedRecoveryError = m_i18n.tr(err);
        emit stateChanged();
        return false;
    }
    const auto checked = m_files.checkRequest(QStringLiteral("recovery"), *obj, &err);
    if (!checked.has_value()) {
        m_issuedRecoveryError = m_i18n.tr(err);
        emit stateChanged();
        return false;
    }
    m_issuedRecoveryReq = *checked;
    emit stateChanged();
    return true;
}

bool SettingsService::exportIssuedRecoveryToFile(const QString &fileUrlOrPath)
{
    if (!canIssue() || !m_issuedRecoveryReq.has_value()) {
        emitError(QStringLiteral("需要调试员权限"));
        return false;
    }
    QString err;
    const auto resp = m_files.issue(QStringLiteral("recovery"), *m_issuedRecoveryReq, QStringLiteral("years"), 1, QString(), &err);
    if (!resp.has_value()) {
        m_issuedRecoveryError = m_i18n.tr(err);
        emitError(err);
        emit stateChanged();
        return false;
    }
    if (!writeJsonFile(fileUrlOrPath, *resp, &err)) {
        emitError(err);
        return false;
    }
    m_issuedRecoveryError.clear();
    emitSuccess(QStringLiteral("签发恢复文件"));
    emit stateChanged();
    return true;
}

bool SettingsService::exportSettingsToFile(const QString &fileUrlOrPath)
{
    if (!canManage()) {
        emitError(QStringLiteral("需要账号管理权限"));
        return false;
    }
    QString err;
    const QJsonArray accounts = m_accounts.backup(&err);
    if (!err.isEmpty()) {
        emitError(err);
        return false;
    }
    const QJsonObject payload{
        {QStringLiteral("type"), QStringLiteral("firesystem.prototype.settings")},
        {QStringLiteral("version"), 1},
        {QStringLiteral("appearance"), QJsonObject{
            {QStringLiteral("lang"), m_i18n.lang()},
            {QStringLiteral("theme"), m_theme},
        }},
        {QStringLiteral("accounts"), accounts},
    };
    if (!writeJsonFile(fileUrlOrPath, payload, &err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("已导出设置"));
    return true;
}

bool SettingsService::prepareSettingsImportFromFile(const QString &fileUrlOrPath)
{
    if (!canManage()) {
        emitError(QStringLiteral("需要账号管理权限"));
        return false;
    }
    m_pendingSettings.reset();
    QString err;
    const auto data = readJsonFile(fileUrlOrPath, &err);
    if (!data.has_value()) {
        emitError(err);
        emit stateChanged();
        return false;
    }

    const QString type = data->value(QStringLiteral("type")).toString();
    const int version = data->value(QStringLiteral("version")).toInt(-1);
    const bool legacy = (type == QLatin1String("firesystem.prototype.backup") && version == 2);

    PendingSettingsImport pending;
    if (!legacy) {
        if (type != QLatin1String("firesystem.prototype.settings") || version != 1 || !data->value(QStringLiteral("appearance")).isObject()) {
            emitError(QStringLiteral("请选择有效的设置文件。"));
            emit stateChanged();
            return false;
        }
        const QJsonObject appearance = data->value(QStringLiteral("appearance")).toObject();
        const QString importTheme = appearance.value(QStringLiteral("theme")).toString();
        const QString importLang = appearance.value(QStringLiteral("lang")).toString();
        if (!PreferencesManager::isSupportedTheme(importTheme) || !I18nCatalog::isSupportedLanguage(importLang)) {
            emitError(QStringLiteral("请选择有效的设置文件。"));
            emit stateChanged();
            return false;
        }
        pending.hasAppearance = true;
        pending.theme = importTheme;
        pending.lang = importLang;
    }

    if (!data->value(QStringLiteral("accounts")).isArray()) {
        emitError(QStringLiteral("账号备份无效或含受保护账号"));
        emit stateChanged();
        return false;
    }
    const QJsonArray accounts = data->value(QStringLiteral("accounts")).toArray();
    if (!AccountManager::validateBackup(accounts, &err)) {
        emitError(err);
        emit stateChanged();
        return false;
    }
    pending.accounts = accounts;
    m_pendingSettings = pending;
    emit stateChanged();
    return true;
}

bool SettingsService::confirmSettingsImport()
{
    if (!m_pendingSettings.has_value() || !canManage()) {
        return false;
    }
    const PendingSettingsImport imported = *m_pendingSettings;
    m_pendingSettings.reset();

    QString err;
    if (!m_accounts.restore(imported.accounts, &err)) {
        m_pendingSettings = imported;
        emitError(err);
        emit stateChanged();
        return false;
    }

    bool prefsSaved = true;
    if (imported.hasAppearance) {
        prefsSaved = m_prefs.save({imported.lang, imported.theme}, &err);
        if (prefsSaved) {
            m_i18n.setLang(imported.lang);
            m_theme = imported.theme;
        }
    }

    m_accounts.logout();
    refreshAccessAndClearTransients();
    m_currentPage = QStringLiteral("login");
    emit stateChanged();

    if (prefsSaved) {
        emitSuccess(QStringLiteral("设置已导入"));
    } else {
        emitError(QStringLiteral("账号已导入，但通用设置保存失败。"));
    }
    return true;
}

void SettingsService::cancelSettingsImport()
{
    if (!m_pendingSettings.has_value()) return;
    m_pendingSettings.reset();
    emit stateChanged();
}

bool SettingsService::recordTemplateReview()
{
    if (!canManage()) {
        emitError(QStringLiteral("需要账号管理权限"));
        return false;
    }
    QString err;
    if (!m_templates.recordReview(&err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("已记录模板复核"));
    emit stateChanged();
    return true;
}

bool SettingsService::exportTemplateReviewToFile(const QString &fileUrlOrPath)
{
    if (!unlocked()) {
        emitError(QStringLiteral("请先登录"));
        return false;
    }
    QString err;
    if (!writeJsonFile(fileUrlOrPath, m_templates.manifestJson(), &err)) {
        emitError(err);
        return false;
    }
    emitSuccess(QStringLiteral("已导出评审清单"));
    return true;
}

bool SettingsService::setLanguage(const QString &lang)
{
    if (!I18nCatalog::isSupportedLanguage(lang)) {
        emitError(QStringLiteral("无法保存通用设置。"));
        return false;
    }
    QString err;
    if (!m_prefs.save({lang, m_theme}, &err)) {
        emitError(err);
        return false;
    }
    m_i18n.setLang(lang);
    emit stateChanged();
    return true;
}

bool SettingsService::setTheme(const QString &theme)
{
    if (!PreferencesManager::isSupportedTheme(theme)) {
        emitError(QStringLiteral("无法保存通用设置。"));
        return false;
    }
    QString err;
    if (!m_prefs.save({m_i18n.lang(), theme}, &err)) {
        emitError(err);
        return false;
    }
    m_theme = theme;
    emit stateChanged();
    return true;
}

QString SettingsService::suggestedExportFileName(const QString &kind) const
{
    if (kind == QLatin1String("license-request")) {
        const QString id = licenseRequestId();
        return id.isEmpty()
            ? QStringLiteral("firesystem-license-request.json")
            : QStringLiteral("firesystem-license-request-%1.json").arg(id.left(8));
    }
    if (kind == QLatin1String("license")) {
        const QString id = m_issuedLicenseReq.has_value()
            ? m_issuedLicenseReq->value(QStringLiteral("id")).toString()
            : QString();
        return id.isEmpty()
            ? QStringLiteral("firesystem-license.json")
            : QStringLiteral("firesystem-license-%1.json").arg(id.left(8));
    }
    if (kind == QLatin1String("recovery-request")) {
        const QString id = recoveryRequestId();
        return id.isEmpty()
            ? QStringLiteral("firesystem-recovery-request.json")
            : QStringLiteral("firesystem-recovery-request-%1.json").arg(id.left(8));
    }
    if (kind == QLatin1String("recovery")) {
        const QString id = m_issuedRecoveryReq.has_value()
            ? m_issuedRecoveryReq->value(QStringLiteral("id")).toString()
            : QString();
        return id.isEmpty()
            ? QStringLiteral("firesystem-recovery.json")
            : QStringLiteral("firesystem-recovery-%1.json").arg(id.left(8));
    }
    if (kind == QLatin1String("settings")) {
        return QStringLiteral("firesystem-settings.json");
    }
    if (kind == QLatin1String("template-review")) {
        return QStringLiteral("template-review.json");
    }
    return QStringLiteral("export.json");
}

QUrl SettingsService::suggestedExportFileUrl(const QString &fileName) const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (dir.isEmpty()) {
        dir = QDir::homePath();
    }
    const QString cleanName = QFileInfo(fileName).fileName();
    return QUrl::fromLocalFile(QDir(dir).filePath(cleanName.isEmpty() ? QStringLiteral("export.json") : cleanName));
}

int SettingsService::maxUtcYearOffset() const
{
    return qMax(1, 9999 - QDateTime::currentDateTimeUtc().date().year());
}

QString SettingsService::todayUtcIso() const
{
    return QDateTime::currentDateTimeUtc().date().toString(Qt::ISODate);
}

} // namespace FireSystem
