#pragma once

#include <QJsonObject>
#include <QString>
#include <optional>

namespace FireSystem {

class AccountManager;

struct LicenseStatus {
    bool licensed = false;
    QString notAfter;
};

/**
 * @brief Local workstation demo offline license and admin-recovery file manager.
 *
 * NOTE (Unconfirmed / "无法确认"):
 * Real hardware binding, cryptographic signature verification, and production
 * trust roots are unconfirmed ("无法确认") and not defined in firmware/contracts.
 * This manager strictly implements the local workstation simulation format
 * (`demo: true`, `SIM-ROOT-01`) from the FireSystem prototype.
 */
class OfflineFileManager
{
public:
    explicit OfflineFileManager(QString storageDir, AccountManager *accounts);

    QString storageFilePath() const;
    QString computerName() const;
    void setComputerNameOverride(const QString &name);

    LicenseStatus licenseStatus() const;
    std::optional<QJsonObject> localRequest(const QString &kind) const;

    std::optional<QJsonObject> exportRequest(const QString &kind, QString *errorOut = nullptr);
    std::optional<QJsonObject> checkRequest(const QString &kind,
                                            const QJsonObject &fileObj,
                                            QString *errorOut = nullptr) const;
    std::optional<QJsonObject> issue(const QString &kind,
                                     const QJsonObject &importedRequest,
                                     const QString &mode,
                                     int years,
                                     const QString &notAfter,
                                     QString *errorOut = nullptr) const;
    bool apply(const QString &kind, const QJsonObject &fileObj, QString *errorOut = nullptr);
    bool clearLicense(QString *errorOut = nullptr);

    static bool isValidId(const QString &value);
    static bool isValidExpiry(const QString &value);
    static QString computeLicenseEndDate(const QString &mode,
                                         int years,
                                         const QString &notAfter,
                                         QString *errorOut = nullptr);

private:
    QJsonObject readDb() const;
    bool writeDb(const QJsonObject &db, QString *errorOut = nullptr) const;
    bool canManage() const;
    static LicenseStatus licenseStatusFromDb(const QJsonObject &db);
    bool mayApply(const QString &kind, const QJsonObject &db) const;
    std::optional<QJsonObject> localRequestFromDb(const QString &kind, const QJsonObject &db) const;
    static QString expectedType(const QString &kind, bool isRequest, QString *errorOut = nullptr);

    QString m_storageDir;
    AccountManager *m_accounts;
    QString m_computerNameOverride;
};

} // namespace FireSystem
