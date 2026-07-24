#include <QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMediaMetaData>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "runtime/audio/LocalTrackIdentity.h"

class LocalTrackIdentityTest final : public QObject
{
    Q_OBJECT

private slots:
    void filenameFallbackPreservesUnicode();
    void trackIdSurvivesRenameAndMove();
    void duplicateContentSharesTrackIdButNotSourceId();
    void networkFallbackCreatesUrlIdentity();
    void resolvesEmbeddedMetadataAndArtwork();
    void prefersTrackArtistOverAlbumArtist();
};

void LocalTrackIdentityTest::filenameFallbackPreservesUnicode()
{
    const QUrl source = QUrl::fromLocalFile(QStringLiteral("/tmp/失眠.flac"));
    const LocalTrackIdentity identity = LocalTrackIdentityResolver::fallbackFor(source);

    QVERIFY(identity.trackId.startsWith(QStringLiteral("track:provisional:v1:")));
    QVERIFY(identity.sourceId.startsWith(QStringLiteral("source:local-file:v1:")));
    QCOMPARE(identity.filePath, QStringLiteral("/tmp/失眠.flac"));
    QCOMPARE(identity.canonicalTitle, QStringLiteral("失眠"));
    QCOMPARE(identity.title, QStringLiteral("失眠"));
    QCOMPARE(identity.artist, QStringLiteral("Local audio"));
    QCOMPARE(identity.availability, QStringLiteral("Unavailable"));
    QVERIFY(identity.artworkSource.isEmpty());
    QVERIFY(!identity.metadataAvailable);
    QCOMPARE(identity.provenance, QStringLiteral("Filename fallback · Generated identity"));
}

void LocalTrackIdentityTest::trackIdSurvivesRenameAndMove()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString firstPath = directory.filePath(QStringLiteral("first-name.flac"));
    const QString secondPath = directory.filePath(QStringLiteral("renamed.flac"));

    QFile file(firstPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("stable content fixture");
    file.close();

    const LocalTrackIdentity first = LocalTrackIdentityResolver::fallbackFor(
        QUrl::fromLocalFile(firstPath));

    QVERIFY(QFile::rename(firstPath, secondPath));

    const LocalTrackIdentity second = LocalTrackIdentityResolver::fallbackFor(
        QUrl::fromLocalFile(secondPath));

    QCOMPARE(first.trackId, second.trackId);
    QVERIFY(first.sourceId != second.sourceId);
    QCOMPARE(second.availability, QStringLiteral("Available"));
}

void LocalTrackIdentityTest::duplicateContentSharesTrackIdButNotSourceId()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString firstPath = directory.filePath(QStringLiteral("a.ogg"));
    const QString secondPath = directory.filePath(QStringLiteral("b.ogg"));

    QFile firstFile(firstPath);
    QVERIFY(firstFile.open(QIODevice::WriteOnly));
    firstFile.write("duplicate audio fixture");
    firstFile.close();
    QVERIFY(QFile::copy(firstPath, secondPath));

    const LocalTrackIdentity first = LocalTrackIdentityResolver::fallbackFor(
        QUrl::fromLocalFile(firstPath));
    const LocalTrackIdentity second = LocalTrackIdentityResolver::fallbackFor(
        QUrl::fromLocalFile(secondPath));

    QCOMPARE(first.trackId, second.trackId);
    QVERIFY(first.sourceId != second.sourceId);
}

void LocalTrackIdentityTest::networkFallbackCreatesUrlIdentity()
{
    const QUrl source(QStringLiteral("https://example.com/music/night-signal.mp3?token=demo"));
    const LocalTrackIdentity identity = LocalTrackIdentityResolver::fallbackFor(source);

    QVERIFY(identity.trackId.startsWith(QStringLiteral("track:network-url:v1:")));
    QVERIFY(identity.sourceId.startsWith(QStringLiteral("source:network-url:v1:")));
    QVERIFY(identity.filePath.isEmpty());
    QCOMPARE(identity.title, QStringLiteral("night-signal"));
    QCOMPARE(identity.canonicalTitle, QStringLiteral("night-signal"));
    QCOMPARE(identity.artist, QStringLiteral("Online source"));
    QCOMPARE(identity.availability, QStringLiteral("Available"));
    QVERIFY(!identity.metadataAvailable);
    QCOMPARE(identity.provenance, QStringLiteral("Network source · URL fallback"));
}

void LocalTrackIdentityTest::resolvesEmbeddedMetadataAndArtwork()
{
    QTemporaryDir cacheDirectory;
    QVERIFY(cacheDirectory.isValid());

    QTemporaryFile mediaFile;
    mediaFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/aurora-track-XXXXXX.mp3"));
    QVERIFY(mediaFile.open());
    mediaFile.write("fixture");
    mediaFile.flush();

    QMediaMetaData metaData;
    metaData.insert(QMediaMetaData::Title, QStringLiteral("Midnight Signal"));
    metaData.insert(QMediaMetaData::ContributingArtist,
                    QStringList{QStringLiteral("Lin"), QStringLiteral("Aurora")});
    metaData.insert(QMediaMetaData::AlbumTitle, QStringLiteral("Night Rooms"));
    metaData.insert(QMediaMetaData::TrackNumber, 4);

    QImage cover(80, 80, QImage::Format_ARGB32);
    cover.fill(QColor(QStringLiteral("#D06642")));
    metaData.insert(QMediaMetaData::CoverArtImage, cover);

    const LocalTrackIdentity identity = LocalTrackIdentityResolver::resolve(
        QUrl::fromLocalFile(mediaFile.fileName()),
        metaData,
        cacheDirectory.path());

    QCOMPARE(identity.title, QStringLiteral("Midnight Signal"));
    QCOMPARE(identity.canonicalTitle, QStringLiteral("midnight signal"));
    QCOMPARE(identity.artist, QStringLiteral("Lin, Aurora"));
    QCOMPARE(identity.album, QStringLiteral("Night Rooms"));
    QCOMPARE(identity.trackNumber, 4);
    QCOMPARE(identity.availability, QStringLiteral("Available"));
    QVERIFY(identity.metadataAvailable);
    QVERIFY(identity.hasEmbeddedArtwork);
    QVERIFY(identity.identityColor.isValid());
    QVERIFY(identity.artworkSource.isLocalFile());
    QVERIFY(QFileInfo::exists(identity.artworkSource.toLocalFile()));
    QCOMPARE(identity.provenance, QStringLiteral("Embedded artwork · Local metadata"));
}

void LocalTrackIdentityTest::prefersTrackArtistOverAlbumArtist()
{
    QMediaMetaData metaData;
    metaData.insert(QMediaMetaData::AlbumArtist, QStringLiteral("Compilation Artist"));
    metaData.insert(QMediaMetaData::LeadPerformer,
                    QStringList{QStringLiteral("Track Performer")});

    const LocalTrackIdentity identity = LocalTrackIdentityResolver::resolve(
        QUrl::fromLocalFile(QStringLiteral("/tmp/demo.ogg")),
        metaData,
        QStringLiteral("/tmp/aurora-test-artwork-cache"));

    QCOMPARE(identity.artist, QStringLiteral("Track Performer"));
}

QTEST_APPLESS_MAIN(LocalTrackIdentityTest)
#include "test_local_track_identity.moc"
