#pragma once

#include <QJsonObject>
#include <QString>

namespace FireSystem {

class TemplateReviewManager
{
public:
    static constexpr const char *kDefaultTemplateId = "sim.baseline_ring";
    static constexpr const char *kDefaultBoardId = "kzb_f407";
    static constexpr const char *kDefaultVersion = "1.2.1";

    explicit TemplateReviewManager(QString storageDir);

    QString storageFilePath() const;
    QString templateId() const { return QString::fromLatin1(kDefaultTemplateId); }
    QString boardId() const { return QString::fromLatin1(kDefaultBoardId); }
    QString version() const { return QString::fromLatin1(kDefaultVersion); }

    QString reviewStatus() const;
    bool recordReview(QString *errorOut = nullptr);
    QJsonObject manifestJson() const;

private:
    QJsonObject readDb() const;

    QString m_storageDir;
};

} // namespace FireSystem
