#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>

#ifdef FLUENTUI_BUILD_STATIC_LIB
Q_IMPORT_QML_PLUGIN(FluentUIPlugin)
#include <FluentUI.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
#endif
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral(APP_NAME));
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));

    QQmlApplicationEngine engine;
#ifdef FLUENTUI_BUILD_STATIC_LIB
    FluentUI::registerTypes(&engine);
#endif
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FireSystem.UI", "Main");

    return app.exec();
}
