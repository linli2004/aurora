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

struct DefaultMusicDirectory { QUrl url; QString label; bool available = false; }; DefaultMusicDirectory resolveDefaultMusicDirectory() { QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation); if (musicPath.isEmpty()) musicPath = QDir::home().filePath(QStringLiteral("Music")); const QFileInfo info(musicPath); const bool available = info.exists() && info.isDir() && info.isReadable(); return { QUrl::fromLocalFile(info.absoluteFilePath()), QDir::toNativeSeparators(info.absoluteFilePath()), available }; } bool supportedAudioFile(const QFileInfo &fileInfo)
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
}

LocalLibraryService::LocalLibraryService(QObject *parent)
    : QObject(parent)
    , m_databasePath(defaultDatabasePath())
{ const DefaultMusicDirectory defaultMusic = resolveDefaultMusicDirectory(); m_defaultMusicDirectory = defaultMusic.url; m_defaultMusicDirectoryLabel = defaultMusic.label; m_defaultMusicDirectoryAvailable = defaultMusic.available;
    int startupMissingSources = 0;
    {
        LocalLibraryRepository repository;
        if (repository.open(m_databasePath)
            && repository.reconcileMissingSources(&startupMissingSources)
            && startupMissingSources > 0) {
            setLastScanStatus(tr("%n missing local source(s)", nullptr,
                                 startupMissingSources));
        }
    }


    refreshCounts();
    refreshTracks();
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
} bool LocalLibraryService::firstRun() const { return !m_scanning && m_sourceCount == 0 && m_trackCount == 0 && m_missingSourceCount == 0; } QUrl LocalLibraryService::defaultMusicDirectory() const { return m_defaultMusicDirectory; } QString LocalLibraryService::defaultMusicDirectoryLabel() const { return m_defaultMusicDirectoryLabel; } bool LocalLibraryService::defaultMusicDirectoryAvailable() const { return m_defaultMusicDirectoryAvailable; }

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

void LocalLibraryService::scanDefaultMusicDirectory() { if (!m_defaultMusicDirectoryAvailable) { setErrorString(tr("The default Music folder is not readable. Choose another folder.")); return; } scanDirectory(m_defaultMusicDirectory); } void LocalLibraryService::scanDirectory(const QUrl &directory)
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

    setErrorString({});
    setScannedFileCount(0);
    setLastScanStatus(tr("Scanning %1").arg(QDir::toNativeSeparators(rootInfo.absoluteFilePath())));
    setScanning(true);

    const QString rootPath = rootInfo.canonicalFilePath();
    const QString databasePath = m_databasePath;
    m_cancelScan = std::make_shared<std::atomic_bool>(false);
    const std::shared_ptr<std::atomic_bool> cancelFlag = m_cancelScan;
    QThread *worker = QThread::create([this, rootPath, databasePath, cancelFlag]() {
        LocalLibraryRepository repository;
        int scanned = 0;
        int missingSources = 0;
        bool cancelled = false;
        QString error;

        if (!repository.open(databasePath)) {
            error = repository.lastError();
        } else {
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
                    QMetaObject::invokeMethod(this, [this, scanned]() {
                        setScannedFileCount(scanned);
                    }, Qt::QueuedConnection);
                }
            }
            if (error.isEmpty() && !cancelled
                && !repository.reconcileMissingSources(&missingSources)) {
                error = repository.lastError();
            }

        }

        QMetaObject::invokeMethod(this, [this, scanned, cancelled, error, missingSources]() {
            if (!error.isEmpty())
                setErrorString(error);
            setScannedFileCount(scanned);
            refreshCounts();
            refreshTracks();
            QString completionStatus;
        if (!error.isEmpty()) {
            completionStatus = tr("Scan failed");
        } else if (cancelled) {
            completionStatus = tr("Scan cancelled after %1 local audio files").arg(scanned);
        } else if (missingSources > 0) {
            completionStatus =
                tr("Scanned %1 local audio files · %n missing source(s)",
                   nullptr, missingSources)
                    .arg(scanned);
        } else {
            completionStatus = tr("Scanned %1 local audio files").arg(scanned);
        }
        setLastScanStatus(completionStatus);
            setScanning(false);
            m_cancelScan.reset();
        }, Qt::QueuedConnection);
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
    if (m_trackCount == tracks && m_sourceCount == sources && m_missingSourceCount == missingSources)
        return;

    m_trackCount = tracks;
    m_sourceCount = sources;
    m_missingSourceCount = missingSources;
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
