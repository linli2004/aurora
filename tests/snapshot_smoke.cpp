#include <QGuiApplication>
#include <QImage>
#include <QJSEngine>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QtQml/qqml.h>

#include <cstdlib>

#include "runtime/AuroraStateMapper.h"
#include "runtime/AuroraTypes.h"
#include "runtime/audio/AudioRuntime.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const QString outputPath = app.arguments().value(1);
    if (outputPath.isEmpty())
        return EXIT_FAILURE;

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

    QTimer::singleShot(800, &app, [&app, &engine, outputPath]() {
        QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (!window)
            QCoreApplication::exit(EXIT_FAILURE);

        const QImage image = window->grabWindow();
        if (image.isNull() || !image.save(outputPath))
            QCoreApplication::exit(EXIT_FAILURE);

        QCoreApplication::exit(EXIT_SUCCESS);
    });

    return app.exec();
}
