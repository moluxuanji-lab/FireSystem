#include <QApplication>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QtQml/qqmlextensionplugin.h>

#ifdef FLUENTUI_BUILD_STATIC_LIB
Q_IMPORT_QML_PLUGIN(FluentUIPlugin)
#include <FluentUI.h>
#endif
Q_IMPORT_QML_PLUGIN(FireSystem_UIPlugin)

#include "core/SettingsService.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
    }
#endif
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral(APP_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));

    const QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    FireSystem::SettingsService settingsService(storageDir);

    QQmlApplicationEngine engine;
#ifdef FLUENTUI_BUILD_STATIC_LIB
    FluentUI::registerTypes(&engine);
#endif
    engine.setInitialProperties({
        {QStringLiteral("service"), QVariant::fromValue(&settingsService)},
    });
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FireSystem.UI", "Main");

    return app.exec();
}
