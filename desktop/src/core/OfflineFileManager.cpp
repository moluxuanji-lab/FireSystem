#include "OfflineFileManager.h"
#include "AccountManager.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSysInfo>
#include <QTimeZone>

#include <limits>

namespace FireSystem {

OfflineFileManager::OfflineFileManager(QString storageDir, AccountManager *accounts)
    : m_storageDir(std::move(storageDir))
    , m_accounts(accounts)
{
}

QString OfflineFileManager::storageFilePath() const
{
    return QDir(m_storageDir).filePath(QStringLiteral("offline-files.json"));
}

QString OfflineFileManager::computerName() const
{
    if (!m_computerNameOverride.isEmpty()) {
        return m_computerNameOverride.left(255);
    }
    const QString host = QSysInfo::machineHostName().trimmed();
    if (host.isEmpty()) {
        return QStringLiteral("未获取电脑名称");
    }
    return host.left(255);
}

void OfflineFileManager::setComputerNameOverride(const QString &name)
{
    m_computerNameOverride = name;
}

bool OfflineFileManager::isValidId(const QString &value)
{
    static const QRegularExpression re(QStringLiteral("^[0-9a-f]{32}$"));
    return re.match(value).hasMatch();
}

bool OfflineFileManager::isValidExpiry(const QString &value)
{
    static const QRegularExpression re(QStringLiteral("^\\d{4}-\\d{2}-\\d{2}$"));
    if (!re.match(value).hasMatch()) {
        return false;
    }
    const QDate date = QDate::fromString(value, Qt::ISODate);
    if (!date.isValid()) {
        return false;
    }
    const QDateTime endUtc(date, QTime(23, 59, 59, 999), QTimeZone::utc());
    return endUtc >= QDateTime::currentDateTimeUtc();
}

QString OfflineFileManager::computeLicenseEndDate(const QString &mode,
                                                  int years,
                                                  const QString &notAfter,
                                                  QString *errorOut)
{
    const QDate todayUtc = QDateTime::currentDateTimeUtc().date();
    if (mode == QLatin1String("years")) {
        if (years < 1 || years > 9999 - todayUtc.year()) {
            if (errorOut) *errorOut = QStringLiteral("请输入有效的许可年数。");
            return {};
        }
        const int targetYear = todayUtc.year() + years;
        const int targetMonth = todayUtc.month();
        const int lastDay = QDate(targetYear, targetMonth, 1).daysInMonth();
        const QDate result(targetYear, targetMonth, qMin(lastDay, todayUtc.day()));
        return result.toString(Qt::ISODate);
    }
    if (mode != QLatin1String("date") || !isValidExpiry(notAfter)) {
        if (errorOut) *errorOut = QStringLiteral("许可有效期无效或已到期");
        return {};
    }
    return notAfter;
}

QJsonObject OfflineFileManager::readDb() const
{
    QFile file(storageFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return {{QStringLiteral("license"), QJsonValue::Null}};
    }
    return doc.object();
}

bool OfflineFileManager::writeDb(const QJsonObject &db, QString *errorOut) const
{
    const QString path = storageFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorOut) *errorOut = QStringLiteral("无法写入文件，请检查路径权限。");
        return false;
    }
    saveFile.write(QJsonDocument(db).toJson(QJsonDocument::Indented));
    if (!saveFile.commit()) {
        if (errorOut) *errorOut = QStringLiteral("无法写入文件，请检查路径权限。");
        return false;
    }
    return true;
}

bool OfflineFileManager::canManage() const
{
    if (!m_accounts) return false;
    const auto actor = m_accounts->current();
    return actor.has_value() && AccountManager::can(actor->role, QStringLiteral("accounts"));
}

LicenseStatus OfflineFileManager::licenseStatusFromDb(const QJsonObject &db)
{
    const QJsonValue licVal = db.value(QStringLiteral("license"));
    if (!licVal.isObject()) {
        return {false, QString()};
    }
    const QJsonObject lic = licVal.toObject();
    const QJsonValue idVal = lic.value(QStringLiteral("id"));
    const QJsonValue machineIdVal = lic.value(QStringLiteral("machineId"));
    const QJsonValue notAfterVal = lic.value(QStringLiteral("notAfter"));
    if (!idVal.isString() || !machineIdVal.isString() || !notAfterVal.isString()) {
        return {false, QString()};
    }
    const QString id = idVal.toString();
    const QString machineId = machineIdVal.toString();
    const QString notAfter = notAfterVal.toString();
    if (!isValidId(id) || !isValidId(machineId) || notAfter.isEmpty()) {
        return {false, QString()};
    }
    const QString dbMachineId = db.value(QStringLiteral("machineId")).toString();
    if (!dbMachineId.isEmpty() && machineId != dbMachineId) {
        return {false, QString()};
    }
    if (!isValidExpiry(notAfter)) {
        static const QRegularExpression dateRe(QStringLiteral("^\\d{4}-\\d{2}-\\d{2}$"));
        const bool isIsoDate = dateRe.match(notAfter).hasMatch() && QDate::fromString(notAfter, Qt::ISODate).isValid();
        return {false, isIsoDate ? notAfter : QString()};
    }
    return {true, notAfter};
}

LicenseStatus OfflineFileManager::licenseStatus() const
{
    return licenseStatusFromDb(readDb());
}

bool OfflineFileManager::mayApply(const QString &kind, const QJsonObject &db) const
{
    if (kind == QLatin1String("recovery")) {
        return m_accounts && m_accounts->canRecover();
    }
    return canManage() || !licenseStatusFromDb(db).licensed;
}

std::optional<QJsonObject> OfflineFileManager::localRequestFromDb(const QString &kind, const QJsonObject &db) const
{
    const QString key = kind + QLatin1String("Request");
    const QJsonValue val = db.value(key);
    if (!val.isObject()) {
        return std::nullopt;
    }
    const QJsonObject req = val.toObject();
    const QString machineId = db.value(QStringLiteral("machineId")).toString();
    if (machineId.isEmpty() || req.value(QStringLiteral("machineId")).toString() != machineId) {
        return std::nullopt;
    }
    if (kind == QLatin1String("recovery")) {
        const int gen = req.value(QStringLiteral("generation")).toInt(-1);
        if (!m_accounts || gen != m_accounts->generation()) {
            return std::nullopt;
        }
    }
    return req;
}

std::optional<QJsonObject> OfflineFileManager::localRequest(const QString &kind) const
{
    return localRequestFromDb(kind, readDb());
}

QString OfflineFileManager::expectedType(const QString &kind, bool isRequest, QString *errorOut)
{
    if (kind != QLatin1String("license") && kind != QLatin1String("recovery")) {
        if (errorOut) *errorOut = QStringLiteral("请选择对应的请求文件");
        return {};
    }
    return QStringLiteral("firesystem.prototype.") + kind + (isRequest ? QStringLiteral(".request") : QString());
}

std::optional<QJsonObject> OfflineFileManager::exportRequest(const QString &kind, QString *errorOut)
{
    const QString reqType = expectedType(kind, true, errorOut);
    if (reqType.isEmpty()) {
        return std::nullopt;
    }
    QJsonObject db = readDb();
    if (!mayApply(kind, db)) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return std::nullopt;
    }
    if (const auto existing = localRequestFromDb(kind, db); existing.has_value()) {
        return existing;
    }
    QString machineId = db.value(QStringLiteral("machineId")).toString();
    if (!isValidId(machineId)) {
        machineId = AccountManager::randomHex16();
        db.insert(QStringLiteral("machineId"), machineId);
    }

    QJsonObject request{
        {QStringLiteral("version"), 2},
        {QStringLiteral("type"), reqType},
        {QStringLiteral("id"), AccountManager::randomHex16()},
        {QStringLiteral("machineId"), machineId},
        {QStringLiteral("computerName"), computerName()},
    };
    if (kind == QLatin1String("recovery")) {
        request.insert(QStringLiteral("generation"), m_accounts ? m_accounts->generation() : 0);
    }
    db.insert(kind + QLatin1String("Request"), request);
    if (!writeDb(db, errorOut)) {
        return std::nullopt;
    }
    return request;
}

std::optional<QJsonObject> OfflineFileManager::checkRequest(const QString &kind,
                                                            const QJsonObject &fileObj,
                                                            QString *errorOut) const
{
    const auto actor = m_accounts ? m_accounts->current() : std::nullopt;
    if (!actor.has_value() || !AccountManager::can(actor->role, QStringLiteral("issueFiles"))) {
        if (errorOut) *errorOut = QStringLiteral("需要调试员权限");
        return std::nullopt;
    }
    const QString reqType = expectedType(kind, true, errorOut);
    if (reqType.isEmpty()) {
        return std::nullopt;
    }
    const QString actualType = fileObj.value(QStringLiteral("type")).toString();
    if (actualType != reqType) {
        if (kind == QLatin1String("recovery") && actualType == QLatin1String("firesystem.prototype.request")) {
            if (errorOut) *errorOut = QStringLiteral("请在目标电脑重新导出新版请求文件。");
        } else {
            if (errorOut) *errorOut = (kind == QLatin1String("license"))
                ? QStringLiteral("请选择许可请求文件。")
                : QStringLiteral("请选择账号恢复请求文件。");
        }
        return std::nullopt;
    }
    const QJsonValue verVal = fileObj.value(QStringLiteral("version"));
    if (!verVal.isDouble() || verVal.toInt(-1) != 2) {
        if (errorOut) *errorOut = QStringLiteral("请在目标电脑重新导出新版请求文件。");
        return std::nullopt;
    }
    const QString id = fileObj.value(QStringLiteral("id")).toString();
    const QString machineId = fileObj.value(QStringLiteral("machineId")).toString();
    const QJsonValue nameVal = fileObj.value(QStringLiteral("computerName"));
    const bool validName = nameVal.isString() && nameVal.toString().size() <= 255;
    bool validGen = true;
    int generation = 0;
    if (kind == QLatin1String("recovery")) {
        const QJsonValue genVal = fileObj.value(QStringLiteral("generation"));
        const double d = genVal.toDouble(-1.0);
        if (genVal.isDouble()
            && d >= 1.0
            && d <= static_cast<double>(std::numeric_limits<int>::max())) {
            const int candidate = static_cast<int>(d);
            if (d == static_cast<double>(candidate)) {
                generation = candidate;
                validGen = true;
            } else {
                validGen = false;
            }
        } else {
            validGen = false;
        }
    }
    if (!isValidId(id) || !isValidId(machineId) || !validName || !validGen) {
        if (errorOut) *errorOut = QStringLiteral("请求文件内容不完整或已损坏。");
        return std::nullopt;
    }

    QJsonObject clean{
        {QStringLiteral("version"), 2},
        {QStringLiteral("type"), actualType},
        {QStringLiteral("id"), id},
        {QStringLiteral("machineId"), machineId},
        {QStringLiteral("computerName"), nameVal.toString()},
    };
    if (kind == QLatin1String("recovery")) {
        clean.insert(QStringLiteral("generation"), generation);
    }
    return clean;
}

std::optional<QJsonObject> OfflineFileManager::issue(const QString &kind,
                                                     const QJsonObject &importedRequest,
                                                     const QString &mode,
                                                     int years,
                                                     const QString &notAfter,
                                                     QString *errorOut) const
{
    const auto checked = checkRequest(kind, importedRequest, errorOut);
    if (!checked.has_value()) {
        return std::nullopt;
    }
    const QString respType = expectedType(kind, false, errorOut);
    if (respType.isEmpty()) {
        return std::nullopt;
    }
    QJsonObject response = *checked;
    response.insert(QStringLiteral("type"), respType);
    response.insert(QStringLiteral("demo"), true);
    if (kind == QLatin1String("license")) {
        const QString expiry = computeLicenseEndDate(mode, years, notAfter, errorOut);
        if (expiry.isEmpty()) {
            return std::nullopt;
        }
        response.insert(QStringLiteral("scope"), QStringLiteral("workstation"));
        response.insert(QStringLiteral("notAfter"), expiry);
    }
    return response;
}

bool OfflineFileManager::apply(const QString &kind, const QJsonObject &fileObj, QString *errorOut)
{
    const QString respType = expectedType(kind, false, errorOut);
    if (respType.isEmpty()) {
        return false;
    }
    QJsonObject db = readDb();
    if (!mayApply(kind, db)) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return false;
    }
    if (fileObj.value(QStringLiteral("version")).toInt(-1) != 2
        || fileObj.value(QStringLiteral("type")).toString() != respType
        || fileObj.value(QStringLiteral("demo")).toBool(false) != true) {
        if (errorOut) *errorOut = QStringLiteral("请选择由新版请求签发的对应文件");
        return false;
    }
    const auto req = localRequestFromDb(kind, db);
    if (!req.has_value()
        || fileObj.value(QStringLiteral("machineId")).toString() != db.value(QStringLiteral("machineId")).toString()
        || fileObj.value(QStringLiteral("id")).toString() != req->value(QStringLiteral("id")).toString()) {
        if (errorOut) *errorOut = QStringLiteral("文件与本机当前请求不匹配");
        return false;
    }
    if (kind == QLatin1String("recovery")) {
        const int reqGen = req->value(QStringLiteral("generation")).toInt(-1);
        const int fileGen = fileObj.value(QStringLiteral("generation")).toInt(-2);
        if (fileGen != reqGen) {
            if (errorOut) *errorOut = QStringLiteral("恢复文件与当前请求或账号库代次不匹配");
            return false;
        }
        if (!m_accounts || !m_accounts->recoverAdmin(reqGen, errorOut)) {
            return false;
        }
    } else {
        const QString notAfter = fileObj.value(QStringLiteral("notAfter")).toString();
        if (fileObj.value(QStringLiteral("scope")).toString() != QLatin1String("workstation")
            || !isValidExpiry(notAfter)) {
            if (errorOut) *errorOut = QStringLiteral("许可有效期无效或已到期");
            return false;
        }
        db.insert(QStringLiteral("license"), QJsonObject{
            {QStringLiteral("id"), fileObj.value(QStringLiteral("id")).toString()},
            {QStringLiteral("machineId"), fileObj.value(QStringLiteral("machineId")).toString()},
            {QStringLiteral("notAfter"), notAfter},
        });
    }
    db.remove(kind + QLatin1String("Request"));
    return writeDb(db, errorOut);
}

bool OfflineFileManager::clearLicense(QString *errorOut)
{
    if (!canManage()) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return false;
    }
    QJsonObject db = readDb();
    db.insert(QStringLiteral("license"), QJsonValue::Null);
    db.remove(QStringLiteral("licenseRequest"));
    return writeDb(db, errorOut);
}

} // namespace FireSystem
