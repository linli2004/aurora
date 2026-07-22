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
}

LocalLibraryService::LocalLibraryService(QObject *parent)
    : QObject(parent)
    , m_databasePath(defaultDatabasePath())
{
    refreshCounts();
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

    setErrorString({});
    setLastScanStatus(tr("Scanning %1").arg(QDir::toNativeSeparators(rootInfo.absoluteFilePath())));
    setScanning(true);

    const QString rootPath = rootInfo.canonicalFilePath();
    const QString databasePath = m_databasePath;
    QThread *worker = QThread::create([this, rootPath, databasePath]() {
        LocalLibraryRepository repository;
        int scanned = 0;
        QString error;

        if (!repository.open(databasePath)) {
            error = repository.lastError();
        } else {
            QDirIterator iterator(
                rootPath,
                QDir::Files | QDir::Readable | QDir::NoSymLinks,
                QDirIterator::Subdirectories);

            while (iterator.hasNext()) {
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
            }
        }

        QMetaObject::invokeMethod(this, [this, scanned, error]() {
            if (!error.isEmpty())
                setErrorString(error);
            refreshCounts();
            setLastScanStatus(error.isEmpty()
                ? tr("Scanned %1 local audio files").arg(scanned)
                : tr("Scan failed"));
            setScanning(false);
        }, Qt::QueuedConnection);
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
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

void LocalLibraryService::refreshCounts()
{
    LocalLibraryRepository repository;
    if (!repository.open(m_databasePath)) {
        m_trackCount = 0;
        m_sourceCount = 0;
        emit libraryChanged();
        return;
    }

    const int tracks = repository.trackCount();
    const int sources = repository.sourceCount();
    if (m_trackCount == tracks && m_sourceCount == sources)
        return;

    m_trackCount = tracks;
    m_sourceCount = sources;
    emit libraryChanged();
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
