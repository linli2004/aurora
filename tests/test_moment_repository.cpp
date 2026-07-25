#include <QtTest>

#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QUrl>

#include "runtime/memory/MomentRepository.h"

class MomentRepositoryTest final : public QObject
{
    Q_OBJECT

private slots:
    void keepsAndRestoresLatestMoment();
    void storesMultipleMomentsNewestFirst();
    void updatesConfirmedMeaningWithoutChangingIdentity();
    void resolvesRelinkedTrackSource();
    void resolvesNetworkMomentUrl();
    void keepsDetachedMemoryWhenSourceIsMissing();
};

namespace {
MomentRecord sampleMoment(
    const QString &sourcePath,
    const QString &momentId = QStringLiteral("moment:test"),
    const QDateTime &createdAt = QDateTime::currentDateTimeUtc())
{
    MomentRecord moment;
    moment.momentId = momentId;
    moment.trackId = QStringLiteral("track:test");
    moment.sourceId = QStringLiteral("source:test-old");
    moment.sourcePath = sourcePath;
    moment.title = QStringLiteral("Quiet Signals");
    moment.artist = QStringLiteral("Aurora");
    moment.album = QStringLiteral("Night");
    moment.artworkUrl = QStringLiteral("qrc:/artwork.png");
    moment.identityColor = QStringLiteral("#ff7c8fcf");
    moment.periodLabel = QStringLiteral("Late Night");
    moment.createdAt = createdAt;
    return moment;
}
}

void MomentRepositoryTest::keepsAndRestoresLatestMoment()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString databasePath =
        directory.filePath(QStringLiteral("library.sqlite"));
    const QString audioPath =
        directory.filePath(QStringLiteral("quiet.mp3"));
    QFile audio(audioPath);
    QVERIFY(audio.open(QIODevice::WriteOnly));
    audio.write("aurora");
    audio.close();

    {
        MomentRepository repository(QStringLiteral("moment-test-write"));
        QVERIFY2(repository.open(databasePath), qPrintable(repository.lastError()));
        QVERIFY2(repository.keepMoment(sampleMoment(audioPath)),
                 qPrintable(repository.lastError()));
        QCOMPARE(repository.count(), 1);
    }

    MomentRepository repository(QStringLiteral("moment-test-read"));
    QVERIFY(repository.open(databasePath));
    const auto latest = repository.latestMoment();
    QVERIFY(latest.has_value());
    QCOMPARE(latest->trackId, QStringLiteral("track:test"));
    QCOMPARE(latest->title, QStringLiteral("Quiet Signals"));
    QCOMPARE(repository.resolvedPlayablePath(*latest),
             QFileInfo(audioPath).canonicalFilePath());
}

void MomentRepositoryTest::storesMultipleMomentsNewestFirst()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    MomentRepository repository(QStringLiteral("moment-test-order"));
    QVERIFY(repository.open(
        directory.filePath(QStringLiteral("library.sqlite"))));

    const QDateTime now = QDateTime::currentDateTimeUtc();
    MomentRecord older = sampleMoment(
        directory.filePath(QStringLiteral("older.mp3")),
        QStringLiteral("moment:older"),
        now.addSecs(-60));
    older.title = QStringLiteral("Older");

    MomentRecord newer = sampleMoment(
        directory.filePath(QStringLiteral("newer.mp3")),
        QStringLiteral("moment:newer"),
        now);
    newer.title = QStringLiteral("Newer");

    QVERIFY(repository.keepMoment(older));
    QVERIFY(repository.keepMoment(newer));

    const QList<MomentRecord> moments = repository.moments();
    QCOMPARE(moments.size(), 2);
    QCOMPARE(moments.at(0).momentId, QStringLiteral("moment:newer"));
    QCOMPARE(moments.at(1).momentId, QStringLiteral("moment:older"));
    QCOMPARE(repository.latestMoment()->momentId,
             QStringLiteral("moment:newer"));
}

void MomentRepositoryTest::updatesConfirmedMeaningWithoutChangingIdentity()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    MomentRepository repository(QStringLiteral("moment-test-meaning"));
    QVERIFY(repository.open(
        directory.filePath(QStringLiteral("library.sqlite"))));

    const MomentRecord original = sampleMoment(
        directory.filePath(QStringLiteral("meaning.mp3")));
    QVERIFY(repository.keepMoment(original));

    QVERIFY(repository.updateConfirmedMeaning(
        original.momentId,
        QStringLiteral("The night finally became quiet.")));

    const auto updated = repository.moment(original.momentId);
    QVERIFY(updated.has_value());
    QCOMPARE(updated->confirmedMeaning,
             QStringLiteral("The night finally became quiet."));
    QCOMPARE(updated->trackId, original.trackId);
    QCOMPARE(updated->sourceId, original.sourceId);
    QCOMPARE(updated->sourcePath, original.sourcePath);

    QVERIFY(repository.updateConfirmedMeaning(original.momentId, QString()));
    const auto cleared = repository.moment(original.momentId);
    QVERIFY(cleared.has_value());
    QCOMPARE(cleared->confirmedMeaning, QStringLiteral(""));
}

void MomentRepositoryTest::resolvesRelinkedTrackSource()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString databasePath =
        directory.filePath(QStringLiteral("library.sqlite"));
    const QString oldPath = directory.filePath(QStringLiteral("old.mp3"));
    const QString newPath = directory.filePath(QStringLiteral("new.mp3"));

    QFile newFile(newPath);
    QVERIFY(newFile.open(QIODevice::WriteOnly));
    newFile.write("same track");
    newFile.close();

    MomentRepository repository(QStringLiteral("moment-test-relink"));
    QVERIFY(repository.open(databasePath));
    QVERIFY(repository.keepMoment(sampleMoment(oldPath)));

    {
        const QString connectionName =
            QStringLiteral("moment-test-relink-sources");
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        database.setDatabaseName(databasePath);
        QVERIFY(database.open());

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "CREATE TABLE track_sources ("
            "source_id TEXT PRIMARY KEY,"
            "track_id TEXT,"
            "file_path TEXT,"
            "availability TEXT,"
            "last_verified_at TEXT,"
            "updated_at TEXT)")));
        query.prepare(QStringLiteral(
            "INSERT INTO track_sources("
            "source_id, track_id, file_path, availability, "
            "last_verified_at, updated_at"
            ") VALUES(?, ?, ?, ?, ?, ?)"));
        query.addBindValue(QStringLiteral("source:test-new"));
        query.addBindValue(QStringLiteral("track:test"));
        query.addBindValue(newPath);
        query.addBindValue(QStringLiteral("Available"));
        query.addBindValue(QStringLiteral("2026-07-24T10:00:00.000Z"));
        query.addBindValue(QStringLiteral("2026-07-24T10:00:00.000Z"));
        QVERIFY(query.exec());

        database.close();
        database = {};
        QSqlDatabase::removeDatabase(connectionName);
    }

    const auto latest = repository.latestMoment();
    QVERIFY(latest.has_value());
    QCOMPARE(repository.resolvedPlayablePath(*latest),
             QFileInfo(newPath).canonicalFilePath());
}

void MomentRepositoryTest::resolvesNetworkMomentUrl()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    MomentRepository repository(QStringLiteral("moment-test-network"));
    QVERIFY(repository.open(
        directory.filePath(QStringLiteral("library.sqlite"))));

    MomentRecord moment = sampleMoment(
        QStringLiteral("https://cdn.example.com/audio/hoshi.mp3?token=demo"),
        QStringLiteral("moment:network"));
    moment.trackId = QStringLiteral("track:network-url:v1:test");
    moment.sourceId = QStringLiteral("source:network-url:v1:test");

    QVERIFY2(repository.keepMoment(moment), qPrintable(repository.lastError()));

    const auto latest = repository.latestMoment();
    QVERIFY(latest.has_value());
    QVERIFY(repository.resolvedPlayablePath(*latest).isEmpty());
    QCOMPARE(repository.resolvedPlayableUrl(*latest),
             QUrl(QStringLiteral("https://cdn.example.com/audio/hoshi.mp3?token=demo")));
}

void MomentRepositoryTest::keepsDetachedMemoryWhenSourceIsMissing()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    MomentRepository repository(QStringLiteral("moment-test-detached"));
    QVERIFY(repository.open(
        directory.filePath(QStringLiteral("library.sqlite"))));
    QVERIFY(repository.keepMoment(
        sampleMoment(directory.filePath(QStringLiteral("missing.mp3")))));

    const auto latest = repository.latestMoment();
    QVERIFY(latest.has_value());
    QVERIFY(repository.resolvedPlayablePath(*latest).isEmpty());
    QCOMPARE(repository.count(), 1);
}

QTEST_GUILESS_MAIN(MomentRepositoryTest)
#include "test_moment_repository.moc"
