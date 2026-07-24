#include "runtime/memory/MomentService.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>

namespace {
QString defaultDatabasePath()
{
    QString dataLocation = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty())
        dataLocation = QDir::home().filePath(QStringLiteral(".local/share/Aurora"));

    QDir directory;
    directory.mkpath(dataLocation);
    return QDir(dataLocation).filePath(QStringLiteral("library.sqlite"));
}

QString normalizedPath(const QString &path)
{
    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
}
}

MomentService::MomentService(QObject *parent)
    : QObject(parent)
    , m_databasePath(defaultDatabasePath())
{
    if (!m_repository.open(m_databasePath)) {
        setErrorString(m_repository.lastError());
        return;
    }

    refresh();
}

bool MomentService::hasMoment() const
{
    return m_latestMoment.has_value();
}

int MomentService::momentCount() const
{
    return m_momentCount;
}

QString MomentService::latestMomentId() const
{
    return m_latestMoment ? m_latestMoment->momentId : QString();
}

QString MomentService::latestTrackId() const
{
    return m_latestMoment ? m_latestMoment->trackId : QString();
}

QString MomentService::latestTitle() const
{
    return m_latestMoment ? m_latestMoment->title : QString();
}

QString MomentService::latestArtist() const
{
    return m_latestMoment ? m_latestMoment->artist : QString();
}

QString MomentService::latestAlbum() const
{
    return m_latestMoment ? m_latestMoment->album : QString();
}

QUrl MomentService::latestArtworkSource() const
{
    return m_latestMoment ? QUrl(m_latestMoment->artworkUrl) : QUrl();
}

QColor MomentService::latestIdentityColor() const
{
    const QColor color(m_latestMoment ? m_latestMoment->identityColor : QString());
    return color.isValid() ? color : QColor(QStringLiteral("#7c8fcf"));
}

QString MomentService::latestPeriodLabel() const
{
    return m_latestMoment ? m_latestMoment->periodLabel : QString();
}

QString MomentService::latestConfirmedMeaning() const
{
    return m_latestMoment ? m_latestMoment->confirmedMeaning : QString();
}

bool MomentService::latestAvailable() const
{
    return !m_resolvedPlayablePath.isEmpty();
}

QUrl MomentService::latestUrl() const
{
    return latestAvailable()
        ? QUrl::fromLocalFile(m_resolvedPlayablePath)
        : QUrl();
}

QString MomentService::lastStatus() const
{
    return m_lastStatus;
}

QString MomentService::errorString() const
{
    return m_errorString;
}

bool MomentService::keepCurrentMoment(
    const QString &trackId,
    const QString &sourceId,
    const QString &filePath,
    const QString &title,
    const QString &artist,
    const QString &album,
    const QUrl &artworkSource,
    const QColor &identityColor,
    const QString &confirmedMeaning)
{
    if (trackId.trimmed().isEmpty() || filePath.trimmed().isEmpty()) {
        setErrorString(tr("Load a local track before keeping a moment."));
        return false;
    }

    setErrorString({});

    MomentRecord moment;
    moment.momentId = QStringLiteral("moment:%1").arg(
        QUuid::createUuid().toString(QUuid::Id128));
    moment.trackId = trackId;
    moment.sourceId = sourceId;
    moment.sourcePath = normalizedPath(filePath);
    moment.title = title.trimmed().isEmpty()
        ? QFileInfo(filePath).completeBaseName()
        : title.trimmed();
    moment.artist = artist.trimmed();
    moment.album = album.trimmed();
    moment.artworkUrl = artworkSource.toString();
    moment.identityColor = identityColor.isValid()
        ? identityColor.name(QColor::HexArgb)
        : QStringLiteral("#ff7c8fcf");
    moment.periodLabel = periodLabel(QDateTime::currentDateTime());
    moment.confirmedMeaning = confirmedMeaning.trimmed();
    moment.createdAt = QDateTime::currentDateTimeUtc();

    if (!m_repository.keepMoment(moment)) {
        setErrorString(m_repository.lastError());
        return false;
    }

    refresh();
    setLastStatus(tr("Moment kept on this device"));
    return true;
}

void MomentService::refresh()
{
    const int count = m_repository.count();
    const std::optional<MomentRecord> latest = m_repository.latestMoment();
    const QString resolved = latest
        ? m_repository.resolvedPlayablePath(*latest)
        : QString();

    const bool changed = m_momentCount != count
        || m_latestMoment.has_value() != latest.has_value()
        || (m_latestMoment && latest
            && (m_latestMoment->momentId != latest->momentId
                || m_latestMoment->trackId != latest->trackId
                || m_latestMoment->title != latest->title
                || m_latestMoment->artist != latest->artist
                || m_latestMoment->album != latest->album
                || m_latestMoment->artworkUrl != latest->artworkUrl
                || m_latestMoment->identityColor != latest->identityColor
                || m_latestMoment->periodLabel != latest->periodLabel
                || m_latestMoment->confirmedMeaning != latest->confirmedMeaning))
        || m_resolvedPlayablePath != resolved;

    m_momentCount = count;
    m_latestMoment = latest;
    m_resolvedPlayablePath = resolved;

    if (changed)
        emit momentsChanged();
}

QString MomentService::periodLabel(const QDateTime &localTime) const
{
    const int hour = localTime.time().hour();
    if (hour < 5)
        return tr("After Midnight");
    if (hour < 12)
        return tr("Morning");
    if (hour < 17)
        return tr("Afternoon");
    if (hour < 21)
        return tr("Evening");
    return tr("Late Night");
}

void MomentService::setLastStatus(const QString &status)
{
    if (m_lastStatus == status)
        return;

    m_lastStatus = status;
    emit statusChanged();
}

void MomentService::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;

    m_errorString = error;
    emit errorChanged();
}
