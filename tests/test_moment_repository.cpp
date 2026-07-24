#include <QtTest>

#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>

#include "runtime/memory/MomentRepository.h"

class MomentRepositoryTest final : public QObject
{
    Q_OBJECT

private slots:
    void keepsAndRestoresLatestMoment();
    void resolvesRelinkedTrackSource();
    void keepsDetachedMemoryWhenSourceIsMissing();
};

namespace {
MomentRecord sampleMoment(const QString &sourcePath)
{
    MomentRecord moment;
    moment.momentId = QStringLiteral("moment:test");
    moment.trackId = QStringLiteral("track:test");
    moment.sourceId = QStringLiteral("source:test-old");
    moment.sourcePath = sourcePath;
    moment.title = QStringLiteral("Quiet Signals");
    moment.artist = QStringLiteral("Aurora");
    moment.album = QStringLiteral("Night");
    moment.artworkUrl = QStringLiteral("qrc:/artwork.png");
    moment.identityColor = QStringLiteral("#ff7c8fcf");
    moment.periodLabel = QStringLiteral("Late Night");
    moment.createdAt = QDateTime::currentDateTimeUtc();
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
