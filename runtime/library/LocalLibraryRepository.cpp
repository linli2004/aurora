#include "runtime/library/LocalLibraryRepository.h"

#include <QCoreApplication>

#include <QFileInfo>
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

QDateTime parseTime(const QString &value)
{
    return QDateTime::fromString(value, Qt::ISODateWithMs);
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
    if (!query.exec(QStringLiteral(
            "SELECT COUNT(DISTINCT track_id) FROM track_sources "
            "WHERE availability = 'Available'"))) {
        return 0;
    }
    return query.next() ? query.value(0).toInt() : 0;
}

int LocalLibraryRepository::sourceCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT COUNT(*) FROM track_sources "
            "WHERE availability = 'Available'"))) {
        return 0;
    }
    return query.next() ? query.value(0).toInt() : 0;
}

int LocalLibraryRepository::missingSourceCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT COUNT(*) FROM track_sources "
            "WHERE availability = 'Unavailable'"))) {
        return 0;
    }
    return query.next() ? query.value(0).toInt() : 0;
}

QStringList LocalLibraryRepository::playableFilePaths() const
{
    QStringList paths;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT s.file_path FROM track_sources s "
            "WHERE s.availability = 'Available' "
            "AND s.source_id = ("
            "SELECT s2.source_id FROM track_sources s2 "
            "WHERE s2.track_id = s.track_id "
            "AND s2.availability = 'Available' "
            "ORDER BY COALESCE(s2.last_verified_at, '') DESC, "
            "COALESCE(s2.updated_at, '') DESC, "
            "s2.file_path ASC, s2.source_id ASC "
            "LIMIT 1"
            ") "
            "ORDER BY s.file_path"))) {
        return paths;
    }

    while (query.next())
        paths.append(query.value(0).toString());
    return paths;
}

QList<LocalLibraryTrackRecord> LocalLibraryRepository::tracks(
    const QString &searchText) const
{
    QList<LocalLibraryTrackRecord> records;
    QSqlQuery query(m_database);

    const QString trimmedSearch = searchText.trimmed().toCaseFolded();
    const bool hasSearch = !trimmedSearch.isEmpty();

    QString sql = QStringLiteral(
        "SELECT t.track_id, s.source_id, t.canonical_title, t.artist, t.album, "
        "s.file_path, s.availability, s.file_size, s.modified_time "
        "FROM track_sources s "
        "JOIN tracks t ON t.track_id = s.track_id "
        "WHERE s.availability = 'Available' "
        "AND s.source_id = ("
        "SELECT s2.source_id FROM track_sources s2 "
        "WHERE s2.track_id = s.track_id "
        "AND s2.availability = 'Available' "
        "ORDER BY COALESCE(s2.last_verified_at, '') DESC, "
        "COALESCE(s2.updated_at, '') DESC, "
        "s2.file_path ASC, s2.source_id ASC "
        "LIMIT 1"
        ")");

    if (hasSearch) {
        sql += QStringLiteral(
            " AND (t.canonical_title LIKE ? OR t.artist LIKE ? OR t.album LIKE ? "
            "OR EXISTS ("
            "SELECT 1 FROM track_sources search_source "
            "WHERE search_source.track_id = t.track_id "
            "AND search_source.availability = 'Available' "
            "AND search_source.file_path LIKE ?"
            "))");
    }

    sql += QStringLiteral(" ORDER BY t.canonical_title, s.file_path");

    if (!query.prepare(sql))
        return records;

    if (hasSearch) {
        const QString pattern = QStringLiteral("%%1%").arg(trimmedSearch);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    }

    if (!query.exec())
        return records;

    while (query.next()) {
        LocalLibraryTrackRecord record;
        record.trackId = query.value(0).toString();
        record.sourceId = query.value(1).toString();
        record.title = query.value(2).toString();
        record.artist = query.value(3).toString();
        record.album = query.value(4).toString();
        record.filePath = query.value(5).toString();
        record.availability = query.value(6).toString();
        record.fileSize = query.value(7).toLongLong();
        record.modifiedTime = parseTime(query.value(8).toString());
        records.append(record);
    }

    return records;
}

QVariantMap LocalLibraryRepository::setting(const QString &key) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT value_json FROM settings WHERE key = ?"));
    query.addBindValue(key);

    if (!query.exec() || !query.next())
        return {};

    const QJsonDocument document = QJsonDocument::fromJson(query.value(0).toByteArray());
    return document.isObject() ? document.object().toVariantMap() : QVariantMap {};
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
               "CREATE TABLE IF NOT EXISTS settings ("
               "key TEXT PRIMARY KEY,"
               "value_json TEXT NOT NULL,"
               "updated_at TEXT NOT NULL)"))
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

bool LocalLibraryRepository::reconcileMissingSources(int *markedMissing)
{
    if (markedMissing)
        *markedMissing = 0;

    QSqlQuery sources(m_database);
    if (!sources.exec(QStringLiteral(
            "SELECT source_id, file_path FROM track_sources "
            "WHERE source_kind = 'LocalFile' AND availability = 'Available'"))) {
        m_lastError = sources.lastError().text();
        return false;
    }

    QStringList missingSourceIds;
    while (sources.next()) {
        const QString sourceId = sources.value(0).toString();
        const QFileInfo fileInfo(sources.value(1).toString());
        if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable())
            missingSourceIds.append(sourceId);
    }
    sources.finish();

    if (missingSourceIds.isEmpty())
        return true;

    if (!m_database.transaction()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    const QString timestamp = nowUtc();
    const QString reconciliationProvenance =
        provenanceJson(QStringLiteral("AUR-025 missing-source reconciliation"));

    QSqlQuery update(m_database);
    update.prepare(QStringLiteral(
        "UPDATE track_sources SET "
        "availability = 'Unavailable',"
        "availability_reason = 'Missing',"
        "last_verified_at = ?,"
        "updated_at = ?,"
        "provenance_json = ? "
        "WHERE source_id = ? AND availability = 'Available'"));

    for (const QString &sourceId : missingSourceIds) {
        update.bindValue(0, timestamp);
        update.bindValue(1, timestamp);
        update.bindValue(2, reconciliationProvenance);
        update.bindValue(3, sourceId);

        if (!update.exec()) {
            m_lastError = update.lastError().text();
            m_database.rollback();
            return false;
        }
    }

    if (!m_database.commit()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    if (markedMissing)
        *markedMissing = missingSourceIds.size();
    return true;
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

bool LocalLibraryRepository::setSetting(const QString &key, const QVariantMap &value)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO settings(key, value_json, updated_at) VALUES(?, ?, ?) "
        "ON CONFLICT(key) DO UPDATE SET "
        "value_json = excluded.value_json,"
        "updated_at = excluded.updated_at"));
    query.addBindValue(key);
    query.addBindValue(QString::fromUtf8(
        QJsonDocument(QJsonObject::fromVariantMap(value)).toJson(QJsonDocument::Compact)));
    query.addBindValue(nowUtc());

    if (query.exec())
        return true;

    m_lastError = query.lastError().text();
    return false;
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
