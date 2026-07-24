#include "runtime/memory/MomentService.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>
#include <QVariantMap>

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

QColor resolvedColor(const QString &value)
{
    const QColor color(value);
    return color.isValid() ? color : QColor(QStringLiteral("#7c8fcf"));
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

QVariantList MomentService::momentItems() const
{
    return m_momentItems;
}

QString MomentService::latestMomentId() const
{
    return m_latestMoment ? m_latestMoment->momentId : QString();
}

QString MomentService::latestTrackId() const
{
    return m_latestMoment ? m_latestMoment->trackId : QString();
}

QString MomentService::latestHeading() const
{
    return m_latestMoment
        ? headingForPeriod(m_latestMoment->periodLabel)
        : QString();
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
    return resolvedColor(
        m_latestMoment ? m_latestMoment->identityColor : QString());
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
    return !m_latestPlayablePath.isEmpty();
}

QUrl MomentService::latestUrl() const
{
    return latestAvailable()
        ? QUrl::fromLocalFile(m_latestPlayablePath)
        : QUrl();
}

QString MomentService::selectedMomentId() const
{
    return m_selectedMoment ? m_selectedMoment->momentId : QString();
}

QString MomentService::selectedTrackId() const
{
    return m_selectedMoment ? m_selectedMoment->trackId : QString();
}

QString MomentService::selectedHeading() const
{
    return m_selectedMoment
        ? headingForPeriod(m_selectedMoment->periodLabel)
        : QString();
}

QString MomentService::selectedTitle() const
{
    return m_selectedMoment ? m_selectedMoment->title : QString();
}

QString MomentService::selectedArtist() const
{
    return m_selectedMoment ? m_selectedMoment->artist : QString();
}

QString MomentService::selectedAlbum() const
{
    return m_selectedMoment ? m_selectedMoment->album : QString();
}

QUrl MomentService::selectedArtworkSource() const
{
    return m_selectedMoment ? QUrl(m_selectedMoment->artworkUrl) : QUrl();
}

QColor MomentService::selectedIdentityColor() const
{
    return resolvedColor(
        m_selectedMoment ? m_selectedMoment->identityColor : QString());
}

QString MomentService::selectedPeriodLabel() const
{
    return m_selectedMoment ? m_selectedMoment->periodLabel : QString();
}

QString MomentService::selectedConfirmedMeaning() const
{
    return m_selectedMoment
        ? m_selectedMoment->confirmedMeaning
        : QString();
}

QString MomentService::selectedCreatedLabel() const
{
    if (!m_selectedMoment || !m_selectedMoment->createdAt.isValid())
        return {};

    return m_selectedMoment->createdAt.toLocalTime().toString(
        QStringLiteral("MMM d · h:mm AP"));
}

bool MomentService::selectedAvailable() const
{
    return !m_selectedPlayablePath.isEmpty();
}

QUrl MomentService::selectedUrl() const
{
    return selectedAvailable()
        ? QUrl::fromLocalFile(m_selectedPlayablePath)
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
    selectMoment(moment.momentId);
    setLastStatus(tr("Moment kept on this device"));
    return true;
}

bool MomentService::selectMoment(const QString &momentId)
{
    const std::optional<MomentRecord> selected =
        m_repository.moment(momentId);
    if (!selected) {
        setErrorString(tr("The selected moment is no longer available."));
        return false;
    }

    setErrorString({});
    setSelectedMoment(
        selected,
        m_repository.resolvedPlayablePath(*selected));
    return true;
}

bool MomentService::updateConfirmedMeaning(
    const QString &momentId,
    const QString &confirmedMeaning)
{
    const QString normalized = confirmedMeaning.trimmed();
    if (!m_repository.updateConfirmedMeaning(momentId, normalized)) {
        setErrorString(m_repository.lastError());
        return false;
    }

    setErrorString({});
    refresh();
    selectMoment(momentId);
    setLastStatus(normalized.isEmpty()
        ? tr("Meaning cleared")
        : tr("Meaning saved"));
    return true;
}

void MomentService::refresh()
{
    const QString previousSelectedId = selectedMomentId();
    const QList<MomentRecord> records = m_repository.moments();

    QVariantList items;
    items.reserve(records.size());
    for (const MomentRecord &moment : records) {
        items.append(momentItem(
            moment,
            m_repository.resolvedPlayablePath(moment)));
    }

    const std::optional<MomentRecord> latest =
        records.isEmpty()
        ? std::optional<MomentRecord> {}
        : std::optional<MomentRecord> { records.first() };
    const QString latestPath = latest
        ? m_repository.resolvedPlayablePath(*latest)
        : QString();

    const bool collectionChanged =
        m_momentCount != records.size()
        || m_momentItems != items
        || m_latestPlayablePath != latestPath;

    m_moments = records;
    m_momentCount = records.size();
    m_momentItems = items;
    m_latestMoment = latest;
    m_latestPlayablePath = latestPath;

    std::optional<MomentRecord> selected;
    QString selectedPath;
    for (const MomentRecord &moment : records) {
        if (moment.momentId == previousSelectedId) {
            selected = moment;
            selectedPath = m_repository.resolvedPlayablePath(moment);
            break;
        }
    }

    if (!selected && latest) {
        selected = latest;
        selectedPath = latestPath;
    }

    setSelectedMoment(selected, selectedPath);

    if (collectionChanged)
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

QString MomentService::headingForPeriod(const QString &period) const
{
    if (period == QStringLiteral("After Midnight"))
        return tr("After midnight");
    if (period == QStringLiteral("Morning"))
        return tr("This morning");
    if (period == QStringLiteral("Afternoon"))
        return tr("This afternoon");
    if (period == QStringLiteral("Evening"))
        return tr("This evening");
    if (period == QStringLiteral("Late Night"))
        return tr("Late tonight");
    return tr("A remembered moment");
}

QVariantMap MomentService::momentItem(
    const MomentRecord &moment,
    const QString &resolvedPlayablePath) const
{
    QVariantMap item;
    item.insert(QStringLiteral("momentId"), moment.momentId);
    item.insert(QStringLiteral("trackId"), moment.trackId);
    item.insert(QStringLiteral("heading"), headingForPeriod(moment.periodLabel));
    item.insert(QStringLiteral("title"), moment.title);
    item.insert(QStringLiteral("artist"), moment.artist);
    item.insert(QStringLiteral("album"), moment.album);
    item.insert(QStringLiteral("artworkSource"), QUrl(moment.artworkUrl));
    item.insert(QStringLiteral("identityColor"),
                resolvedColor(moment.identityColor));
    item.insert(QStringLiteral("periodLabel"), moment.periodLabel);
    item.insert(QStringLiteral("confirmedMeaning"),
                moment.confirmedMeaning);
    item.insert(QStringLiteral("createdLabel"),
                moment.createdAt.toLocalTime().toString(
                    QStringLiteral("MMM d · h:mm AP")));
    item.insert(QStringLiteral("available"),
                !resolvedPlayablePath.isEmpty());
    item.insert(QStringLiteral("url"),
                resolvedPlayablePath.isEmpty()
                    ? QUrl()
                    : QUrl::fromLocalFile(resolvedPlayablePath));
    return item;
}

void MomentService::setSelectedMoment(
    const std::optional<MomentRecord> &moment,
    const QString &resolvedPlayablePath)
{
    const QString oldId = selectedMomentId();
    const QString oldMeaning = selectedConfirmedMeaning();
    const QString oldPath = m_selectedPlayablePath;

    m_selectedMoment = moment;
    m_selectedPlayablePath = resolvedPlayablePath;

    if (oldId != selectedMomentId()
        || oldMeaning != selectedConfirmedMeaning()
        || oldPath != m_selectedPlayablePath) {
        emit selectionChanged();
    }
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
