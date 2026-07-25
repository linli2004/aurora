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

bool isNetworkAudioUrl(const QUrl &url)
{
    const QString scheme = url.scheme().toCaseFolded();
    return (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"))
        && !url.host().isEmpty();
}

QUrl normalizedNetworkAudioUrl(const QUrl &source)
{
    if (!isNetworkAudioUrl(source))
        return {};

    return source.adjusted(QUrl::NormalizePathSegments | QUrl::RemovePassword);
}

QString normalizedMomentSource(const QUrl &source)
{
    if (source.isLocalFile())
        return normalizedPath(source.toLocalFile());

    const QUrl networkUrl = normalizedNetworkAudioUrl(source);
    return networkUrl.isEmpty()
        ? QString()
        : networkUrl.toString(QUrl::RemovePassword);
}

QString titleFallbackForSource(
    const QUrl &source,
    const QString &storedSource)
{
    if (source.isLocalFile())
        return QFileInfo(source.toLocalFile()).completeBaseName();

    const QUrl networkUrl = normalizedNetworkAudioUrl(source);
    if (!networkUrl.isEmpty()) {
        const QString sourceName = QFileInfo(networkUrl.path()).completeBaseName();
        return sourceName.isEmpty() ? networkUrl.host() : sourceName;
    }

    return QFileInfo(storedSource).completeBaseName();
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
    return !m_latestPlayableUrl.isEmpty();
}

QUrl MomentService::latestUrl() const
{
    return m_latestPlayableUrl;
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
    return !m_selectedPlayableUrl.isEmpty();
}

QUrl MomentService::selectedUrl() const
{
    return m_selectedPlayableUrl;
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
    const QUrl &sourceUrl,
    const QString &title,
    const QString &artist,
    const QString &album,
    const QUrl &artworkSource,
    const QColor &identityColor,
    const QString &confirmedMeaning)
{
    const QString storedSource = normalizedMomentSource(sourceUrl);
    if (trackId.trimmed().isEmpty() || storedSource.isEmpty()) {
        setErrorString(tr("Load a track before keeping a moment."));
        return false;
    }

    setErrorString({});

    MomentRecord moment;
    moment.momentId = QStringLiteral("moment:%1").arg(
        QUuid::createUuid().toString(QUuid::Id128));
    moment.trackId = trackId;
    moment.sourceId = sourceId;
    moment.sourcePath = storedSource;
    moment.title = title.trimmed().isEmpty()
        ? titleFallbackForSource(sourceUrl, storedSource)
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
        m_repository.resolvedPlayableUrl(*selected));
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
            m_repository.resolvedPlayableUrl(moment)));
    }

    const std::optional<MomentRecord> latest =
        records.isEmpty()
        ? std::optional<MomentRecord> {}
        : std::optional<MomentRecord> { records.first() };
    const QUrl latestUrl = latest
        ? m_repository.resolvedPlayableUrl(*latest)
        : QUrl();

    const bool collectionChanged =
        m_momentCount != records.size()
        || m_momentItems != items
        || m_latestPlayableUrl != latestUrl;

    m_moments = records;
    m_momentCount = records.size();
    m_momentItems = items;
    m_latestMoment = latest;
    m_latestPlayableUrl = latestUrl;

    std::optional<MomentRecord> selected;
    QUrl selectedUrl;
    for (const MomentRecord &moment : records) {
        if (moment.momentId == previousSelectedId) {
            selected = moment;
            selectedUrl = m_repository.resolvedPlayableUrl(moment);
            break;
        }
    }

    if (!selected && latest) {
        selected = latest;
        selectedUrl = latestUrl;
    }

    setSelectedMoment(selected, selectedUrl);

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
    const QUrl &resolvedPlayableUrl) const
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
                !resolvedPlayableUrl.isEmpty());
    item.insert(QStringLiteral("url"), resolvedPlayableUrl);
    return item;
}

void MomentService::setSelectedMoment(
    const std::optional<MomentRecord> &moment,
    const QUrl &resolvedPlayableUrl)
{
    const QString oldId = selectedMomentId();
    const QString oldMeaning = selectedConfirmedMeaning();
    const QUrl oldUrl = m_selectedPlayableUrl;

    m_selectedMoment = moment;
    m_selectedPlayableUrl = resolvedPlayableUrl;

    if (oldId != selectedMomentId()
        || oldMeaning != selectedConfirmedMeaning()
        || oldUrl != m_selectedPlayableUrl) {
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
