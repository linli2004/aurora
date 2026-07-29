#include <QGuiApplication>
#include <cstdlib>
#include <QQmlApplicationEngine>
#include <QJSEngine>
#include <QtQml/qqml.h>

#include "runtime/AuroraStateMapper.h"
#include "runtime/AuroraTypes.h"
#include "runtime/audio/AudioRuntime.h"
#include "runtime/artwork/ArtworkIllustrationService.h"
#include "runtime/library/LocalLibraryService.h"
#include "runtime/lyrics/LyricsService.h"
#include "runtime/memory/MomentService.h"
#include "runtime/platform/MprisService.h"
#include "runtime/sources/MusicSourceRegistry.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Aurora"));
    QCoreApplication::setApplicationName(QStringLiteral("Aurora Music Framework"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Aurora"));
    QGuiApplication::setDesktopFileName(QStringLiteral("aurora"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.5.3"));

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

    AudioRuntime audioRuntime;
    MprisService mprisService(&audioRuntime);
    Q_UNUSED(mprisService);
    qmlRegisterSingletonInstance(
        "Aurora.Runtime", 1, 0, "AudioRuntime", &audioRuntime);
    LyricsService lyricsService;
    qmlRegisterSingletonInstance(
        "Aurora.Runtime", 1, 0, "Lyrics", &lyricsService);
    ArtworkIllustrationService artworkIllustrationService;
    qmlRegisterSingletonInstance(
        "Aurora.Runtime", 1, 0, "ArtworkIllustration",
        &artworkIllustrationService);
    MusicSourceRegistry musicSourceRegistry;
    qmlRegisterSingletonInstance(
        "Aurora.Runtime", 1, 0, "MusicSources", &musicSourceRegistry);

    qmlRegisterSingletonType<LocalLibraryService>(
        "Aurora.Runtime",
        1,
        0,
        "LocalLibrary",
        [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new LocalLibraryService;
        });

    qmlRegisterSingletonType<MomentService>(
        "Aurora.Runtime",
        1,
        0,
        "Moments",
        [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new MomentService;
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
