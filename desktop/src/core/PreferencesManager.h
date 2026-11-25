#pragma once

#include <QString>

namespace FireSystem {

struct PreferencesData {
    QString lang = QStringLiteral("zh-CN");
    QString theme = QStringLiteral("system");
};

class PreferencesManager
{
public:
    explicit PreferencesManager(QString storageDir);

    QString storageFilePath() const;
    PreferencesData load() const;
    bool save(const PreferencesData &prefs, QString *errorOut = nullptr) const;

    static bool isSupportedTheme(const QString &theme);

private:
    QString m_storageDir;
};

} // namespace FireSystem
