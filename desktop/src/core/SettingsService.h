#pragma once

#include "AccountManager.h"
#include "I18nCatalog.h"
#include "OfflineFileManager.h"
#include "PreferencesManager.h"
#include "TemplateReviewManager.h"

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <optional>

namespace FireSystem {

/**
 * @brief QML-facing facade for FireSystem Desktop Settings ("设置") module.
 *
 * NOTE (Unconfirmed / "无法确认"):
 * Real hardware binding, board management API, and production trust root are
 * unconfirmed ("无法确认"). This service operates strictly on local simulation
 * files (`SIM-ROOT-01`, `demo: true`, `deployable: false`) in `storageDir`.
 */
class SettingsService : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject *i18n READ i18n CONSTANT)
    Q_PROPERTY(QString currentPage READ currentPage NOTIFY stateChanged)
    Q_PROPERTY(bool signedIn READ signedIn NOTIFY stateChanged)
    Q_PROPERTY(bool licensed READ licensed NOTIFY stateChanged)
    Q_PROPERTY(bool unlocked READ unlocked NOTIFY stateChanged)
    Q_PROPERTY(bool canManage READ canManage NOTIFY stateChanged)
    Q_PROPERTY(bool canIssue READ canIssue NOTIFY stateChanged)
    Q_PROPERTY(bool canRecover READ canRecover NOTIFY stateChanged)

    Q_PROPERTY(QString currentAccountId READ currentAccountId NOTIFY stateChanged)
    Q_PROPERTY(QString currentAccountRole READ currentAccountRole NOTIFY stateChanged)
    Q_PROPERTY(QString computerName READ computerName NOTIFY stateChanged)
    Q_PROPERTY(QString licenseNotAfter READ licenseNotAfter NOTIFY stateChanged)
    Q_PROPERTY(QString licenseRequestId READ licenseRequestId NOTIFY stateChanged)
    Q_PROPERTY(QString recoveryRequestId READ recoveryRequestId NOTIFY stateChanged)
    Q_PROPERTY(int accountGeneration READ accountGeneration NOTIFY stateChanged)

    Q_PROPERTY(QString lang READ lang NOTIFY stateChanged)
    Q_PROPERTY(QString theme READ theme NOTIFY stateChanged)

    Q_PROPERTY(QString accountSearchQuery READ accountSearchQuery WRITE setAccountSearchQuery NOTIFY stateChanged)
    Q_PROPERTY(QVariantList accountsModel READ accountsModel NOTIFY stateChanged)

    Q_PROPERTY(QString templateId READ templateId CONSTANT)
    Q_PROPERTY(QString templateBoardId READ templateBoardId CONSTANT)
    Q_PROPERTY(QString templateVersion READ templateVersion CONSTANT)
    Q_PROPERTY(QString templateReviewStatus READ templateReviewStatus NOTIFY stateChanged)

    Q_PROPERTY(bool hasPendingSettingsImport READ hasPendingSettingsImport NOTIFY stateChanged)
    Q_PROPERTY(bool pendingImportHasAppearance READ pendingImportHasAppearance NOTIFY stateChanged)
    Q_PROPERTY(int pendingImportAccountCount READ pendingImportAccountCount NOTIFY stateChanged)

    Q_PROPERTY(QVariantMap issuedLicenseRequest READ issuedLicenseRequest NOTIFY stateChanged)
    Q_PROPERTY(QString issuedLicenseError READ issuedLicenseError NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap issuedRecoveryRequest READ issuedRecoveryRequest NOTIFY stateChanged)
    Q_PROPERTY(QString issuedRecoveryError READ issuedRecoveryError NOTIFY stateChanged)

    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)

public:
    static constexpr qint64 kMaxJsonImportBytes = 256 * 1024;

    explicit SettingsService(const QString &storageDir,
                             int pbkdf2Rounds = AccountManager::kDefaultPbkdf2Rounds,
                             QObject *parent = nullptr);

    QObject *i18n() { return &m_i18n; }
    I18nCatalog *i18nCatalog() { return &m_i18n; }
    AccountManager *accountManager() { return &m_accounts; }
    OfflineFileManager *offlineFileManager() { return &m_files; }
    PreferencesManager *preferencesManager() { return &m_prefs; }
    TemplateReviewManager *templateReviewManager() { return &m_templates; }

    QString currentPage() const { return m_currentPage; }
    bool signedIn() const;
    bool licensed() const;
    bool unlocked() const;
    bool canManage() const;
    bool canIssue() const;
    bool canRecover() const;

    QString currentAccountId() const;
    QString currentAccountRole() const;
    QString computerName() const;
    QString licenseNotAfter() const;
    QString licenseRequestId() const;
    QString recoveryRequestId() const;
    int accountGeneration() const;

    QString lang() const;
    QString theme() const;

    QString accountSearchQuery() const { return m_accountSearchQuery; }
    void setAccountSearchQuery(const QString &query);
    QVariantList accountsModel() const;

    QString templateId() const;
    QString templateBoardId() const;
    QString templateVersion() const;
    QString templateReviewStatus() const;

    bool hasPendingSettingsImport() const { return m_pendingSettings.has_value(); }
    bool pendingImportHasAppearance() const;
    int pendingImportAccountCount() const;

    QVariantMap issuedLicenseRequest() const;
    QString issuedLicenseError() const { return m_issuedLicenseError; }
    QVariantMap issuedRecoveryRequest() const;
    QString issuedRecoveryError() const { return m_issuedRecoveryError; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool pageAvailable(const QString &page) const;
    Q_INVOKABLE QString accessLanding() const;
    Q_INVOKABLE void navigateTo(const QString &page);

    Q_INVOKABLE QString signIn(const QString &id, const QString &password);
    Q_INVOKABLE void signOut();

    Q_INVOKABLE bool saveAccount(const QString &originalId,
                                 const QString &id,
                                 const QString &role,
                                 bool enabled,
                                 bool hasPassword,
                                 const QString &password);
    Q_INVOKABLE bool changeOwnPassword(const QString &oldPassword,
                                       const QString &newPassword,
                                       const QString &confirmPassword);
    Q_INVOKABLE bool deleteAccount(const QString &id);

    Q_INVOKABLE bool exportLicenseRequestToFile(const QString &fileUrlOrPath);
    Q_INVOKABLE bool importLicenseFromFile(const QString &fileUrlOrPath);
    Q_INVOKABLE bool clearLicense();
    Q_INVOKABLE bool importLicenseRequestForIssue(const QString &fileUrlOrPath);
    Q_INVOKABLE bool exportIssuedLicenseToFile(const QString &fileUrlOrPath,
                                               const QString &mode,
                                               int years,
                                               const QString &notAfter);

    Q_INVOKABLE bool exportRecoveryRequestToFile(const QString &fileUrlOrPath);
    Q_INVOKABLE bool importRecoveryFromFile(const QString &fileUrlOrPath);
    Q_INVOKABLE bool importRecoveryRequestForIssue(const QString &fileUrlOrPath);
    Q_INVOKABLE bool exportIssuedRecoveryToFile(const QString &fileUrlOrPath);

    Q_INVOKABLE bool exportSettingsToFile(const QString &fileUrlOrPath);
    Q_INVOKABLE bool prepareSettingsImportFromFile(const QString &fileUrlOrPath);
    Q_INVOKABLE bool confirmSettingsImport();
    Q_INVOKABLE void cancelSettingsImport();

    Q_INVOKABLE bool recordTemplateReview();
    Q_INVOKABLE bool exportTemplateReviewToFile(const QString &fileUrlOrPath);

    Q_INVOKABLE bool setLanguage(const QString &lang);
    Q_INVOKABLE bool setTheme(const QString &theme);

    Q_INVOKABLE QString suggestedExportFileName(const QString &kind) const;
    Q_INVOKABLE int maxUtcYearOffset() const;
    Q_INVOKABLE QString todayUtcIso() const;

    static QString resolveLocalPath(const QString &fileUrlOrPath);

signals:
    void stateChanged();
    void toastRequested(const QString &message, const QString &kind);

private:
    struct PendingSettingsImport {
        QJsonArray accounts;
        bool hasAppearance = false;
        QString lang;
        QString theme;
    };

    static bool isKnownSettingsPage(const QString &page);
    void refreshAccessAndClearTransients();
    void emitError(const QString &zhError);
    void emitSuccess(const QString &zhMsg);
    std::optional<QJsonObject> readJsonFile(const QString &fileUrlOrPath, QString *errorOut) const;
    bool writeJsonFile(const QString &fileUrlOrPath, const QJsonObject &obj, QString *errorOut) const;

    QString m_storageDir;
    I18nCatalog m_i18n;
    AccountManager m_accounts;
    OfflineFileManager m_files;
    PreferencesManager m_prefs;
    TemplateReviewManager m_templates;

    QString m_currentPage;
    QString m_theme = QStringLiteral("system");
    QString m_accountSearchQuery;
    std::optional<PendingSettingsImport> m_pendingSettings;
    std::optional<QJsonObject> m_issuedLicenseReq;
    QString m_issuedLicenseError;
    std::optional<QJsonObject> m_issuedRecoveryReq;
    QString m_issuedRecoveryError;
    QString m_lastError;
};

} // namespace FireSystem
