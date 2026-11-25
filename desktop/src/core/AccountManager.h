#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <optional>

namespace FireSystem {

struct AccountPublicRow {
    QString id;
    QString role;
    bool enabled = false;
    bool pending = false;

    bool isValid() const { return !id.isEmpty(); }
    QVariantMap toVariantMap() const;
};

class AccountManager
{
public:
    static constexpr int kSchemaVersion = 3;
    static constexpr int kPresetRevision = 1;
    static constexpr int kDefaultPbkdf2Rounds = 600000;
    static constexpr const char *kDebugId = "debug";

    explicit AccountManager(QString storageDir, int pbkdf2Rounds = kDefaultPbkdf2Rounds);

    QString storageFilePath() const;
    int pbkdf2Rounds() const { return m_pbkdf2Rounds; }

    static bool can(const QString &role, const QString &capability);
    static bool isManagedRole(const QString &role);
    static bool isValidProfile(const QString &id, const QString &role);

    bool initialize(QString *errorOut = nullptr);
    bool isInitialized() const;
    int generation() const;

    bool login(const QString &id, const QString &password, QString *errorOut = nullptr);
    void logout();
    std::optional<AccountPublicRow> current() const;

    QVariantList list(QString *errorOut = nullptr) const;
    bool save(const QString &originalId,
              const QString &id,
              const QString &role,
              bool enabled,
              bool hasPassword,
              const QString &password,
              QString *errorOut = nullptr);
    bool changePassword(const QString &oldPassword,
                        const QString &newPassword,
                        QString *errorOut = nullptr);
    bool remove(const QString &id, QString *errorOut = nullptr);

    QJsonArray backup(QString *errorOut = nullptr) const;
    static bool validateBackup(const QJsonArray &rows, QString *errorOut = nullptr);
    bool restore(const QJsonArray &rows, QString *errorOut = nullptr);

    bool canRecover() const;
    bool recoverAdmin(int expectedGeneration, QString *errorOut = nullptr);

    // Exposed for deterministic testing of PBKDF2-HMAC-SHA256
    QString derivePasswordHash(const QString &password, const QString &saltHex) const;
    static QString randomHex16();

private:
    struct Credential {
        QString salt;
        QString hash;
    };

    struct Session {
        QString id;
        QString token;
    };

    std::optional<QJsonObject> readDatabase(QString *errorOut = nullptr) const;
    bool writeDatabase(QJsonObject nextDb, QString *errorOut = nullptr) const;
    std::optional<QJsonObject> currentRow(const QJsonObject &db) const;
    std::optional<Credential> createCredential(const QString &password, bool allowEmpty, QString *errorOut) const;
    static bool constantTimeEquals(const QByteArray &a, const QByteArray &b);
    static bool isValidHex(const QString &value, int length);

    QString m_storageDir;
    int m_pbkdf2Rounds;
    std::optional<Session> m_session;
};

} // namespace FireSystem
