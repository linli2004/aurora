#pragma once

#include <atomic>
#include <memory>

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVariantList>

#include "runtime/library/LocalLibraryTrackModel.h"
#include "runtime/library/LocalLibraryWatcher.h"

class LocalLibraryService final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY libraryChanged)
    Q_PROPERTY(int sourceCount READ sourceCount NOTIFY libraryChanged)
    Q_PROPERTY(int missingSourceCount READ missingSourceCount NOTIFY libraryChanged)
    Q_PROPERTY(int libraryRootCount READ libraryRootCount NOTIFY rootsChanged)
    Q_PROPERTY(int unavailableRootCount READ unavailableRootCount NOTIFY rootsChanged)
    Q_PROPERTY(QString libraryRootsSummary READ libraryRootsSummary NOTIFY rootsChanged)
    Q_PROPERTY(bool automaticRefreshActive READ automaticRefreshActive NOTIFY watchStateChanged)
    Q_PROPERTY(int watchedDirectoryCount READ watchedDirectoryCount NOTIFY watchStateChanged)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
    Q_PROPERTY(QString lastScanStatus READ lastScanStatus NOTIFY scanStatusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(bool firstRun READ firstRun NOTIFY libraryChanged)
    Q_PROPERTY(QUrl defaultMusicDirectory READ defaultMusicDirectory CONSTANT)
    Q_PROPERTY(QString defaultMusicDirectoryLabel READ defaultMusicDirectoryLabel CONSTANT)
    Q_PROPERTY(bool defaultMusicDirectoryAvailable READ defaultMusicDirectoryAvailable CONSTANT)
    Q_PROPERTY(int scannedFileCount READ scannedFileCount NOTIFY scanProgressChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchChanged)
    Q_PROPERTY(int visibleTrackCount READ visibleTrackCount NOTIFY tracksChanged)
    Q_PROPERTY(QAbstractListModel *tracks READ tracks CONSTANT)

public:
    explicit LocalLibraryService(QObject *parent = nullptr);

    [[nodiscard]] bool scanning() const;
    [[nodiscard]] int trackCount() const;
    [[nodiscard]] int sourceCount() const;
    [[nodiscard]] int missingSourceCount() const;
    [[nodiscard]] int libraryRootCount() const;
    [[nodiscard]] int unavailableRootCount() const;
    [[nodiscard]] QString libraryRootsSummary() const;
    [[nodiscard]] bool automaticRefreshActive() const;
    [[nodiscard]] int watchedDirectoryCount() const;
    [[nodiscard]] QString databasePath() const;
    [[nodiscard]] QString lastScanStatus() const;
    [[nodiscard]] QString errorString() const;
    [[nodiscard]] bool firstRun() const;
    [[nodiscard]] QUrl defaultMusicDirectory() const;
    [[nodiscard]] QString defaultMusicDirectoryLabel() const;
    [[nodiscard]] bool defaultMusicDirectoryAvailable() const;
    [[nodiscard]] int scannedFileCount() const;
    [[nodiscard]] QString searchText() const;
    [[nodiscard]] int visibleTrackCount() const;
    [[nodiscard]] QAbstractListModel *tracks();

    Q_INVOKABLE void scanDirectory(const QUrl &directory);
    Q_INVOKABLE void scanDefaultMusicDirectory();
    Q_INVOKABLE void rescanLibraryRoots();
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE QVariantList playableUrls() const;
    Q_INVOKABLE QVariantList visiblePlayableUrls() const;
    Q_INVOKABLE QVariantList visiblePlayableUrlsStartingAt(int row) const;

public slots:
    void setSearchText(const QString &searchText);

signals:
    void scanningChanged();
    void libraryChanged();
    void rootsChanged();
    void watchStateChanged();
    void scanStatusChanged();
    void errorChanged();
    void scanProgressChanged();
    void searchChanged();
    void tracksChanged();

private:
    void startScan(
        const QStringList &rootPaths,
        const QString &rootToRemember = {},
        bool automaticRefresh = false);
    void requestAutomaticRefresh();
    void refreshCounts();
    void refreshLibraryRoots();
    void refreshTracks();
    void setScanning(bool scanning);
    void setLastScanStatus(const QString &status);
    void setErrorString(const QString &message);
    void setScannedFileCount(int count);

    QString m_databasePath;
    QUrl m_defaultMusicDirectory;
    QString m_defaultMusicDirectoryLabel;
    bool m_defaultMusicDirectoryAvailable = false;
    LocalLibraryTrackModel m_trackModel;
    LocalLibraryWatcher m_libraryWatcher;
    std::shared_ptr<std::atomic_bool> m_cancelScan;
    QStringList m_libraryRoots;
    bool m_scanning = false;
    bool m_automaticRefreshPending = false;
    int m_scannedFileCount = 0;
    int m_trackCount = 0;
    int m_sourceCount = 0;
    int m_missingSourceCount = 0;
    int m_unavailableRootCount = 0;
    QString m_lastScanStatus;
    QString m_errorString;
    QString m_searchText;
};
