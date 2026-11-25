#include "AccountManager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QVariantMap>

namespace FireSystem {

namespace {

bool isValidAccountId(const QString &id)
{
    static const QRegularExpression re(QStringLiteral("^[a-zA-Z0-9._-]{1,32}$"));
    return re.match(id).hasMatch() && id.toLower() != QLatin1String(AccountManager::kDebugId);
}

AccountPublicRow rowFromJson(const QJsonObject &obj)
{
    AccountPublicRow row;
    row.id = obj.value(QStringLiteral("id")).toString();
    row.role = obj.value(QStringLiteral("role")).toString();
    row.enabled = obj.value(QStringLiteral("enabled")).toBool(false);
    row.pending = obj.value(QStringLiteral("pending")).toBool(false);
    return row;
}

} // namespace

QVariantMap AccountPublicRow::toVariantMap() const
{
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("role"), role},
        {QStringLiteral("enabled"), enabled},
        {QStringLiteral("pending"), pending},
    };
}

AccountManager::AccountManager(QString storageDir, int pbkdf2Rounds)
    : m_storageDir(std::move(storageDir))
    , m_pbkdf2Rounds(pbkdf2Rounds > 0 ? pbkdf2Rounds : kDefaultPbkdf2Rounds)
{
}

QString AccountManager::storageFilePath() const
{
    return QDir(m_storageDir).filePath(QStringLiteral("accounts.json"));
}

bool AccountManager::can(const QString &role, const QString &capability)
{
    if (capability == QLatin1String("monitor")) {
        return role == QLatin1String("operator")
            || role == QLatin1String("admin")
            || role == QLatin1String("debugger");
    }
    if (capability == QLatin1String("ack") || capability == QLatin1String("control")) {
        return role == QLatin1String("operator") || role == QLatin1String("debugger");
    }
    if (capability == QLatin1String("accounts") || capability == QLatin1String("export")) {
        return role == QLatin1String("admin") || role == QLatin1String("debugger");
    }
    if (capability == QLatin1String("diagnose")
        || capability == QLatin1String("edit")
        || capability == QLatin1String("deploy")
        || capability == QLatin1String("maint")
        || capability == QLatin1String("update")
        || capability == QLatin1String("issueFiles")) {
        return role == QLatin1String("debugger");
    }
    return false;
}

bool AccountManager::isManagedRole(const QString &role)
{
    return role == QLatin1String("operator") || role == QLatin1String("admin");
}

bool AccountManager::isValidProfile(const QString &id, const QString &role)
{
    return isValidAccountId(id) && isManagedRole(role);
}

bool AccountManager::isValidHex(const QString &value, int length)
{
    if (value.size() != length) {
        return false;
    }
    for (const QChar ch : value) {
        const ushort u = ch.unicode();
        const bool ok = (u >= '0' && u <= '9') || (u >= 'a' && u <= 'f');
        if (!ok) {
            return false;
        }
    }
    return true;
}

QString AccountManager::randomHex16()
{
    QByteArray bytes(16, Qt::Uninitialized);
    auto *rng = QRandomGenerator::system();
    for (int i = 0; i < 16; i += 4) {
        const quint32 val = rng->generate();
        bytes[i] = static_cast<char>((val >> 24) & 0xff);
        bytes[i + 1] = static_cast<char>((val >> 16) & 0xff);
        bytes[i + 2] = static_cast<char>((val >> 8) & 0xff);
        bytes[i + 3] = static_cast<char>(val & 0xff);
    }
    return QString::fromLatin1(bytes.toHex());
}

bool AccountManager::constantTimeEquals(const QByteArray &a, const QByteArray &b)
{
    if (a.size() != b.size()) {
        return false;
    }
    unsigned char diff = 0;
    for (int i = 0; i < a.size(); ++i) {
        diff |= static_cast<unsigned char>(a.at(i) ^ b.at(i));
    }
    return diff == 0;
}

QString AccountManager::derivePasswordHash(const QString &password, const QString &saltHex) const
{
    const QByteArray saltBytes = QByteArray::fromHex(saltHex.toLatin1());
    const QByteArray derived = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        saltBytes,
        m_pbkdf2Rounds,
        32);
    return QString::fromLatin1(derived.toHex());
}

std::optional<AccountManager::Credential> AccountManager::createCredential(
    const QString &password, bool allowEmpty, QString *errorOut) const
{
    if (password.isEmpty() && !allowEmpty) {
        if (errorOut) *errorOut = QStringLiteral("请输入密码");
        return std::nullopt;
    }
    Credential cred;
    cred.salt = randomHex16();
    cred.hash = derivePasswordHash(password, cred.salt);
    return cred;
}

std::optional<QJsonObject> AccountManager::readDatabase(QString *errorOut) const
{
    QFile file(storageFilePath());
    if (!file.exists()) {
        return std::nullopt;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorOut) *errorOut = QStringLiteral("账号库无法读取");
        return std::nullopt;
    }
    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorOut) *errorOut = QStringLiteral("账号库无法读取");
        return std::nullopt;
    }
    const QJsonObject db = doc.object();
    const QJsonObject debugObj = db.value(QStringLiteral("debug")).toObject();
    if (!db.contains(QStringLiteral("debug"))
        || debugObj.value(QStringLiteral("id")).toString() != QLatin1String(kDebugId)
        || debugObj.value(QStringLiteral("role")).toString() != QLatin1String("debugger")
        || !db.value(QStringLiteral("managed")).isArray()) {
        if (errorOut) *errorOut = QStringLiteral("账号库无法读取");
        return std::nullopt;
    }
    return db;
}

bool AccountManager::writeDatabase(QJsonObject nextDb, QString *errorOut) const
{
    const QString path = storageFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QString readErr;
    const auto currentDb = readDatabase(&readErr);
    const int prevGen = currentDb.has_value() ? currentDb->value(QStringLiteral("generation")).toInt(0) : 0;
    nextDb.insert(QStringLiteral("generation"), prevGen + 1);

    QSaveFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorOut) *errorOut = QStringLiteral("账号库无法读取");
        return false;
    }
    saveFile.write(QJsonDocument(nextDb).toJson(QJsonDocument::Indented));
    if (!saveFile.commit()) {
        if (errorOut) *errorOut = QStringLiteral("账号库无法读取");
        return false;
    }
    return true;
}

bool AccountManager::isInitialized() const
{
    QString err;
    return readDatabase(&err).has_value();
}

int AccountManager::generation() const
{
    QString err;
    const auto db = readDatabase(&err);
    return db.has_value() ? db->value(QStringLiteral("generation")).toInt(0) : 0;
}

bool AccountManager::initialize(QString *errorOut)
{
    QString readErr;
    const auto previous = readDatabase(&readErr);
    if (!readErr.isEmpty()) {
        if (errorOut) *errorOut = readErr;
        return false;
    }
    if (previous.has_value() && previous->value(QStringLiteral("presetRevision")).toInt(0) >= kPresetRevision) {
        return true;
    }

    const auto debugCred = createCredential(QStringLiteral("lince"), false, errorOut);
    if (!debugCred.has_value()) {
        return false;
    }
    const auto adminCred = createCredential(QString(), true, errorOut);
    if (!adminCred.has_value()) {
        return false;
    }

    QJsonObject next = previous.value_or(QJsonObject{});
    const QJsonObject debugObj{
        {QStringLiteral("id"), QStringLiteral("debug")},
        {QStringLiteral("role"), QStringLiteral("debugger")},
        {QStringLiteral("enabled"), true},
        {QStringLiteral("token"), randomHex16()},
        {QStringLiteral("salt"), debugCred->salt},
        {QStringLiteral("hash"), debugCred->hash},
    };
    const QJsonObject adminObj{
        {QStringLiteral("id"), QStringLiteral("admin")},
        {QStringLiteral("role"), QStringLiteral("admin")},
        {QStringLiteral("enabled"), true},
        {QStringLiteral("pending"), false},
        {QStringLiteral("token"), randomHex16()},
        {QStringLiteral("salt"), adminCred->salt},
        {QStringLiteral("hash"), adminCred->hash},
    };

    QJsonArray nextManaged;
    nextManaged.append(adminObj);
    const QJsonArray prevManaged = next.value(QStringLiteral("managed")).toArray();
    for (const QJsonValue &v : prevManaged) {
        QJsonObject row = v.toObject();
        if (row.value(QStringLiteral("id")).toString() == QLatin1String("admin")) {
            continue;
        }
        row.remove(QStringLiteral("name"));
        nextManaged.append(row);
    }

    next.insert(QStringLiteral("schemaVersion"), kSchemaVersion);
    next.insert(QStringLiteral("presetRevision"), kPresetRevision);
    next.insert(QStringLiteral("debug"), debugObj);
    next.insert(QStringLiteral("managed"), nextManaged);

    return writeDatabase(next, errorOut);
}

std::optional<QJsonObject> AccountManager::currentRow(const QJsonObject &db) const
{
    if (!m_session.has_value()) {
        return std::nullopt;
    }
    const QJsonObject debugObj = db.value(QStringLiteral("debug")).toObject();
    if (debugObj.value(QStringLiteral("id")).toString() == m_session->id) {
        if (debugObj.value(QStringLiteral("enabled")).toBool(false)
            && debugObj.value(QStringLiteral("token")).toString() == m_session->token) {
            return debugObj;
        }
        return std::nullopt;
    }
    const QJsonArray managed = db.value(QStringLiteral("managed")).toArray();
    for (const QJsonValue &v : managed) {
        const QJsonObject row = v.toObject();
        if (row.value(QStringLiteral("id")).toString() == m_session->id) {
            if (row.value(QStringLiteral("enabled")).toBool(false)
                && row.value(QStringLiteral("token")).toString() == m_session->token) {
                return row;
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<AccountPublicRow> AccountManager::current() const
{
    QString err;
    const auto db = readDatabase(&err);
    if (!db.has_value()) {
        return std::nullopt;
    }
    const auto row = currentRow(*db);
    if (!row.has_value()) {
        return std::nullopt;
    }
    return rowFromJson(*row);
}

bool AccountManager::login(const QString &id, const QString &password, QString *errorOut)
{
    if (!initialize(errorOut)) {
        return false;
    }
    const auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("账号库无法读取");
        return false;
    }

    auto findAccount = [&](const QJsonObject &source) -> std::optional<QJsonObject> {
        const QJsonObject dbg = source.value(QStringLiteral("debug")).toObject();
        if (dbg.value(QStringLiteral("id")).toString() == id
            && dbg.value(QStringLiteral("enabled")).toBool(false)
            && !dbg.value(QStringLiteral("pending")).toBool(false)) {
            return dbg;
        }
        const QJsonArray managed = source.value(QStringLiteral("managed")).toArray();
        for (const QJsonValue &v : managed) {
            const QJsonObject row = v.toObject();
            if (row.value(QStringLiteral("id")).toString() == id
                && row.value(QStringLiteral("enabled")).toBool(false)
                && !row.value(QStringLiteral("pending")).toBool(false)) {
                return row;
            }
        }
        return std::nullopt;
    };

    const auto row = findAccount(*db);
    if (!row.has_value()) {
        if (errorOut) *errorOut = QStringLiteral("账号或密码错误");
        return false;
    }

    const QString salt = row->value(QStringLiteral("salt")).toString();
    const QString expectedHash = row->value(QStringLiteral("hash")).toString();
    const QString actualHash = derivePasswordHash(password, salt);
    if (!constantTimeEquals(actualHash.toLatin1(), expectedHash.toLatin1())) {
        if (errorOut) *errorOut = QStringLiteral("账号或密码错误");
        return false;
    }

    m_session = Session{
        row->value(QStringLiteral("id")).toString(),
        row->value(QStringLiteral("token")).toString()
    };
    return true;
}

void AccountManager::logout()
{
    m_session.reset();
}

QVariantList AccountManager::list(QString *errorOut) const
{
    const auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("账号库无法读取");
        return {};
    }
    const auto actor = currentRow(*db);
    if (!actor.has_value() || !can(actor->value(QStringLiteral("role")).toString(), QStringLiteral("accounts"))) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return {};
    }

    QVariantList rows;
    rows.append(rowFromJson(db->value(QStringLiteral("debug")).toObject()).toVariantMap());
    const QJsonArray managed = db->value(QStringLiteral("managed")).toArray();
    for (const QJsonValue &v : managed) {
        rows.append(rowFromJson(v.toObject()).toVariantMap());
    }
    return rows;
}

bool AccountManager::save(const QString &originalId,
                          const QString &id,
                          const QString &role,
                          bool enabled,
                          bool hasPassword,
                          const QString &password,
                          QString *errorOut)
{
    auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("账号库无法读取");
        return false;
    }
    const auto actor = currentRow(*db);
    if (!actor.has_value() || !can(actor->value(QStringLiteral("role")).toString(), QStringLiteral("accounts"))) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return false;
    }
    if (!isValidProfile(id, role) || (!originalId.isEmpty() && id != originalId)) {
        if (errorOut) *errorOut = QStringLiteral("请检查账号和角色");
        return false;
    }

    QJsonArray managed = db->value(QStringLiteral("managed")).toArray();
    int existingIndex = -1;
    QJsonObject oldRow;
    for (int i = 0; i < managed.size(); ++i) {
        const QJsonObject row = managed.at(i).toObject();
        if (row.value(QStringLiteral("id")).toString() == id) {
            existingIndex = i;
            oldRow = row;
            break;
        }
    }

    if (originalId.isEmpty() && existingIndex >= 0) {
        if (errorOut) *errorOut = QStringLiteral("此账号已存在");
        return false;
    }
    if (!originalId.isEmpty() && existingIndex < 0) {
        if (errorOut) *errorOut = QStringLiteral("此账号已变更");
        return false;
    }

    const QString actorId = actor->value(QStringLiteral("id")).toString();
    const QString actorRole = actor->value(QStringLiteral("role")).toString();
    if (id == actorId && (role != actorRole || !enabled || hasPassword)) {
        if (errorOut) *errorOut = QStringLiteral("请在登录页修改自己的密码");
        return false;
    }

    const bool oldPending = existingIndex >= 0 && oldRow.value(QStringLiteral("pending")).toBool(false);
    std::optional<Credential> nextCred;
    if (hasPassword) {
        nextCred = createCredential(password, !oldPending, errorOut);
        if (!nextCred.has_value()) {
            return false;
        }
    } else if (existingIndex < 0 || oldPending) {
        if (errorOut) *errorOut = QStringLiteral("请为账号设置密码");
        return false;
    }

    QJsonObject nextRow = existingIndex >= 0 ? oldRow : QJsonObject{};
    nextRow.insert(QStringLiteral("id"), id);
    nextRow.insert(QStringLiteral("role"), role);
    nextRow.insert(QStringLiteral("enabled"), enabled);
    nextRow.insert(QStringLiteral("pending"), false);
    if (nextCred.has_value()) {
        nextRow.insert(QStringLiteral("salt"), nextCred->salt);
        nextRow.insert(QStringLiteral("hash"), nextCred->hash);
    }
    const QString token = (existingIndex >= 0 && id == actorId)
        ? actor->value(QStringLiteral("token")).toString()
        : randomHex16();
    nextRow.insert(QStringLiteral("token"), token);

    if (existingIndex >= 0) {
        managed[existingIndex] = nextRow;
    } else {
        managed.append(nextRow);
    }

    QJsonObject nextDb = *db;
    nextDb.insert(QStringLiteral("managed"), managed);
    return writeDatabase(nextDb, errorOut);
}

bool AccountManager::changePassword(const QString &oldPassword,
                                    const QString &newPassword,
                                    QString *errorOut)
{
    auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("当前密码错误");
        return false;
    }
    const auto actor = currentRow(*db);
    if (!actor.has_value()) {
        if (errorOut) *errorOut = QStringLiteral("当前密码错误");
        return false;
    }
    const QString currentHash = derivePasswordHash(oldPassword, actor->value(QStringLiteral("salt")).toString());
    if (!constantTimeEquals(currentHash.toLatin1(), actor->value(QStringLiteral("hash")).toString().toLatin1())) {
        if (errorOut) *errorOut = QStringLiteral("当前密码错误");
        return false;
    }
    const auto nextCred = createCredential(newPassword, false, errorOut);
    if (!nextCred.has_value()) {
        return false;
    }

    const QString nextToken = randomHex16();
    QJsonObject nextActor = *actor;
    nextActor.insert(QStringLiteral("salt"), nextCred->salt);
    nextActor.insert(QStringLiteral("hash"), nextCred->hash);
    nextActor.insert(QStringLiteral("token"), nextToken);

    QJsonObject nextDb = *db;
    if (actor->value(QStringLiteral("role")).toString() == QLatin1String("debugger")) {
        nextDb.insert(QStringLiteral("debug"), nextActor);
    } else {
        QJsonArray managed = nextDb.value(QStringLiteral("managed")).toArray();
        const QString actorId = actor->value(QStringLiteral("id")).toString();
        for (int i = 0; i < managed.size(); ++i) {
            if (managed.at(i).toObject().value(QStringLiteral("id")).toString() == actorId) {
                managed[i] = nextActor;
                break;
            }
        }
        nextDb.insert(QStringLiteral("managed"), managed);
    }

    if (!writeDatabase(nextDb, errorOut)) {
        return false;
    }
    m_session = Session{actor->value(QStringLiteral("id")).toString(), nextToken};
    return true;
}

bool AccountManager::remove(const QString &id, QString *errorOut)
{
    auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("账号库无法读取");
        return false;
    }
    const auto actor = currentRow(*db);
    if (!actor.has_value() || !can(actor->value(QStringLiteral("role")).toString(), QStringLiteral("accounts"))) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return false;
    }
    const QString actorId = actor->value(QStringLiteral("id")).toString();
    const QJsonArray managed = db->value(QStringLiteral("managed")).toArray();
    bool found = false;
    QJsonArray nextManaged;
    for (const QJsonValue &v : managed) {
        const QJsonObject row = v.toObject();
        if (row.value(QStringLiteral("id")).toString() == id) {
            found = true;
            continue;
        }
        nextManaged.append(row);
    }
    if (id == QLatin1String(kDebugId) || id == actorId || !found) {
        if (errorOut) *errorOut = QStringLiteral("此账号无法删除");
        return false;
    }

    QJsonObject nextDb = *db;
    nextDb.insert(QStringLiteral("managed"), nextManaged);
    return writeDatabase(nextDb, errorOut);
}

QJsonArray AccountManager::backup(QString *errorOut) const
{
    const auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("账号库无法读取");
        return {};
    }
    const auto actor = currentRow(*db);
    if (!actor.has_value() || !can(actor->value(QStringLiteral("role")).toString(), QStringLiteral("accounts"))) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return {};
    }
    QJsonArray result;
    const QJsonArray managed = db->value(QStringLiteral("managed")).toArray();
    for (const QJsonValue &v : managed) {
        QJsonObject row = v.toObject();
        row.remove(QStringLiteral("token"));
        result.append(row);
    }
    return result;
}

bool AccountManager::validateBackup(const QJsonArray &rows, QString *errorOut)
{
    QSet<QString> seenIds;
    for (const QJsonValue &v : rows) {
        if (!v.isObject()) {
            if (errorOut) *errorOut = QStringLiteral("账号备份无效或含受保护账号");
            return false;
        }
        const QJsonObject a = v.toObject();
        const QString id = a.value(QStringLiteral("id")).toString();
        const QString role = a.value(QStringLiteral("role")).toString();
        if (!a.value(QStringLiteral("enabled")).isBool()) {
            if (errorOut) *errorOut = QStringLiteral("账号备份无效或含受保护账号");
            return false;
        }
        const bool enabled = a.value(QStringLiteral("enabled")).toBool();
        const bool pending = a.value(QStringLiteral("pending")).toBool(false);
        const QString salt = a.value(QStringLiteral("salt")).toString();
        const QString hash = a.value(QStringLiteral("hash")).toString();
        if (!isValidProfile(id, role)
            || seenIds.contains(id)
            || !isValidHex(salt, 32)
            || !isValidHex(hash, 64)
            || (pending && enabled)) {
            if (errorOut) *errorOut = QStringLiteral("账号备份无效或含受保护账号");
            return false;
        }
        seenIds.insert(id);
    }
    return true;
}

bool AccountManager::restore(const QJsonArray &rows, QString *errorOut)
{
    if (!validateBackup(rows, errorOut)) {
        return false;
    }
    auto db = readDatabase(errorOut);
    if (!db.has_value()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("账号库无法读取");
        return false;
    }
    const auto actor = currentRow(*db);
    if (!actor.has_value() || !can(actor->value(QStringLiteral("role")).toString(), QStringLiteral("accounts"))) {
        if (errorOut) *errorOut = QStringLiteral("需要账号管理权限");
        return false;
    }

    QJsonArray nextManaged;
    for (const QJsonValue &v : rows) {
        const QJsonObject src = v.toObject();
        QJsonObject dst{
            {QStringLiteral("id"), src.value(QStringLiteral("id")).toString()},
            {QStringLiteral("role"), src.value(QStringLiteral("role")).toString()},
            {QStringLiteral("enabled"), src.value(QStringLiteral("enabled")).toBool()},
            {QStringLiteral("pending"), src.value(QStringLiteral("pending")).toBool(false)},
            {QStringLiteral("salt"), src.value(QStringLiteral("salt")).toString()},
            {QStringLiteral("hash"), src.value(QStringLiteral("hash")).toString()},
            {QStringLiteral("token"), randomHex16()},
        };
        nextManaged.append(dst);
    }

    QJsonObject nextDb = *db;
    nextDb.insert(QStringLiteral("managed"), nextManaged);
    return writeDatabase(nextDb, errorOut);
}

bool AccountManager::canRecover() const
{
    QString err;
    const auto db = readDatabase(&err);
    if (!db.has_value()) {
        return false;
    }
    const auto actor = currentRow(*db);
    if (actor.has_value()) {
        return can(actor->value(QStringLiteral("role")).toString(), QStringLiteral("accounts"));
    }
    const QJsonArray managed = db->value(QStringLiteral("managed")).toArray();
    int nonPendingAdmins = 0;
    for (const QJsonValue &v : managed) {
        const QJsonObject row = v.toObject();
        if (row.value(QStringLiteral("role")).toString() == QLatin1String("admin")
            && !row.value(QStringLiteral("pending")).toBool(false)) {
            ++nonPendingAdmins;
            if (row.value(QStringLiteral("enabled")).toBool(false)) {
                return false;
            }
        }
    }
    return nonPendingAdmins > 0;
}

bool AccountManager::recoverAdmin(int expectedGeneration, QString *errorOut)
{
    auto db = readDatabase(errorOut);
    if (!db.has_value() || !canRecover()) {
        if (errorOut && errorOut->isEmpty()) *errorOut = QStringLiteral("需要账号管理权限");
        return false;
    }
    if (db->value(QStringLiteral("generation")).toInt(0) != expectedGeneration) {
        if (errorOut) *errorOut = QStringLiteral("恢复请求已失效");
        return false;
    }
    QJsonArray managed = db->value(QStringLiteral("managed")).toArray();
    bool recovered = false;
    for (int i = 0; i < managed.size(); ++i) {
        QJsonObject row = managed.at(i).toObject();
        if (row.value(QStringLiteral("role")).toString() == QLatin1String("admin")
            && !row.value(QStringLiteral("enabled")).toBool(false)
            && !row.value(QStringLiteral("pending")).toBool(false)) {
            row.insert(QStringLiteral("enabled"), true);
            row.insert(QStringLiteral("token"), randomHex16());
            managed[i] = row;
            recovered = true;
            break;
        }
    }
    if (!recovered) {
        if (errorOut) *errorOut = QStringLiteral("没有可解锁的管理员账号");
        return false;
    }

    QJsonObject nextDb = *db;
    nextDb.insert(QStringLiteral("managed"), managed);
    return writeDatabase(nextDb, errorOut);
}

} // namespace FireSystem
