#include <QtTest>

#include <QDir>
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
    void resolvesEmbeddedMetadataAndArtwork();
    void prefersTrackArtistOverAlbumArtist();
};

void LocalTrackIdentityTest::filenameFallbackPreservesUnicode()
{
    const QUrl source = QUrl::fromLocalFile(QStringLiteral("/tmp/失眠.flac"));
    const LocalTrackIdentity identity = LocalTrackIdentityResolver::fallbackFor(source);

    QCOMPARE(identity.title, QStringLiteral("失眠"));
    QCOMPARE(identity.artist, QStringLiteral("Local audio"));
    QVERIFY(identity.artworkSource.isEmpty());
    QVERIFY(!identity.metadataAvailable);
    QCOMPARE(identity.provenance, QStringLiteral("Filename fallback · Generated identity"));
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
    QCOMPARE(identity.artist, QStringLiteral("Lin, Aurora"));
    QCOMPARE(identity.album, QStringLiteral("Night Rooms"));
    QCOMPARE(identity.trackNumber, 4);
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
