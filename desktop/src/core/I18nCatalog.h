#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QHash>
#include <array>

namespace FireSystem {

class I18nCatalog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lang READ lang WRITE setLang NOTIFY langChanged)
    Q_PROPERTY(int revision READ revision NOTIFY langChanged)
    Q_PROPERTY(QVariantList languageOptions READ languageOptions CONSTANT)

public:
    explicit I18nCatalog(QObject *parent = nullptr);

    QString lang() const;
    bool setLang(const QString &lang);
    int revision() const;

    QVariantList languageOptions() const;
    static bool isSupportedLanguage(const QString &lang);
    static QStringList supportedLanguages();

    Q_INVOKABLE QString tr(const QString &zhSource, const QString &arg0 = QString()) const;
    Q_INVOKABLE QString t(const QString &zhSource, int rev, const QString &arg0 = QString()) const;

signals:
    void langChanged();

private:
    QString translateInternal(const QString &zhSource, int langIdx, int depth = 0) const;

    QString m_lang = QStringLiteral("zh-CN");
    int m_revision = 0;
    QHash<QString, std::array<QString, 5>> m_dict;
};

} // namespace FireSystem
