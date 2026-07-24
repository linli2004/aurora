#include "runtime/library/LocalLibraryWatcher.h"

#include <QDir>
#include <QFileInfo>

#include <algorithm>

namespace {

QString normalizedDirectoryPath(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir() || !info.isReadable())
        return {};

    const QString canonicalPath = info.canonicalFilePath();
    return QDir::cleanPath(
        canonicalPath.isEmpty() ? info.absoluteFilePath() : canonicalPath);
}

} // namespace

LocalLibraryWatcher::LocalLibraryWatcher(QObject *parent)
    : QObject(parent)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(900);

    connect(
        &m_watcher,
        &QFileSystemWatcher::directoryChanged,
        this,
        &LocalLibraryWatcher::scheduleRefresh);
    connect(
        &m_watcher,
        &QFileSystemWatcher::fileChanged,
        this,
        &LocalLibraryWatcher::scheduleRefresh);
    connect(
        &m_debounce,
        &QTimer::timeout,
        this,
        &LocalLibraryWatcher::refreshRequested);
}

int LocalLibraryWatcher::watchedDirectoryCount() const
{
    return m_watchedDirectories.size();
}

QStringList LocalLibraryWatcher::watchedDirectories() const
{
    return m_watchedDirectories;
}

void LocalLibraryWatcher::setWatchedDirectories(const QStringList &directories)
{
    QStringList normalizedDirectories;
    normalizedDirectories.reserve(directories.size());

    for (const QString &path : directories) {
        const QString normalized = normalizedDirectoryPath(path);
        if (!normalized.isEmpty() && !normalizedDirectories.contains(normalized))
            normalizedDirectories.append(normalized);
    }

    normalizedDirectories.sort(Qt::CaseInsensitive);

    const QStringList currentDirectories = m_watcher.directories();
    if (!currentDirectories.isEmpty())
        m_watcher.removePaths(currentDirectories);

    const QStringList currentFiles = m_watcher.files();
    if (!currentFiles.isEmpty())
        m_watcher.removePaths(currentFiles);

    if (!normalizedDirectories.isEmpty())
        m_watcher.addPaths(normalizedDirectories);

    QStringList watched = m_watcher.directories();
    watched.sort(Qt::CaseInsensitive);

    if (m_watchedDirectories == watched)
        return;

    m_watchedDirectories = watched;
    emit watchedDirectoriesChanged();
}

void LocalLibraryWatcher::clear()
{
    m_debounce.stop();

    const QStringList directories = m_watcher.directories();
    if (!directories.isEmpty())
        m_watcher.removePaths(directories);

    const QStringList files = m_watcher.files();
    if (!files.isEmpty())
        m_watcher.removePaths(files);

    if (m_watchedDirectories.isEmpty())
        return;

    m_watchedDirectories.clear();
    emit watchedDirectoriesChanged();
}

void LocalLibraryWatcher::setDebounceInterval(int intervalMs)
{
    m_debounce.setInterval(std::max(0, intervalMs));
}

void LocalLibraryWatcher::scheduleRefresh()
{
    m_debounce.start();
}
