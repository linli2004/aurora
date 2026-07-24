#include <QtTest/QtTest>

#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QUuid>

#include "runtime/sources/MusicSourceRegistry.h"

class MusicSourceRegistryTest final : public QObject
{
    Q_OBJECT

private slots:
    void splitsSpaceSeparatedScriptUrls();
    void parsesOneLineLxScriptMetadata();
    void importsLocalScriptFileOnce();
    void exposesDefaultOnlineTracks();
    void encodesLxBufferLikeNodeForHeaders();
    void resolvesMockScriptMusicUrl();
    void resolvesMockOnlineTrackMusicUrl();
    void resolvesMockOnlinePlaylistWithMetadata();
    void resolvesConfiguredRealSourceMusicUrl();
    void resolvesConfiguredRealOnlinePlaylist();
};

void MusicSourceRegistryTest::splitsSpaceSeparatedScriptUrls()
{
    const QString text = QStringLiteral(
        "https://raw.githubusercontent.com/pdone/lx-music-source/main/flower/latest.js "
        "https://raw.githubusercontent.com/pdone/lx-music-source/main/huibq/latest.js "
        "https://ghproxy.net/raw.githubusercontent.com/pdone/lx-music-source/main/ikun/latest.js");

    const QStringList entries = MusicSourceRegistry::entriesFromText(text);

    QCOMPARE(entries.size(), 3);
    QVERIFY(entries.at(0).endsWith(QStringLiteral("/flower/latest.js")));
    QVERIFY(entries.at(1).endsWith(QStringLiteral("/huibq/latest.js")));
    QVERIFY(entries.at(2).contains(QStringLiteral("/ikun/latest.js")));
}

void MusicSourceRegistryTest::parsesOneLineLxScriptMetadata()
{
    const QString script = QStringLiteral(
        "/** * @name 野花 * @version 1 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "send(EVENT_NAMES.inited, { sources: {} });");
    const QUrl scriptUrl(
        QStringLiteral("https://raw.githubusercontent.com/pdone/lx-music-source/main/flower/latest.js"));

    const std::optional<MusicSourceRecord> record =
        MusicSourceRegistry::parseScriptMetadata(script, scriptUrl);

    QVERIFY(record.has_value());
    QCOMPARE(record->name, QStringLiteral("野花"));
    QCOMPARE(record->version, QStringLiteral("1"));
    QVERIFY(record->sourceId.startsWith(QStringLiteral("source:lx-script:v1:")));
    QCOMPARE(record->scriptUrl, scriptUrl);
}

void MusicSourceRegistryTest::importsLocalScriptFileOnce()
{
    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceRegistry-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString filePath = directory.filePath(QStringLiteral("latest.js"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(
        "/** * @name 回聲 * @version 2 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "send(EVENT_NAMES.inited, { sources: {} });");
    file.close();

    MusicSourceRegistry registry;
    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);
    QCOMPARE(registry.sourceNames(), QStringList { QStringLiteral("回聲") });

    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);
}

void MusicSourceRegistryTest::exposesDefaultOnlineTracks()
{
    MusicSourceRegistry registry;
    QVERIFY(registry.onlineTrackCount() > 1);

    const QVariantMap firstTrack = registry.onlineTracks().first().toMap();
    QCOMPARE(firstTrack.value(QStringLiteral("source")).toString(), QStringLiteral("wy"));
    QVERIFY(!firstTrack.value(QStringLiteral("songId")).toString().isEmpty());
    QVERIFY(!firstTrack.value(QStringLiteral("title")).toString().isEmpty());
    QVERIFY(firstTrack.value(QStringLiteral("requestText")).toString().startsWith(QStringLiteral("wy:")));
}

void MusicSourceRegistryTest::encodesLxBufferLikeNodeForHeaders()
{
    MusicSourceRegistry registry;

    const QString textBytes = registry.scriptBufferFromBase64(QStringLiteral("Aurora"), QStringLiteral("utf8"));
    QCOMPARE(registry.scriptBufferToStringFromBase64(textBytes, QStringLiteral("hex")),
             QStringLiteral("4175726f7261"));

    const QString invalidHex = QStringLiteral("[\n \"33894312\",\n \"128k\"\n]");
    const QString invalidHexBytes = registry.scriptBufferFromBase64(invalidHex, QStringLiteral("hex"));
    QCOMPARE(registry.scriptBufferToStringFromBase64(invalidHexBytes, QStringLiteral("hex")), QString());
}

void MusicSourceRegistryTest::resolvesMockScriptMusicUrl()
{
    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceResolver-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString filePath = directory.filePath(QStringLiteral("resolver.js"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(
        "/** * @name Resolver * @version 1 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "on(EVENT_NAMES.request, ({ action, source, info }) => { "
        "  const songId = info.musicInfo.hash ?? info.musicInfo.songmid; "
        "  if (action !== 'musicUrl') return Promise.reject(new Error('bad action')); "
        "  if (source !== 'wy' || info.type !== '128k' || songId.length === 0) return Promise.reject(new Error('bad request')); "
        "  return Promise.resolve('https://example.com/demo.mp3'); "
        "}); "
        "send(EVENT_NAMES.inited, { sources: { wy: { name: 'wy', type: 'music', actions: ['musicUrl'], qualitys: ['128k'] } } });");
    file.close();

    MusicSourceRegistry registry;
    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);

    QSignalSpy resolvedSpy(&registry, &MusicSourceRegistry::musicUrlResolved);
    registry.resolveFromText(QStringLiteral("wy:33894312 128k"));

    QVERIFY2(
        resolvedSpy.wait(2000),
        qPrintable(QStringLiteral("error=%1 status=%2 resolving=%3")
                       .arg(registry.errorString(), registry.statusText())
                       .arg(registry.resolving())));
    QCOMPARE(resolvedSpy.takeFirst().at(0).toString(), QStringLiteral("https://example.com/demo.mp3"));
    QVERIFY(!registry.resolving());
}

void MusicSourceRegistryTest::resolvesMockOnlineTrackMusicUrl()
{
    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceOnlineResolver-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString filePath = directory.filePath(QStringLiteral("resolver.js"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(
        "/** * @name Resolver * @version 1 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "on(EVENT_NAMES.request, ({ action, source, info }) => { "
        "  const songId = info.musicInfo.hash ?? info.musicInfo.songmid; "
        "  if (action !== 'musicUrl') return Promise.reject(new Error('bad action')); "
        "  if (source !== 'wy' || info.type !== '128k' || songId.length === 0) return Promise.reject(new Error('bad request')); "
        "  return Promise.resolve('https://example.com/online.mp3'); "
        "}); "
        "send(EVENT_NAMES.inited, { sources: { wy: { name: 'wy', type: 'music', actions: ['musicUrl'], qualitys: ['128k'] } } });");
    file.close();

    MusicSourceRegistry registry;
    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);
    QVERIFY(registry.onlineTrackCount() > 0);

    QSignalSpy resolvedSpy(&registry, &MusicSourceRegistry::musicTracksResolved);
    registry.resolveOnlineTrackAt(0);

    QVERIFY2(
        resolvedSpy.wait(2000),
        qPrintable(QStringLiteral("error=%1 status=%2 resolving=%3")
                       .arg(registry.errorString(), registry.statusText())
                       .arg(registry.resolving())));
    const QVariantList resolvedTracks = resolvedSpy.takeFirst().at(0).toList();
    QCOMPARE(resolvedTracks.size(), 1);
    const QVariantMap resolvedTrack = resolvedTracks.first().toMap();
    const QVariantMap catalogTrack = registry.onlineTracks().first().toMap();
    QCOMPARE(resolvedTrack.value(QStringLiteral("url")).toString(), QStringLiteral("https://example.com/online.mp3"));
    QCOMPARE(resolvedTrack.value(QStringLiteral("title")).toString(), catalogTrack.value(QStringLiteral("title")).toString());
    QCOMPARE(resolvedTrack.value(QStringLiteral("artist")).toString(), catalogTrack.value(QStringLiteral("artist")).toString());
    QVERIFY(!registry.resolving());
}

void MusicSourceRegistryTest::resolvesMockOnlinePlaylistWithMetadata()
{
    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceOnlinePlaylist-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString filePath = directory.filePath(QStringLiteral("resolver.js"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(
        "/** * @name Resolver * @version 1 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "on(EVENT_NAMES.request, ({ action, source, info }) => { "
        "  const songId = info.musicInfo.hash ?? info.musicInfo.songmid; "
        "  if (action !== 'musicUrl') return Promise.reject(new Error('bad action')); "
        "  if (source !== 'wy' || info.type !== '128k') return Promise.reject(new Error('bad request')); "
        "  return Promise.resolve('https://example.com/' + songId + '.mp3'); "
        "}); "
        "send(EVENT_NAMES.inited, { sources: { wy: { name: 'wy', type: 'music', actions: ['musicUrl'], qualitys: ['128k'] } } });");
    file.close();

    MusicSourceRegistry registry;
    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);

    QSignalSpy resolvedSpy(&registry, &MusicSourceRegistry::musicTracksResolved);
    QSignalSpy appendedSpy(&registry, &MusicSourceRegistry::musicTracksAppendResolved);
    registry.resolveOnlineTracksFrom(0);

    QVERIFY2(
        resolvedSpy.wait(5000),
        qPrintable(QStringLiteral("error=%1 status=%2 resolving=%3")
                       .arg(registry.errorString(), registry.statusText())
                       .arg(registry.resolving())));

    const int expectedTrackCount = qMin(24, registry.onlineTrackCount());
    const QVariantList resolvedTracks = resolvedSpy.takeFirst().at(0).toList();
    QCOMPARE(resolvedTracks.size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(appendedSpy.size(), expectedTrackCount - 1, 5000);

    const QVariantMap firstResolvedTrack = resolvedTracks.first().toMap();
    const QVariantMap firstCatalogTrack = registry.onlineTracks().first().toMap();
    QCOMPARE(firstResolvedTrack.value(QStringLiteral("title")).toString(),
             firstCatalogTrack.value(QStringLiteral("title")).toString());
    QCOMPARE(firstResolvedTrack.value(QStringLiteral("artist")).toString(),
             firstCatalogTrack.value(QStringLiteral("artist")).toString());
    QVERIFY(firstResolvedTrack.value(QStringLiteral("url")).toString().startsWith(QStringLiteral("https://example.com/")));
    const QVariantList appendedTracks = appendedSpy.takeFirst().at(0).toList();
    QCOMPARE(appendedTracks.size(), 1);
    QVERIFY(!registry.resolving());
}

void MusicSourceRegistryTest::resolvesConfiguredRealSourceMusicUrl()
{
    const QString scriptPath = QString::fromUtf8(qgetenv("AURORA_REAL_SOURCE_SCRIPT"));
    if (scriptPath.isEmpty())
        QSKIP("Set AURORA_REAL_SOURCE_SCRIPT to run a real LX source resolver.");

    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceRealResolver-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    MusicSourceRegistry registry;
    registry.importFromText(scriptPath);
    if (registry.busy())
        QTRY_VERIFY_WITH_TIMEOUT(!registry.busy(), 20000);

    QVERIFY2(
        registry.sourceCount() > 0,
        qPrintable(QStringLiteral("error=%1 status=%2")
                       .arg(registry.errorString(), registry.statusText())));

    const QString requestText = QString::fromUtf8(qgetenv("AURORA_REAL_SOURCE_REQUEST"))
                                    .simplified()
                                    .isEmpty()
        ? QStringLiteral("wy:33894312 128k")
        : QString::fromUtf8(qgetenv("AURORA_REAL_SOURCE_REQUEST")).simplified();

    QSignalSpy resolvedSpy(&registry, &MusicSourceRegistry::musicUrlResolved);
    registry.resolveFromText(requestText);

    QVERIFY2(
        resolvedSpy.wait(20000),
        qPrintable(QStringLiteral("request=%1 error=%2 status=%3 resolving=%4")
                       .arg(requestText, registry.errorString(), registry.statusText())
                       .arg(registry.resolving())));

    const QString resolvedUrl = resolvedSpy.takeFirst().at(0).toString();
    const QUrl url = QUrl::fromUserInput(resolvedUrl);
    QVERIFY2(
        (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https"))
            && !url.host().isEmpty()
            && !url.path().endsWith(QStringLiteral(".js")),
        qPrintable(resolvedUrl));
}

void MusicSourceRegistryTest::resolvesConfiguredRealOnlinePlaylist()
{
    if (qEnvironmentVariableIsEmpty("AURORA_REAL_ONLINE_PLAYLIST"))
        QSKIP("Set AURORA_REAL_ONLINE_PLAYLIST=1 to run the real online playlist resolver.");

    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceRealOnlinePlaylist-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    MusicSourceRegistry registry;
    QSignalSpy resolvedSpy(&registry, &MusicSourceRegistry::musicTracksResolved);
    registry.resolveOnlineTracksFrom(0);

    QVERIFY2(
        resolvedSpy.wait(15000),
        qPrintable(QStringLiteral("error=%1 status=%2 resolving=%3")
                       .arg(registry.errorString(), registry.statusText())
                       .arg(registry.resolving())));

    const QVariantList resolvedTracks = resolvedSpy.takeFirst().at(0).toList();
    QCOMPARE(resolvedTracks.size(), 1);
    const QUrl url = QUrl::fromUserInput(
        resolvedTracks.first().toMap().value(QStringLiteral("url")).toString());
    QVERIFY2(
        (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https"))
            && !url.host().isEmpty(),
        qPrintable(url.toString()));
}

QTEST_MAIN(MusicSourceRegistryTest)
#include "test_music_source_registry.moc"
