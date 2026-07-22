#pragma once

#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "runtime/audio/LocalTrackIdentity.h"

struct LocalLibrarySourceRecord
{
    LocalTrackIdentity identity;
    qint64 fileSize = 0;
    QDateTime modifiedTime;
};

struct LocalLibraryTrackRecord
{
    QString trackId;
    QString sourceId;
    QString title;
    QString artist;
    QString album;
    QString filePath;
    QString availability;
    qint64 fileSize = 0;
    QDateTime modifiedTime;
};

class LocalLibraryRepository final
{
public:
    explicit LocalLibraryRepository(QString connectionName = {});
    ~LocalLibraryRepository();

    [[nodiscard]] bool open(const QString &databasePath);
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] int trackCount() const;
    [[nodiscard]] int sourceCount() const;
    [[nodiscard]] QStringList playableFilePaths() const;
    [[nodiscard]] QList<LocalLibraryTrackRecord> tracks(const QString &searchText = {}) const;
    [[nodiscard]] QVariantMap setting(const QString &key) const;

    bool initializeSchema();
    bool upsertSource(const LocalLibrarySourceRecord &record);
    bool setSetting(const QString &key, const QVariantMap &value);

private:
    [[nodiscard]] QString identityState(const LocalTrackIdentity &identity) const;
    [[nodiscard]] QString identityAlgorithm(const LocalTrackIdentity &identity) const;
    [[nodiscard]] int identityVersion(const LocalTrackIdentity &identity) const;
    [[nodiscard]] QString availabilityReason(const LocalTrackIdentity &identity) const;
    [[nodiscard]] QString provenanceJson(const QString &source) const;
    [[nodiscard]] bool execSchema(const QString &sql);

    QString m_connectionName;
    QSqlDatabase m_database;
    QString m_lastError;
};
