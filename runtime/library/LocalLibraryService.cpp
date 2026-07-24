#include "runtime/library/LocalLibraryService.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMetaObject>
#include <QStandardPaths>
#include <QThread>

#include "runtime/library/LocalLibraryRepository.h"

namespace {

QString defaultDatabasePath()
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty())
        dataLocation = QDir::home().filePath(QStringLiteral(".local/share/Aurora"));

    QDir directory;
    directory.mkpath(dataLocation);
    return QDir(dataLocation).filePath(QStringLiteral("library.sqlite"));
}

struct DefaultMusicDirectory
{
    QUrl url;
    QString label;
    bool available = false;
};

DefaultMusicDirectory resolveDefaultMusicDirectory()
{
    QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (musicPath.isEmpty())
        musicPath = QDir::home().filePath(QStringLiteral("Music"));

    const QFileInfo info(musicPath);
    const bool available = info.exists() && info.isDir() && info.isReadable();

    return {
        QUrl::fromLocalFile(info.absoluteFilePath()),
        QDir::toNativeSeparators(info.absoluteFilePath()),
        available
    };
}

bool supportedAudioFile(const QFileInfo &fileInfo)
{
    static const QStringList extensions {
        QStringLiteral("mp3"),
        QStringLiteral("flac"),
        QStringLiteral("ogg"),
        QStringLiteral("opus"),
        QStringLiteral("wav"),
        QStringLiteral("m4a"),
        QStringLiteral("aac"),
    };
    return extensions.contains(fileInfo.suffix().toCaseFolded());
}

QString normalizedRootPath(const QString &path)
{
    const QFileInfo rootInfo(path);
    const QString canonicalPath = rootInfo.canonicalFilePath();
    return canonicalPath.isEmpty()
        ? QDir::cleanPath(rootInfo.absoluteFilePath())
        : QDir::cleanPath(canonicalPath);
}

bool readableDirectory(const QString &path)
{
    const QFileInfo rootInfo(path);
    return rootInfo.exists() && rootInfo.isDir() && rootInfo.isReadable();
}

} // namespace

LocalLibraryService::LocalLibraryService(QObject *parent)
    : QObject(parent)
    , m_databasePath(defaultDatabasePath())
{
    const DefaultMusicDirectory defaultMusic = resolveDefaultMusicDirectory();
    m_defaultMusicDirectory = defaultMusic.url;
    m_defaultMusicDirectoryLabel = defaultMusic.label;
    m_defaultMusicDirectoryAvailable = defaultMusic.available;

    int startupMissingSources = 0;
    {
        LocalLibraryRepository repository;
        if (repository.open(m_databasePath))
            repository.reconcileMissingSources(&startupMissingSources);
    }

    refreshLibraryRoots();
    refreshCounts();
    refreshTracks();

    if (startupMissingSources > 0) {
        setLastScanStatus(
            tr("%n missing local source(s)", nullptr, startupMissingSources));
    } else if (m_unavailableRootCount > 0) {
        setLastScanStatus(
            tr("%n library folder(s) unavailable", nullptr, m_unavailableRootCount));
    }
}

bool LocalLibraryService::scanning() const
{
    return m_scanning;
}

int LocalLibraryService::trackCount() const
{
    return m_trackCount;
}

int LocalLibraryService::sourceCount() const
{
    return m_sourceCount;
}

int LocalLibraryService::missingSourceCount() const
{
    return m_missingSourceCount;
}

int LocalLibraryService::libraryRootCount() const
{
    return m_libraryRoots.size();
}

int LocalLibraryService::unavailableRootCount() const
{
    return m_unavailableRootCount;
}

QString LocalLibraryService::libraryRootsSummary() const
{
    if (m_libraryRoots.isEmpty())
        return tr("No folders remembered");

    QString summary;
    if (m_libraryRoots.size() == 1) {
        QString name = QFileInfo(m_libraryRoots.first()).fileName();
        if (name.isEmpty())
            name = QDir::toNativeSeparators(m_libraryRoots.first());
        summary = tr("1 folder · %1").arg(name);
    } else {
        summary = tr("%1 folders remembered").arg(m_libraryRoots.size());
    }

    if (m_unavailableRootCount > 0) {
        summary += tr(" · %1 unavailable").arg(m_unavailableRootCount);
    }

    return summary;
}

QString LocalLibraryService::databasePath() const
{
    return m_databasePath;
}

QString LocalLibraryService::lastScanStatus() const
{
    return m_lastScanStatus;
}

QString LocalLibraryService::errorString() const
{
    return m_errorString;
}

bool LocalLibraryService::firstRun() const
{
    return !m_scanning
        && m_libraryRoots.isEmpty()
        && m_sourceCount == 0
        && m_trackCount == 0
        && m_missingSourceCount == 0;
}

QUrl LocalLibraryService::defaultMusicDirectory() const
{
    return m_defaultMusicDirectory;
}

QString LocalLibraryService::defaultMusicDirectoryLabel() const
{
    return m_defaultMusicDirectoryLabel;
}

bool LocalLibraryService::defaultMusicDirectoryAvailable() const
{
    return m_defaultMusicDirectoryAvailable;
}

int LocalLibraryService::scannedFileCount() const
{
    return m_scannedFileCount;
}

QString LocalLibraryService::searchText() const
{
    return m_searchText;
}

int LocalLibraryService::visibleTrackCount() const
{
    return m_trackModel.rowCount();
}

QAbstractListModel *LocalLibraryService::tracks()
{
    return &m_trackModel;
}

void LocalLibraryService::scanDefaultMusicDirectory()
{
    if (!m_defaultMusicDirectoryAvailable) {
        setErrorString(
            tr("The default Music folder is not readable. Choose another folder."));
        return;
    }

    scanDirectory(m_defaultMusicDirectory);
}

void LocalLibraryService::scanDirectory(const QUrl &directory)
{
    if (m_scanning)
        return;

    if (!directory.isLocalFile()) {
        setErrorString(tr("Choose a local music folder."));
        return;
    }

    const QFileInfo rootInfo(directory.toLocalFile());
    if (!rootInfo.exists() || !rootInfo.isDir() || !rootInfo.isReadable()) {
        setErrorString(tr("The selected music folder is not readable."));
        return;
    }

    const QString rootPath = normalizedRootPath(rootInfo.absoluteFilePath());
    startScan({rootPath}, rootPath);
}

void LocalLibraryService::rescanLibraryRoots()
{
    if (m_scanning)
        return;

    if (m_libraryRoots.isEmpty()) {
        setErrorString(tr("No library folders have been added."));
        return;
    }

    startScan(m_libraryRoots);
}

void LocalLibraryService::startScan(
    const QStringList &rootPaths,
    const QString &rootToRemember)
{
    if (m_scanning)
        return;

    QStringList normalizedRoots;
    for (const QString &path : rootPaths) {
        const QString normalized = normalizedRootPath(path);
        if (!normalized.isEmpty() && !normalizedRoots.contains(normalized))
            normalizedRoots.append(normalized);
    }
    normalizedRoots.sort(Qt::CaseInsensitive);

    if (normalizedRoots.isEmpty()) {
        setErrorString(tr("No library folders are available to scan."));
        return;
    }

    setErrorString({});
    setScannedFileCount(0);

    if (!rootToRemember.isEmpty()) {
        setLastScanStatus(
            tr("Scanning %1").arg(QDir::toNativeSeparators(rootToRemember)));
    } else {
        setLastScanStatus(
            tr("Rescanning %n library folder(s)", nullptr, normalizedRoots.size()));
    }

    setScanning(true);

    const QString databasePath = m_databasePath;
    const QString rememberedRoot = rootToRemember;
    m_cancelScan = std::make_shared<std::atomic_bool>(false);
    const std::shared_ptr<std::atomic_bool> cancelFlag = m_cancelScan;

    QThread *worker = QThread::create(
        [this, normalizedRoots, rememberedRoot, databasePath, cancelFlag]() {
        LocalLibraryRepository repository;
        int scanned = 0;
        int missingSources = 0;
        int unavailableRoots = 0;
        bool cancelled = false;
        QString error;

        if (!repository.open(databasePath)) {
            error = repository.lastError();
        } else {
            for (const QString &rootPath : normalizedRoots) {
                if (cancelFlag->load()) {
                    cancelled = true;
                    break;
                }

                if (!readableDirectory(rootPath)) {
                    ++unavailableRoots;
                    continue;
                }

                QDirIterator iterator(
                    rootPath,
                    QDir::Files | QDir::Readable | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

                while (iterator.hasNext()) {
                    if (cancelFlag->load()) {
                        cancelled = true;
                        break;
                    }

                    const QString filePath = iterator.next();
                    const QFileInfo fileInfo(filePath);
                    if (!supportedAudioFile(fileInfo))
                        continue;

                    LocalLibrarySourceRecord record;
                    record.identity = LocalTrackIdentityResolver::fallbackFor(
                        QUrl::fromLocalFile(filePath));
                    record.fileSize = fileInfo.size();
                    record.modifiedTime = fileInfo.lastModified();

                    if (!repository.upsertSource(record)) {
                        error = repository.lastError();
                        break;
                    }

                    ++scanned;
                    if (scanned == 1 || scanned % 25 == 0) {
                        QMetaObject::invokeMethod(
                            this,
                            [this, scanned]() {
                                setScannedFileCount(scanned);
                            },
                            Qt::QueuedConnection);
                    }
                }

                if (!error.isEmpty() || cancelled)
                    break;
            }

            if (error.isEmpty() && !cancelled) {
                if (!rememberedRoot.isEmpty()) {
                    QStringList roots = repository.libraryRoots();
                    if (!roots.contains(rememberedRoot))
                        roots.append(rememberedRoot);

                    if (!repository.setLibraryRoots(roots))
                        error = repository.lastError();
                }

                if (error.isEmpty()
                    && !repository.reconcileMissingSources(&missingSources)) {
                    error = repository.lastError();
                }
            }
        }

        QMetaObject::invokeMethod(
            this,
            [this, scanned, cancelled, error, missingSources, unavailableRoots]() {
                if (!error.isEmpty())
                    setErrorString(error);

                setScannedFileCount(scanned);
                refreshLibraryRoots();
                refreshCounts();
                refreshTracks();

                QString completionStatus;
                if (!error.isEmpty()) {
                    completionStatus = tr("Scan failed");
                } else if (cancelled) {
                    completionStatus =
                        tr("Scan cancelled after %1 local audio files").arg(scanned);
                } else if (unavailableRoots > 0 && missingSources > 0) {
                    completionStatus =
                        tr("Scanned %1 local audio files · %2 folder(s) unavailable"
                           " · %3 missing source(s)")
                            .arg(scanned)
                            .arg(unavailableRoots)
                            .arg(missingSources);
                } else if (unavailableRoots > 0) {
                    completionStatus =
                        tr("Scanned %1 local audio files · %2 folder(s) unavailable")
                            .arg(scanned)
                            .arg(unavailableRoots);
                } else if (missingSources > 0) {
                    completionStatus =
                        tr("Scanned %1 local audio files · %2 missing source(s)")
                            .arg(scanned)
                            .arg(missingSources);
                } else {
                    completionStatus =
                        tr("Scanned %1 local audio files").arg(scanned);
                }

                setLastScanStatus(completionStatus);
                setScanning(false);
                m_cancelScan.reset();
            },
            Qt::QueuedConnection);
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void LocalLibraryService::cancelScan()
{
    if (!m_scanning || !m_cancelScan)
        return;

    m_cancelScan->store(true);
    setLastScanStatus(tr("Cancelling scan"));
}

QVariantList LocalLibraryService::playableUrls() const
{
    LocalLibraryRepository repository;
    QVariantList urls;
    if (!repository.open(m_databasePath))
        return urls;

    const QStringList paths = repository.playableFilePaths();
    urls.reserve(paths.size());
    for (const QString &path : paths)
        urls.append(QUrl::fromLocalFile(path));
    return urls;
}

QVariantList LocalLibraryService::visiblePlayableUrls() const
{
    return m_trackModel.urls();
}

QVariantList LocalLibraryService::visiblePlayableUrlsStartingAt(int row) const
{
    return m_trackModel.urlsStartingAt(row);
}

void LocalLibraryService::setSearchText(const QString &searchText)
{
    const QString normalized = searchText.trimmed();
    if (m_searchText == normalized)
        return;

    m_searchText = normalized;
    emit searchChanged();
    refreshTracks();
}

void LocalLibraryService::refreshCounts()
{
    LocalLibraryRepository repository;
    if (!repository.open(m_databasePath)) {
        m_trackCount = 0;
        m_sourceCount = 0;
        m_missingSourceCount = 0;
        emit libraryChanged();
        return;
    }

    const int tracks = repository.trackCount();
    const int sources = repository.sourceCount();
    const int missingSources = repository.missingSourceCount();
    if (m_trackCount == tracks
        && m_sourceCount == sources
        && m_missingSourceCount == missingSources) {
        return;
    }

    m_trackCount = tracks;
    m_sourceCount = sources;
    m_missingSourceCount = missingSources;
    emit libraryChanged();
}

void LocalLibraryService::refreshLibraryRoots()
{
    LocalLibraryRepository repository;

    QStringList roots;
    int unavailableRoots = 0;
    if (repository.open(m_databasePath)) {
        roots = repository.libraryRoots();
        for (const QString &path : roots) {
            if (!readableDirectory(path))
                ++unavailableRoots;
        }
    }

    if (m_libraryRoots == roots
        && m_unavailableRootCount == unavailableRoots) {
        return;
    }

    m_libraryRoots = roots;
    m_unavailableRootCount = unavailableRoots;
    emit rootsChanged();
    emit libraryChanged();
}

void LocalLibraryService::refreshTracks()
{
    LocalLibraryRepository repository;
    if (!repository.open(m_databasePath)) {
        m_trackModel.setRecords({});
        emit tracksChanged();
        return;
    }

    m_trackModel.setRecords(repository.tracks(m_searchText));
    emit tracksChanged();
}

void LocalLibraryService::setScanning(bool scanning)
{
    if (m_scanning == scanning)
        return;

    m_scanning = scanning;
    emit scanningChanged();
}

void LocalLibraryService::setLastScanStatus(const QString &status)
{
    if (m_lastScanStatus == status)
        return;

    m_lastScanStatus = status;
    emit scanStatusChanged();
}

void LocalLibraryService::setErrorString(const QString &message)
{
    if (m_errorString == message)
        return;

    m_errorString = message;
    emit errorChanged();
}

void LocalLibraryService::setScannedFileCount(int count)
{
    if (m_scannedFileCount == count)
        return;

    m_scannedFileCount = count;
    emit scanProgressChanged();
}
