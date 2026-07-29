#include <QGuiApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJSEngine>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>
#include <QtQml/qqml.h>

#include <algorithm>
#include <cstdlib>

#include "runtime/AuroraStateMapper.h"
#include "runtime/AuroraTypes.h"
#include "runtime/audio/AudioRuntime.h"
#include "runtime/artwork/ArtworkIllustrationService.h"
#include "runtime/memory/MomentService.h"
#include "runtime/library/LocalLibraryService.h"
#include "runtime/lyrics/LyricsService.h"
#include "runtime/sources/MusicSourceRegistry.h"

namespace {
bool ensureDemoAudioFile(const QString &path, int durationSeconds = 1)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    constexpr quint16 channelCount = 1;
    constexpr quint32 sampleRate = 8000;
    constexpr quint16 bitsPerSample = 16;
    const quint32 sampleCount =
        sampleRate * static_cast<quint32>(std::max(1, durationSeconds));
    constexpr quint16 blockAlign = channelCount * bitsPerSample / 8;
    constexpr quint32 byteRate = sampleRate * blockAlign;
    const quint32 dataSize = sampleCount * blockAlign;

    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32(36 + dataSize);
    stream.writeRawData("WAVE", 4);
    stream.writeRawData("fmt ", 4);
    stream << quint32(16);
    stream << quint16(1);
    stream << channelCount;
    stream << sampleRate;
    stream << byteRate;
    stream << blockAlign;
    stream << bitsPerSample;
    stream.writeRawData("data", 4);
    stream << dataSize;

    return file.write(header) == header.size()
        && file.write(QByteArray(dataSize, '\0')) == dataSize;
}

QQuickItem *findAppShell(QQuickWindow *window)
{
    if (!window || !window->contentItem())
        return nullptr;

    const QList<QQuickItem *> children = window->contentItem()->childItems();
    for (QQuickItem *child : children) {
        if (child && child->property("currentPage").isValid())
            return child;
    }
    return nullptr;
}

QQuickItem *findItemWithProperty(QQuickItem *item, const char *propertyName)
{
    if (!item)
        return nullptr;

    if (item->property(propertyName).isValid())
        return item;

    const QList<QQuickItem *> children = item->childItems();
    for (QQuickItem *child : children) {
        if (QQuickItem *match = findItemWithProperty(child, propertyName))
            return match;
    }
    return nullptr;
}
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const QString outputPath = app.arguments().value(1);
    if (outputPath.isEmpty())
        return EXIT_FAILURE;
    const QString mode = app.arguments().value(2);

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

    QTimer::singleShot(800, &app, [&app, &engine, outputPath, mode, &audioRuntime]() {
        QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (!window)
            QCoreApplication::exit(EXIT_FAILURE);

        if (mode == QStringLiteral("music-presence")) {
            const QString demoPath =
                QDir::temp().filePath(QStringLiteral("aurora-snapshot-demo.wav"));
            if (!ensureDemoAudioFile(demoPath))
                QCoreApplication::exit(EXIT_FAILURE);

            QVariantList urls;
            urls.append(QUrl::fromLocalFile(demoPath));
            audioRuntime.setQueue(urls);

            if (QQuickItem *shell = findAppShell(window))
                shell->setProperty("currentPage", 1);
        }

        if (mode == QStringLiteral("music-network-transition")
                || mode == QStringLiteral("music-network-soft-reveal")
                || mode == QStringLiteral("music-network-reveal")) {
            const QString firstPath =
                QDir::temp().filePath(QStringLiteral("aurora-network-first.wav"));
            const QString secondPath =
                QDir::temp().filePath(QStringLiteral("aurora-network-second.wav"));
            if (!ensureDemoAudioFile(firstPath, 16)
                    || !ensureDemoAudioFile(secondPath, 16)) {
                QCoreApplication::exit(EXIT_FAILURE);
            }

            QVariantMap firstTrack;
            firstTrack.insert(QStringLiteral("url"), QUrl::fromLocalFile(firstPath));
            firstTrack.insert(QStringLiteral("title"), QStringLiteral("aurora-network-first"));
            firstTrack.insert(QStringLiteral("artist"), QStringLiteral("Network Demo"));
            firstTrack.insert(QStringLiteral("album"), QStringLiteral("Transition"));
            firstTrack.insert(
                QStringLiteral("artworkUrl"),
                QStringLiteral("qrc:/qt/qml/Aurora/App/assets/demo-cover-a.png"));

            QVariantMap secondTrack;
            secondTrack.insert(QStringLiteral("url"), QUrl::fromLocalFile(secondPath));
            secondTrack.insert(QStringLiteral("title"), QStringLiteral("aurora-network-second"));
            secondTrack.insert(QStringLiteral("artist"), QStringLiteral("Network Demo"));
            secondTrack.insert(QStringLiteral("album"), QStringLiteral("Transition"));
            secondTrack.insert(
                QStringLiteral("artworkUrl"),
                QStringLiteral("qrc:/qt/qml/Aurora/App/assets/demo-cover-b.png"));

            audioRuntime.setQueueWithMetadata({firstTrack, secondTrack});

            if (QQuickItem *shell = findAppShell(window))
                shell->setProperty("currentPage", 1);
            if (QQuickItem *musicSpace = findItemWithProperty(
                    window->contentItem(), "networkTransitionEnabled")) {
                const bool invoked = QMetaObject::invokeMethod(
                    musicSpace,
                    "beginTrackTransition",
                    Q_ARG(QVariant, QVariant(1)));
                if (!invoked)
                    QCoreApplication::exit(EXIT_FAILURE);
            }
        }

        if (mode == QStringLiteral("music-source-panel")) {
            if (QQuickItem *shell = findAppShell(window))
                shell->setProperty("currentPage", 1);
            if (QQuickItem *musicSpace = findItemWithProperty(
                    window->contentItem(), "sourcePanelExpanded")) {
                musicSpace->setProperty("sourcePanelExpanded", true);
            }
        }

        if (mode == QStringLiteral("music-online-tracks")) {
            if (QQuickItem *shell = findAppShell(window))
                shell->setProperty("currentPage", 1);
            if (QQuickItem *musicSpace = findItemWithProperty(
                    window->contentItem(), "tracksPanelExpanded")) {
                musicSpace->setProperty("tracksPanelExpanded", true);
                musicSpace->setProperty("sourceTracksMode", true);
            }
        }

        if (mode == QStringLiteral("home-online-current")) {
            const QString demoPath =
                QDir::temp().filePath(QStringLiteral("aurora-snapshot-online.wav"));
            if (!ensureDemoAudioFile(demoPath))
                QCoreApplication::exit(EXIT_FAILURE);

            QVariantMap track;
            track.insert(QStringLiteral("url"), QUrl::fromLocalFile(demoPath));
            track.insert(QStringLiteral("title"), QStringLiteral("線上曲目測試"));
            track.insert(QStringLiteral("artist"), QStringLiteral("Source Catalog"));
            track.insert(QStringLiteral("album"), QStringLiteral("Online Demo"));
            audioRuntime.setQueueWithMetadata({track});

            if (QQuickItem *shell = findAppShell(window))
                shell->setProperty("currentPage", 0);
        }

        if (mode == QStringLiteral("home-online-kept-moment")) {
            QVariantMap track;
            track.insert(QStringLiteral("url"), QStringLiteral("https://cdn.example.com/audio/hoshi.mp3?token=demo"));
            track.insert(QStringLiteral("title"), QStringLiteral("星光測試"));
            track.insert(QStringLiteral("artist"), QStringLiteral("Source Catalog"));
            track.insert(QStringLiteral("album"), QStringLiteral("Online Demo"));
            audioRuntime.setQueueWithMetadata({track});

            if (QQuickItem *musicSpace = findItemWithProperty(
                    window->contentItem(), "sourcePanelExpanded")) {
                QMetaObject::invokeMethod(musicSpace, "keepCurrentMoment");
            }

            if (QQuickItem *shell = findAppShell(window))
                shell->setProperty("currentPage", 0);
        }

        const int settleDelay =
            mode == QStringLiteral("music-presence") ? 4700
            : mode == QStringLiteral("music-network-transition") ? 1900
            : mode == QStringLiteral("music-network-soft-reveal") ? 7200
            : mode == QStringLiteral("music-network-reveal") ? 16600
            : 900;
        QTimer::singleShot(settleDelay, &app, [&app, window, outputPath]() {
            const QImage image = window->grabWindow();
            if (image.isNull() || !image.save(outputPath))
                QCoreApplication::exit(EXIT_FAILURE);

            QCoreApplication::exit(EXIT_SUCCESS);
        });
    });

    return app.exec();
}
