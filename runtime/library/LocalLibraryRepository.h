#pragma once

#include <QDateTime>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include "runtime/audio/LocalTrackIdentity.h"

struct LocalLibrarySourceRecord
{
    LocalTrackIdentity identity;
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

    bool initializeSchema();
    bool upsertSource(const LocalLibrarySourceRecord &record);

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
