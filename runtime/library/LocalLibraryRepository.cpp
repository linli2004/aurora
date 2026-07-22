#include "runtime/library/LocalLibraryRepository.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {
QString nowUtc()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

QString optionalTime(const QDateTime &value)
{
    return value.isValid() ? value.toUTC().toString(Qt::ISODateWithMs) : QString();
}

QString nonNull(const QString &value)
{
    return value.isNull() ? QStringLiteral("") : value;
}
}

LocalLibraryRepository::LocalLibraryRepository(QString connectionName)
    : m_connectionName(connectionName.isEmpty()
        ? QStringLiteral("aurora-library-%1").arg(QUuid::createUuid().toString(QUuid::Id128))
        : std::move(connectionName))
{
}

LocalLibraryRepository::~LocalLibraryRepository()
{
    if (m_database.isValid()) {
        m_database.close();
        m_database = {};
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool LocalLibraryRepository::open(const QString &databasePath)
{
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(databasePath);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    return initializeSchema();
}

QString LocalLibraryRepository::lastError() const
{
    return m_lastError;
}

int LocalLibraryRepository::trackCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM tracks"))) {
        return 0;
    }
    return query.next() ? query.value(0).toInt() : 0;
}

int LocalLibraryRepository::sourceCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM track_sources"))) {
        return 0;
    }
    return query.next() ? query.value(0).toInt() : 0;
}

QStringList LocalLibraryRepository::playableFilePaths() const
{
    QStringList paths;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT file_path FROM track_sources "
            "WHERE availability = 'Available' "
            "ORDER BY file_path"))) {
        return paths;
    }

    while (query.next())
        paths.append(query.value(0).toString());
    return paths;
}

bool LocalLibraryRepository::initializeSchema()
{
    return execSchema(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS schema_migrations ("
               "version INTEGER PRIMARY KEY,"
               "applied_at TEXT NOT NULL)"))
        && execSchema(QStringLiteral(
               "INSERT OR IGNORE INTO schema_migrations(version, applied_at) "
               "VALUES(1, '%1')").arg(nowUtc()))
        && execSchema(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS tracks ("
               "track_id TEXT PRIMARY KEY,"
               "identity_state TEXT NOT NULL,"
               "identity_algorithm TEXT NOT NULL,"
               "identity_version INTEGER NOT NULL,"
               "canonical_title TEXT NOT NULL,"
               "artist TEXT NOT NULL DEFAULT '',"
               "album TEXT NOT NULL DEFAULT '',"
               "duration_ms INTEGER,"
               "preferred_artwork_id TEXT,"
               "created_at TEXT NOT NULL,"
               "updated_at TEXT NOT NULL,"
               "provenance_json TEXT NOT NULL)"))
        && execSchema(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS track_sources ("
               "source_id TEXT PRIMARY KEY,"
               "track_id TEXT,"
               "source_kind TEXT NOT NULL,"
               "file_path TEXT NOT NULL,"
               "availability TEXT NOT NULL,"
               "availability_reason TEXT NOT NULL,"
               "file_size INTEGER,"
               "modified_time TEXT,"
               "last_verified_at TEXT,"
               "created_at TEXT NOT NULL,"
               "updated_at TEXT NOT NULL,"
               "provenance_json TEXT NOT NULL,"
               "FOREIGN KEY(track_id) REFERENCES tracks(track_id))"))
        && execSchema(QStringLiteral(
               "CREATE INDEX IF NOT EXISTS idx_track_sources_track_id "
               "ON track_sources(track_id)"))
        && execSchema(QStringLiteral(
               "CREATE INDEX IF NOT EXISTS idx_track_sources_file_path "
               "ON track_sources(file_path)"))
        && execSchema(QStringLiteral(
               "CREATE INDEX IF NOT EXISTS idx_tracks_canonical_title "
               "ON tracks(canonical_title)"));
}

bool LocalLibraryRepository::upsertSource(const LocalLibrarySourceRecord &record)
{
    if (!m_database.transaction()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    const QString timestamp = nowUtc();
    const QString trackProvenance = provenanceJson(QStringLiteral("AUR-018 scanner"));
    const QString state = identityState(record.identity);

    QSqlQuery track(m_database);
    track.prepare(QStringLiteral(
        "INSERT INTO tracks("
        "track_id, identity_state, identity_algorithm, identity_version, canonical_title, "
        "artist, album, duration_ms, preferred_artwork_id, created_at, updated_at, provenance_json"
        ") VALUES(?, ?, ?, ?, ?, ?, ?, NULL, NULL, ?, ?, ?) "
        "ON CONFLICT(track_id) DO UPDATE SET "
        "identity_state = excluded.identity_state,"
        "canonical_title = excluded.canonical_title,"
        "artist = excluded.artist,"
        "album = excluded.album,"
        "updated_at = excluded.updated_at,"
        "provenance_json = excluded.provenance_json"));
    track.addBindValue(record.identity.trackId);
    track.addBindValue(state);
    track.addBindValue(identityAlgorithm(record.identity));
    track.addBindValue(identityVersion(record.identity));
    track.addBindValue(nonNull(record.identity.canonicalTitle));
    track.addBindValue(nonNull(record.identity.artist));
    track.addBindValue(nonNull(record.identity.album));
    track.addBindValue(timestamp);
    track.addBindValue(timestamp);
    track.addBindValue(trackProvenance);

    if (!track.exec()) {
        m_lastError = track.lastError().text();
        m_database.rollback();
        return false;
    }

    QSqlQuery source(m_database);
    source.prepare(QStringLiteral(
        "INSERT INTO track_sources("
        "source_id, track_id, source_kind, file_path, availability, availability_reason, "
        "file_size, modified_time, last_verified_at, created_at, updated_at, provenance_json"
        ") VALUES(?, ?, 'LocalFile', ?, ?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(source_id) DO UPDATE SET "
        "track_id = excluded.track_id,"
        "file_path = excluded.file_path,"
        "availability = excluded.availability,"
        "availability_reason = excluded.availability_reason,"
        "file_size = excluded.file_size,"
        "modified_time = excluded.modified_time,"
        "last_verified_at = excluded.last_verified_at,"
        "updated_at = excluded.updated_at,"
        "provenance_json = excluded.provenance_json"));
    source.addBindValue(record.identity.sourceId);
    source.addBindValue(record.identity.trackId);
    source.addBindValue(record.identity.filePath);
    source.addBindValue(record.identity.availability);
    source.addBindValue(availabilityReason(record.identity));
    source.addBindValue(record.fileSize);
    source.addBindValue(optionalTime(record.modifiedTime));
    source.addBindValue(timestamp);
    source.addBindValue(timestamp);
    source.addBindValue(timestamp);
    source.addBindValue(trackProvenance);

    if (!source.exec()) {
        m_lastError = source.lastError().text();
        m_database.rollback();
        return false;
    }

    if (!m_database.commit()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    return true;
}

QString LocalLibraryRepository::identityState(const LocalTrackIdentity &identity) const
{
    return identity.trackId.startsWith(QStringLiteral("track:local-content:v1:"))
        ? QStringLiteral("Resolved")
        : QStringLiteral("Provisional");
}

QString LocalLibraryRepository::identityAlgorithm(const LocalTrackIdentity &identity) const
{
    return identity.trackId.startsWith(QStringLiteral("track:local-content:v1:"))
        ? QStringLiteral("bounded-file-sample")
        : QStringLiteral("source-path-provisional");
}

int LocalLibraryRepository::identityVersion(const LocalTrackIdentity &) const
{
    return 1;
}

QString LocalLibraryRepository::availabilityReason(const LocalTrackIdentity &identity) const
{
    return identity.availability == QStringLiteral("Available")
        ? QStringLiteral("None")
        : QStringLiteral("Missing");
}

QString LocalLibraryRepository::provenanceJson(const QString &source) const
{
    QJsonObject object;
    object.insert(QStringLiteral("source"), source);
    object.insert(QStringLiteral("processingLocation"), QStringLiteral("Device"));
    object.insert(QStringLiteral("generatedBy"), QStringLiteral("Aurora Local Library"));
    object.insert(QStringLiteral("modelVersion"), QStringLiteral("local-repository-v1"));
    object.insert(QStringLiteral("userConfirmed"), false);
    object.insert(QStringLiteral("lastUpdated"), nowUtc());
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

bool LocalLibraryRepository::execSchema(const QString &sql)
{
    QSqlQuery query(m_database);
    if (query.exec(sql))
        return true;

    m_lastError = query.lastError().text();
    return false;
}
