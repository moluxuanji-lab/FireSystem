#include "TemplateReviewManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

namespace FireSystem {

TemplateReviewManager::TemplateReviewManager(QString storageDir)
    : m_storageDir(std::move(storageDir))
{
}

QString TemplateReviewManager::storageFilePath() const
{
    return QDir(m_storageDir).filePath(QStringLiteral("release-reviews.json"));
}

QJsonObject TemplateReviewManager::readDb() const
{
    QFile file(storageFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    return (err.error == QJsonParseError::NoError && doc.isObject()) ? doc.object() : QJsonObject{};
}

QString TemplateReviewManager::reviewStatus() const
{
    const QJsonObject db = readDb();
    const QString status = db.value(templateId()).toString();
    return status == QStringLiteral("已复核") ? status : QStringLiteral("待复核");
}

bool TemplateReviewManager::recordReview(QString *errorOut)
{
    QJsonObject db = readDb();
    db.insert(templateId(), QStringLiteral("已复核"));

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

QJsonObject TemplateReviewManager::manifestJson() const
{
    return {
        {QStringLiteral("demo"), true},
        {QStringLiteral("template"), templateId()},
        {QStringLiteral("boardId"), boardId()},
        {QStringLiteral("version"), version()},
        {QStringLiteral("review"), reviewStatus()},
        {QStringLiteral("deployable"), false},
    };
}

} // namespace FireSystem
