#include "runtime/memory/MomentRepository.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include <utility>

namespace {
QString nowUtc()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

QString normalizedReadablePath(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile() || !info.isReadable())
        return {};

    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
}

QString nonNull(const QString &value)
{
    return value.isNull() ? QStringLiteral("") : value;
}

MomentRecord readMoment(const QSqlQuery &query)
{
    MomentRecord moment;
    moment.momentId = query.value(0).toString();
    moment.trackId = query.value(1).toString();
    moment.sourceId = query.value(2).toString();
    moment.sourcePath = query.value(3).toString();
    moment.title = query.value(4).toString();
    moment.artist = query.value(5).toString();
    moment.album = query.value(6).toString();
    moment.artworkUrl = query.value(7).toString();
    moment.identityColor = query.value(8).toString();
    moment.periodLabel = query.value(9).toString();
    moment.confirmedMeaning = query.value(10).toString();
    moment.createdAt = QDateTime::fromString(
        query.value(11).toString(), Qt::ISODateWithMs);
    return moment;
}

QString momentProjection()
{
    return QStringLiteral(
        "moment_id, track_id, source_id, source_path, title, artist, album, "
        "artwork_url, identity_color, period_label, confirmed_meaning, created_at");
}
}

MomentRepository::MomentRepository(QString connectionName)
    : m_connectionName(connectionName.isEmpty()
        ? QStringLiteral("aurora-moments-%1").arg(
              QUuid::createUuid().toString(QUuid::Id128))
        : std::move(connectionName))
{
}

MomentRepository::~MomentRepository()
{
    if (m_database.isValid()) {
        m_database.close();
        m_database = {};
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool MomentRepository::open(const QString &databasePath)
{
    m_database = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(databasePath);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    return initializeSchema();
}

QString MomentRepository::lastError() const
{
    return m_lastError;
}

int MomentRepository::count() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM moments")))
        return 0;

    return query.next() ? query.value(0).toInt() : 0;
}

QList<MomentRecord> MomentRepository::moments() const
{
    QList<MomentRecord> records;
    QSqlQuery query(m_database);
    const QString sql = QStringLiteral("SELECT ")
        + momentProjection()
        + QStringLiteral(
            " FROM moments ORDER BY created_at DESC, rowid DESC");

    if (!query.exec(sql))
        return records;

    while (query.next())
        records.append(readMoment(query));
    return records;
}

std::optional<MomentRecord> MomentRepository::moment(
    const QString &momentId) const
{
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("SELECT ")
        + momentProjection()
        + QStringLiteral(" FROM moments WHERE moment_id = ? LIMIT 1"));
    query.addBindValue(momentId);

    if (!query.exec() || !query.next())
        return std::nullopt;
    return readMoment(query);
}

std::optional<MomentRecord> MomentRepository::latestMoment() const
{
    QSqlQuery query(m_database);
    const QString sql = QStringLiteral("SELECT ")
        + momentProjection()
        + QStringLiteral(
            " FROM moments ORDER BY created_at DESC, rowid DESC LIMIT 1");

    if (!query.exec(sql) || !query.next())
        return std::nullopt;
    return readMoment(query);
}

QString MomentRepository::resolvedPlayablePath(
    const MomentRecord &moment) const
{
    if (m_database.tables().contains(QStringLiteral("track_sources"))) {
        QSqlQuery query(m_database);
        query.prepare(QStringLiteral(
            "SELECT file_path FROM track_sources "
            "WHERE track_id = ? AND availability = 'Available' "
            "ORDER BY COALESCE(last_verified_at, '') DESC, "
            "COALESCE(updated_at, '') DESC, file_path ASC, source_id ASC "
            "LIMIT 1"));
        query.addBindValue(moment.trackId);

        if (query.exec() && query.next()) {
            const QString activePath =
                normalizedReadablePath(query.value(0).toString());
            if (!activePath.isEmpty())
                return activePath;
        }
    }

    return normalizedReadablePath(moment.sourcePath);
}

bool MomentRepository::initializeSchema()
{
    return execSchema(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS moments ("
        "moment_id TEXT PRIMARY KEY,"
        "track_id TEXT NOT NULL,"
        "source_id TEXT NOT NULL,"
        "source_path TEXT NOT NULL,"
        "title TEXT NOT NULL,"
        "artist TEXT NOT NULL DEFAULT '',"
        "album TEXT NOT NULL DEFAULT '',"
        "artwork_url TEXT NOT NULL DEFAULT '',"
        "identity_color TEXT NOT NULL,"
        "period_label TEXT NOT NULL,"
        "confirmed_meaning TEXT NOT NULL DEFAULT '',"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "provenance_json TEXT NOT NULL)"))
        && execSchema(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_moments_created_at "
            "ON moments(created_at DESC)"))
        && execSchema(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_moments_track_id "
            "ON moments(track_id)"));
}

bool MomentRepository::keepMoment(const MomentRecord &moment)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO moments("
        "moment_id, track_id, source_id, source_path, title, artist, album, "
        "artwork_url, identity_color, period_label, confirmed_meaning, "
        "created_at, updated_at, provenance_json"
        ") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(nonNull(moment.momentId));
    query.addBindValue(nonNull(moment.trackId));
    query.addBindValue(nonNull(moment.sourceId));
    query.addBindValue(nonNull(moment.sourcePath));
    query.addBindValue(nonNull(moment.title));
    query.addBindValue(nonNull(moment.artist));
    query.addBindValue(nonNull(moment.album));
    query.addBindValue(nonNull(moment.artworkUrl));
    query.addBindValue(nonNull(moment.identityColor));
    query.addBindValue(nonNull(moment.periodLabel));
    query.addBindValue(nonNull(moment.confirmedMeaning));
    query.addBindValue(moment.createdAt.toUTC().toString(Qt::ISODateWithMs));
    query.addBindValue(nowUtc());
    query.addBindValue(provenanceJson());

    if (query.exec())
        return true;

    m_lastError = query.lastError().text();
    return false;
}

bool MomentRepository::updateConfirmedMeaning(
    const QString &momentId,
    const QString &confirmedMeaning)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE moments SET confirmed_meaning = ?, updated_at = ?, "
        "provenance_json = ? WHERE moment_id = ?"));
    query.addBindValue(nonNull(confirmedMeaning));
    query.addBindValue(nowUtc());
    query.addBindValue(provenanceJson());
    query.addBindValue(momentId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() > 0)
        return true;

    m_lastError = QStringLiteral("Moment not found");
    return false;
}

bool MomentRepository::execSchema(const QString &sql)
{
    QSqlQuery query(m_database);
    if (query.exec(sql))
        return true;

    m_lastError = query.lastError().text();
    return false;
}

QString MomentRepository::provenanceJson() const
{
    QJsonObject object;
    object.insert(QStringLiteral("source"), QStringLiteral("Keep This Moment"));
    object.insert(QStringLiteral("processingLocation"), QStringLiteral("Device"));
    object.insert(QStringLiteral("generatedBy"), QStringLiteral("Aurora Memory"));
    object.insert(QStringLiteral("modelVersion"), QStringLiteral("moment-v1"));
    object.insert(QStringLiteral("userConfirmed"), true);
    object.insert(QStringLiteral("lastUpdated"), nowUtc());
    return QString::fromUtf8(
        QJsonDocument(object).toJson(QJsonDocument::Compact));
}
