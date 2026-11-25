#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QUrl>
#include <QtQml/qqmlextensionplugin.h>
#include <memory>
#include <vector>

#ifdef FLUENTUI_BUILD_STATIC_LIB
Q_IMPORT_QML_PLUGIN(FluentUIPlugin)
#include <FluentUI.h>
#endif
Q_IMPORT_QML_PLUGIN(FireSystem_UIPlugin)

#include "core/SettingsService.h"

class SettingsTestFactory : public QObject
{
    Q_OBJECT

public:
    explicit SettingsTestFactory(QObject *parent = nullptr)
        : QObject(parent)
    {
        qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    }

    Q_INVOKABLE QObject *createService()
    {
        auto dir = std::make_unique<QTemporaryDir>();
        auto svc = std::make_unique<FireSystem::SettingsService>(dir->path(), 1000, this);
        QObject *ptr = svc.get();
        m_dirs.push_back(std::move(dir));
        m_services.push_back(std::move(svc));
        return ptr;
    }

    Q_INVOKABLE QString newTempFileUrl(const QString &fileName)
    {
        if (m_dirs.empty()) {
            m_dirs.push_back(std::make_unique<QTemporaryDir>());
        }
        return QUrl::fromLocalFile(m_dirs.back()->filePath(fileName)).toString();
    }

    Q_INVOKABLE QVariantMap readJsonFile(const QString &urlOrPath)
    {
        QFile f(FireSystem::SettingsService::resolveLocalPath(urlOrPath));
        if (!f.open(QIODevice::ReadOnly)) return {};
        return QJsonDocument::fromJson(f.readAll()).object().toVariantMap();
    }

private:
    std::vector<std::unique_ptr<QTemporaryDir>> m_dirs;
    std::vector<std::unique_ptr<FireSystem::SettingsService>> m_services;
};

class SettingsQuickTestSetup : public QObject
{
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
#ifdef FLUENTUI_BUILD_STATIC_LIB
        FluentUI::registerTypes(engine);
#endif
        engine->rootContext()->setContextProperty(QStringLiteral("SettingsTestFactory"), &m_factory);
    }

private:
    SettingsTestFactory m_factory;
};

QUICK_TEST_MAIN_WITH_SETUP(tst_settings_ui, SettingsQuickTestSetup)

#include "tst_settings_main.moc"
