#include <QDir>
#include <QMetaObject>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "runtime/library/LocalLibraryWatcher.h"

class LocalLibraryWatcherTest final : public QObject
{
    Q_OBJECT

private slots:
    void deduplicatesReadableDirectories();
    void coalescesRefreshRequests();
};

void LocalLibraryWatcherTest::deduplicatesReadableDirectories()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString nested =
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("nested"));
    QVERIFY(QDir().mkpath(nested));

    LocalLibraryWatcher watcher;
    watcher.setWatchedDirectories({
        temporaryDirectory.path(),
        QDir(temporaryDirectory.path()).filePath(QStringLiteral(".")),
        nested,
        nested,
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("missing")),
    });

    QCOMPARE(watcher.watchedDirectoryCount(), 2);

    const QStringList watched = watcher.watchedDirectories();
    QVERIFY(watched.contains(QFileInfo(temporaryDirectory.path()).canonicalFilePath()));
    QVERIFY(watched.contains(QFileInfo(nested).canonicalFilePath()));

    watcher.clear();
    QCOMPARE(watcher.watchedDirectoryCount(), 0);
}

void LocalLibraryWatcherTest::coalescesRefreshRequests()
{
    LocalLibraryWatcher watcher;
    watcher.setDebounceInterval(20);

    QSignalSpy spy(&watcher, &LocalLibraryWatcher::refreshRequested);
    QVERIFY(spy.isValid());

    QVERIFY(QMetaObject::invokeMethod(
        &watcher, "scheduleRefresh", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(
        &watcher, "scheduleRefresh", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(
        &watcher, "scheduleRefresh", Qt::DirectConnection));

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 1000);
}

QTEST_GUILESS_MAIN(LocalLibraryWatcherTest)
#include "test_local_library_watcher.moc"
