#pragma once

#include <QObject>
#include <QUrl>
#include <QVariantList>

class LocalLibraryService final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY libraryChanged)
    Q_PROPERTY(int sourceCount READ sourceCount NOTIFY libraryChanged)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
    Q_PROPERTY(QString lastScanStatus READ lastScanStatus NOTIFY scanStatusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit LocalLibraryService(QObject *parent = nullptr);

    [[nodiscard]] bool scanning() const;
    [[nodiscard]] int trackCount() const;
    [[nodiscard]] int sourceCount() const;
    [[nodiscard]] QString databasePath() const;
    [[nodiscard]] QString lastScanStatus() const;
    [[nodiscard]] QString errorString() const;

    Q_INVOKABLE void scanDirectory(const QUrl &directory);
    Q_INVOKABLE QVariantList playableUrls() const;

signals:
    void scanningChanged();
    void libraryChanged();
    void scanStatusChanged();
    void errorChanged();

private:
    void refreshCounts();
    void setScanning(bool scanning);
    void setLastScanStatus(const QString &status);
    void setErrorString(const QString &message);

    QString m_databasePath;
    bool m_scanning = false;
    int m_trackCount = 0;
    int m_sourceCount = 0;
    QString m_lastScanStatus;
    QString m_errorString;
};
