#include <QGuiApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJSEngine>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>
#include <QtQml/qqml.h>

#include <cstdlib>

#include "runtime/AuroraStateMapper.h"
#include "runtime/AuroraTypes.h"
#include "runtime/audio/AudioRuntime.h"
#include "runtime/memory/MomentService.h"
#include "runtime/library/LocalLibraryService.h"

namespace {
bool ensureDemoAudioFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    constexpr quint16 channelCount = 1;
    constexpr quint32 sampleRate = 8000;
    constexpr quint16 bitsPerSample = 16;
    constexpr quint32 sampleCount = sampleRate;
    constexpr quint16 blockAlign = channelCount * bitsPerSample / 8;
    constexpr quint32 byteRate = sampleRate * blockAlign;
    constexpr quint32 dataSize = sampleCount * blockAlign;

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

        const int settleDelay = mode == QStringLiteral("music-presence") ? 4700 : 900;
        QTimer::singleShot(settleDelay, &app, [&app, window, outputPath]() {
            const QImage image = window->grabWindow();
            if (image.isNull() || !image.save(outputPath))
                QCoreApplication::exit(EXIT_FAILURE);

            QCoreApplication::exit(EXIT_SUCCESS);
        });
    });

    return app.exec();
}
