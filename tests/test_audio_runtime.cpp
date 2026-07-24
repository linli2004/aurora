#include <QtTest/QtTest>

#include <QTemporaryDir>
#include <QUuid>
#include <QVariantMap>

#include "runtime/audio/AudioRuntime.h"

class AudioRuntimeTest final : public QObject
{
    Q_OBJECT

private slots:
    void setQueueWithMetadataUsesCatalogIdentity();
};

void AudioRuntimeTest::setQueueWithMetadataUsesCatalogIdentity()
{
    QTemporaryDir dataRoot;
    QVERIFY(dataRoot.isValid());
    qputenv("XDG_DATA_HOME", dataRoot.path().toUtf8());

    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("AudioRuntime-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));

    AudioRuntime runtime;

    QVariantMap track;
    track.insert(QStringLiteral("url"), QStringLiteral("https://example.com/music/hoshi.mp3"));
    track.insert(QStringLiteral("title"), QStringLiteral("星の声"));
    track.insert(QStringLiteral("artist"), QStringLiteral("Aurora Demo"));
    track.insert(QStringLiteral("album"), QStringLiteral("Source Catalog"));
    track.insert(QStringLiteral("artworkUrl"), QStringLiteral("https://example.com/artwork/hoshi.jpg"));

    runtime.setQueueWithMetadata(QVariantList { track });

    QCOMPARE(runtime.queueCount(), 1);
    QCOMPARE(runtime.currentIndex(), 0);
    QCOMPARE(runtime.title(), QStringLiteral("星の声"));
    QCOMPARE(runtime.artist(), QStringLiteral("Aurora Demo"));
    QCOMPARE(runtime.album(), QStringLiteral("Source Catalog"));
    QCOMPARE(runtime.artworkSource(), QUrl(QStringLiteral("https://example.com/artwork/hoshi.jpg")));
    QVERIFY(runtime.metadataAvailable());

    QVariantMap appendedTrack;
    appendedTrack.insert(QStringLiteral("url"), QStringLiteral("https://example.com/music/tsuki.mp3"));
    appendedTrack.insert(QStringLiteral("title"), QStringLiteral("月の舟"));
    appendedTrack.insert(QStringLiteral("artist"), QStringLiteral("Aurora Demo"));

    runtime.appendQueueWithMetadata(QVariantList { appendedTrack });

    QCOMPARE(runtime.queueCount(), 2);
    QCOMPARE(runtime.currentIndex(), 0);
    QCOMPARE(runtime.title(), QStringLiteral("星の声"));

    runtime.setQueueFromText(QStringLiteral("https://example.com/music/plain.mp3"));

    QCOMPARE(runtime.queueCount(), 1);
    QCOMPARE(runtime.title(), QStringLiteral("plain"));
    QCOMPARE(runtime.artist(), QStringLiteral("Online source"));
}

QTEST_MAIN(AudioRuntimeTest)
#include "test_audio_runtime.moc"
