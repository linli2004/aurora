#include <QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "runtime/library/LocalLibraryRepository.h"

class LocalLibraryRepositoryTest final : public QObject
{
    Q_OBJECT

private slots:
    void createsSchemaAndPersistsTrackSource();
    void duplicateContentCreatesOneTrackAndMultipleSources();
    void marksMissingSourcesUnavailable();
    void movedFileRelinksByStableTrackIdentity();
    void readsTracksWithSearchText();
    void persistsLibraryRoots();
    void persistsSettings();
};

namespace {
LocalLibrarySourceRecord recordForFile(const QString &path)
{
    const QFileInfo fileInfo(path);
    LocalLibrarySourceRecord record;
    record.identity = LocalTrackIdentityResolver::fallbackFor(QUrl::fromLocalFile(path));
    record.fileSize = fileInfo.size();
    record.modifiedTime = fileInfo.lastModified();
    return record;
}
}

void LocalLibraryRepositoryTest::createsSchemaAndPersistsTrackSource()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString audioPath = directory.filePath(QStringLiteral("Aurora Song.flac"));
    QFile audioFile(audioPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.write("repository fixture");
    audioFile.close();

    LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-one"));
    QVERIFY(repository.open(directory.filePath(QStringLiteral("library.sqlite"))));
    QVERIFY2(repository.upsertSource(recordForFile(audioPath)),
             qPrintable(repository.lastError()));

    QCOMPARE(repository.trackCount(), 1);
    QCOMPARE(repository.sourceCount(), 1);
    QCOMPARE(repository.playableFilePaths(), QStringList{QFileInfo(audioPath).canonicalFilePath()});
}

void LocalLibraryRepositoryTest::duplicateContentCreatesOneTrackAndMultipleSources()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString firstPath = directory.filePath(QStringLiteral("a.mp3"));
    const QString secondPath = directory.filePath(QStringLiteral("b.mp3"));

    QFile firstFile(firstPath);
    QVERIFY(firstFile.open(QIODevice::WriteOnly));
    firstFile.write("same audio bytes");
    firstFile.close();
    QVERIFY(QFile::copy(firstPath, secondPath));

    LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-duplicate"));
    QVERIFY(repository.open(directory.filePath(QStringLiteral("library.sqlite"))));
    QVERIFY2(repository.upsertSource(recordForFile(firstPath)),
             qPrintable(repository.lastError()));
    QVERIFY2(repository.upsertSource(recordForFile(secondPath)),
             qPrintable(repository.lastError()));

    QCOMPARE(repository.trackCount(), 1);
    QCOMPARE(repository.sourceCount(), 2);
    QCOMPARE(repository.playableFilePaths().size(), 1);
    QCOMPARE(repository.tracks().size(), 1);
}

void LocalLibraryRepositoryTest::marksMissingSourcesUnavailable()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString audioPath = directory.filePath(QStringLiteral("missing.mp3"));
    QFile audioFile(audioPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.write("missing source bytes");
    audioFile.close();

    LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-missing"));
    QVERIFY(repository.open(directory.filePath(QStringLiteral("library.sqlite"))));
    QVERIFY2(repository.upsertSource(recordForFile(audioPath)),
             qPrintable(repository.lastError()));

    QVERIFY(QFile::remove(audioPath));

    int markedMissing = 0;
    QVERIFY2(repository.reconcileMissingSources(&markedMissing),
             qPrintable(repository.lastError()));
    QCOMPARE(markedMissing, 1);
    QCOMPARE(repository.trackCount(), 0);
    QCOMPARE(repository.sourceCount(), 0);
    QCOMPARE(repository.missingSourceCount(), 1);
    QVERIFY(repository.playableFilePaths().isEmpty());
    QVERIFY(repository.tracks().isEmpty());
}

void LocalLibraryRepositoryTest::movedFileRelinksByStableTrackIdentity()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString oldPath = directory.filePath(QStringLiteral("before.mp3"));
    const QString newPath = directory.filePath(QStringLiteral("after.mp3"));

    QFile audioFile(oldPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.write("stable moved audio bytes");
    audioFile.close();

    LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-moved"));
    QVERIFY(repository.open(directory.filePath(QStringLiteral("library.sqlite"))));

    const LocalLibrarySourceRecord original = recordForFile(oldPath);
    QVERIFY2(repository.upsertSource(original), qPrintable(repository.lastError()));
    QVERIFY(QFile::rename(oldPath, newPath));

    const LocalLibrarySourceRecord moved = recordForFile(newPath);
    QCOMPARE(moved.identity.trackId, original.identity.trackId);
    QVERIFY(moved.identity.sourceId != original.identity.sourceId);
    QVERIFY2(repository.upsertSource(moved), qPrintable(repository.lastError()));

    int markedMissing = 0;
    QVERIFY2(repository.reconcileMissingSources(&markedMissing),
             qPrintable(repository.lastError()));

    QCOMPARE(markedMissing, 1);
    QCOMPARE(repository.trackCount(), 1);
    QCOMPARE(repository.sourceCount(), 1);
    QCOMPARE(repository.missingSourceCount(), 1);
    QCOMPARE(repository.playableFilePaths(),
             QStringList{QFileInfo(newPath).canonicalFilePath()});

    const QList<LocalLibraryTrackRecord> visibleTracks = repository.tracks();
    QCOMPARE(visibleTracks.size(), 1);
    QCOMPARE(visibleTracks.first().trackId, original.identity.trackId);
    QCOMPARE(visibleTracks.first().filePath,
             QFileInfo(newPath).canonicalFilePath());
}

void LocalLibraryRepositoryTest::readsTracksWithSearchText()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString audioPath = directory.filePath(QStringLiteral("Night Signal.ogg"));
    QFile audioFile(audioPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.write("searchable audio bytes");
    audioFile.close();

    LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-search"));
    QVERIFY(repository.open(directory.filePath(QStringLiteral("library.sqlite"))));
    QVERIFY2(repository.upsertSource(recordForFile(audioPath)),
             qPrintable(repository.lastError()));

    const QList<LocalLibraryTrackRecord> allTracks = repository.tracks();
    QCOMPARE(allTracks.size(), 1);
    QCOMPARE(allTracks.first().title, QStringLiteral("night signal"));
    QCOMPARE(allTracks.first().filePath, QFileInfo(audioPath).canonicalFilePath());

    QCOMPARE(repository.tracks(QStringLiteral("night")).size(), 1);
    QCOMPARE(repository.tracks(QStringLiteral("missing")).size(), 0);
}


void LocalLibraryRepositoryTest::persistsLibraryRoots()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString firstRoot = directory.filePath(QStringLiteral("A"));
    const QString secondRoot = directory.filePath(QStringLiteral("B"));
    QVERIFY(QDir().mkpath(firstRoot));
    QVERIFY(QDir().mkpath(secondRoot));

    const QString databasePath = directory.filePath(QStringLiteral("library.sqlite"));
    {
        LocalLibraryRepository repository(
            QStringLiteral("aurora-repo-test-roots-write"));
        QVERIFY(repository.open(databasePath));
        QVERIFY2(
            repository.setLibraryRoots({secondRoot, firstRoot, firstRoot}),
            qPrintable(repository.lastError()));
    }

    LocalLibraryRepository repository(
        QStringLiteral("aurora-repo-test-roots-read"));
    QVERIFY(repository.open(databasePath));

    QStringList expected {
        QFileInfo(firstRoot).canonicalFilePath(),
        QFileInfo(secondRoot).canonicalFilePath(),
    };
    expected.sort(Qt::CaseInsensitive);

    QCOMPARE(repository.libraryRoots(), expected);
}

void LocalLibraryRepositoryTest::persistsSettings()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString databasePath = directory.filePath(QStringLiteral("library.sqlite"));
    {
        LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-settings-write"));
        QVERIFY(repository.open(databasePath));

        QVariantMap value;
        value.insert(QStringLiteral("currentIndex"), 2);
        value.insert(QStringLiteral("volume"), 0.5);
        value.insert(QStringLiteral("positionMs"), 87234);
        QVERIFY2(repository.setSetting(QStringLiteral("playback.session.v1"), value),
                 qPrintable(repository.lastError()));
    }

    LocalLibraryRepository repository(QStringLiteral("aurora-repo-test-settings-read"));
    QVERIFY(repository.open(databasePath));
    const QVariantMap value = repository.setting(QStringLiteral("playback.session.v1"));
    QCOMPARE(value.value(QStringLiteral("currentIndex")).toInt(), 2);
    QCOMPARE(value.value(QStringLiteral("volume")).toReal(), 0.5);
    QCOMPARE(value.value(QStringLiteral("positionMs")).toLongLong(), 87234);
}

QTEST_GUILESS_MAIN(LocalLibraryRepositoryTest)
#include "test_local_library_repository.moc"
