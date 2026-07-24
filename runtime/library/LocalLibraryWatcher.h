#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QStringList>
#include <QTimer>

class LocalLibraryWatcher final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int watchedDirectoryCount
               READ watchedDirectoryCount
               NOTIFY watchedDirectoriesChanged)

public:
    explicit LocalLibraryWatcher(QObject *parent = nullptr);

    [[nodiscard]] int watchedDirectoryCount() const;
    [[nodiscard]] QStringList watchedDirectories() const;

    void setWatchedDirectories(const QStringList &directories);
    void clear();
    void setDebounceInterval(int intervalMs);

signals:
    void refreshRequested();
    void watchedDirectoriesChanged();

private slots:
    void scheduleRefresh();

private:
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
    QStringList m_watchedDirectories;
};
