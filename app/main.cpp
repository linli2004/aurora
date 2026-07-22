#include <QGuiApplication>
#include <cstdlib>
#include <QQmlApplicationEngine>
#include <QJSEngine>
#include <QtQml/qqml.h>

#include "runtime/AuroraStateMapper.h"
#include "runtime/AuroraTypes.h"
#include "runtime/audio/AudioRuntime.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Aurora"));
    QCoreApplication::setApplicationName(QStringLiteral("Aurora Music Framework"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.3.0"));

    qmlRegisterUncreatableMetaObject(
        Aurora::staticMetaObject,
        "Aurora.Runtime",
        1,
        0,
        "AuroraTypes",
        QStringLiteral("AuroraTypes contains enums only"));

    qmlRegisterSingletonType<AuroraStateMapper>(
        "Aurora.Runtime",
        1,
        0,
        "AuroraStateMapper",
        [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new AuroraStateMapper;
        });

    qmlRegisterSingletonType<AudioRuntime>(
        "Aurora.Runtime",
        1,
        0,
        "AudioRuntime",
        [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new AudioRuntime;
        });

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);

    engine.loadFromModule("Aurora.App", "Main");
    return app.exec();
}
