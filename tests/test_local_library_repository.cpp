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
    QCOMPARE(repository.playableFilePaths().size(), 2);
}

QTEST_GUILESS_MAIN(LocalLibraryRepositoryTest)
#include "test_local_library_repository.moc"
