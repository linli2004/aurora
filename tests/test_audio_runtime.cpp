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
    void queueTrackMetadataExposesSwitchTargetAndCacheKey();
    void deferredQueueStepUpdatesIdentityBeforePlaybackCommit();
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

void AudioRuntimeTest::queueTrackMetadataExposesSwitchTargetAndCacheKey()
{
    QTemporaryDir dataRoot;
    QVERIFY(dataRoot.isValid());
    qputenv("XDG_DATA_HOME", dataRoot.path().toUtf8());

    QTemporaryDir cacheRoot;
    QVERIFY(cacheRoot.isValid());
    qputenv("XDG_CACHE_HOME", cacheRoot.path().toUtf8());

    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("AudioRuntimeQueue-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));

    QVariantMap firstTrack;
    firstTrack.insert(QStringLiteral("url"), QStringLiteral("https://example.com/music/first.mp3"));
    firstTrack.insert(QStringLiteral("source"), QStringLiteral("wy"));
    firstTrack.insert(QStringLiteral("songId"), QStringLiteral("1001"));
    firstTrack.insert(QStringLiteral("title"), QStringLiteral("First"));
    firstTrack.insert(QStringLiteral("artist"), QStringLiteral("Aurora"));

    QVariantMap secondTrack;
    secondTrack.insert(QStringLiteral("url"), QStringLiteral("https://example.com/music/second.mp3"));
    secondTrack.insert(QStringLiteral("source"), QStringLiteral("wy"));
    secondTrack.insert(QStringLiteral("songId"), QStringLiteral("1002"));
    secondTrack.insert(QStringLiteral("title"), QStringLiteral("Second"));
    secondTrack.insert(QStringLiteral("artist"), QStringLiteral("Aurora"));

    AudioRuntime runtime;
    const QVariantList tracks { firstTrack, secondTrack };
    runtime.setQueueWithMetadata(tracks);

    QCOMPARE(runtime.queueCount(), 2);
    QCOMPARE(runtime.wrappedQueueIndex(1), 1);
    QCOMPARE(runtime.wrappedQueueIndex(-1), 1);

    const QVariantMap nextTrack = runtime.queueTrackMetadata(1);
    QCOMPARE(nextTrack.value(QStringLiteral("title")).toString(), QStringLiteral("Second"));
    QCOMPARE(nextTrack.value(QStringLiteral("source")).toString(), QStringLiteral("wy"));
    QCOMPARE(nextTrack.value(QStringLiteral("songId")).toString(), QStringLiteral("1002"));
    QCOMPARE(nextTrack.value(QStringLiteral("cacheCatalogKey")).toString(), QStringLiteral("wy:1002"));

    runtime.reconcileOnlineTrackCache(tracks);
}

void AudioRuntimeTest::deferredQueueStepUpdatesIdentityBeforePlaybackCommit()
{
    QTemporaryDir dataRoot;
    QVERIFY(dataRoot.isValid());
    qputenv("XDG_DATA_HOME", dataRoot.path().toUtf8());

    QTemporaryDir cacheRoot;
    QVERIFY(cacheRoot.isValid());
    qputenv("XDG_CACHE_HOME", cacheRoot.path().toUtf8());

    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("AudioRuntimeDeferred-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));

    QVariantMap firstTrack;
    firstTrack.insert(QStringLiteral("url"), QStringLiteral("https://example.com/music/one.mp3"));
    firstTrack.insert(QStringLiteral("source"), QStringLiteral("wy"));
    firstTrack.insert(QStringLiteral("songId"), QStringLiteral("2001"));
    firstTrack.insert(QStringLiteral("title"), QStringLiteral("One"));

    QVariantMap secondTrack;
    secondTrack.insert(QStringLiteral("url"), QStringLiteral("https://example.com/music/two.mp3"));
    secondTrack.insert(QStringLiteral("source"), QStringLiteral("wy"));
    secondTrack.insert(QStringLiteral("songId"), QStringLiteral("2002"));
    secondTrack.insert(QStringLiteral("title"), QStringLiteral("Two"));

    AudioRuntime runtime;
    runtime.setQueueWithMetadata(QVariantList { firstTrack, secondTrack });
    QCOMPARE(runtime.currentIndex(), 0);
    QCOMPARE(runtime.title(), QStringLiteral("One"));

    runtime.beginDeferredQueueStep(1);
    QCOMPARE(runtime.currentIndex(), 1);
    QCOMPARE(runtime.title(), QStringLiteral("Two"));
    QCOMPARE(runtime.source(), QUrl(QStringLiteral("https://example.com/music/two.mp3")));

    runtime.commitDeferredPlayback(false);
    QCOMPARE(runtime.currentIndex(), 1);
    QCOMPARE(runtime.title(), QStringLiteral("Two"));
}

QTEST_MAIN(AudioRuntimeTest)
#include "test_audio_runtime.moc"
