#include "PreferencesManager.h"
#include "I18nCatalog.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace FireSystem {

PreferencesManager::PreferencesManager(QString storageDir)
    : m_storageDir(std::move(storageDir))
{
}

QString PreferencesManager::storageFilePath() const
{
    return QDir(m_storageDir).filePath(QStringLiteral("preferences.json"));
}

bool PreferencesManager::isSupportedTheme(const QString &theme)
{
    return theme == QLatin1String("system")
        || theme == QLatin1String("light")
        || theme == QLatin1String("dark");
}

PreferencesData PreferencesManager::load() const
{
    PreferencesData data;
    QFile file(storageFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return data;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return data;
    }
    const QJsonObject obj = doc.object();
    const QString lang = obj.value(QStringLiteral("lang")).toString();
    const QString theme = obj.value(QStringLiteral("theme")).toString();
    if (I18nCatalog::isSupportedLanguage(lang)) {
        data.lang = lang;
    }
    if (isSupportedTheme(theme)) {
        data.theme = theme;
    }
    return data;
}

bool PreferencesManager::save(const PreferencesData &prefs, QString *errorOut) const
{
    if (!I18nCatalog::isSupportedLanguage(prefs.lang) || !isSupportedTheme(prefs.theme)) {
        if (errorOut) *errorOut = QStringLiteral("无法保存通用设置。");
        return false;
    }
    const QString path = storageFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorOut) *errorOut = QStringLiteral("无法保存通用设置。");
        return false;
    }
    const QJsonObject obj{
        {QStringLiteral("lang"), prefs.lang},
        {QStringLiteral("theme"), prefs.theme},
    };
    saveFile.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    if (!saveFile.commit()) {
        if (errorOut) *errorOut = QStringLiteral("无法保存通用设置。");
        return false;
    }
    return true;
}

} // namespace FireSystem
