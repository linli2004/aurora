#include "runtime/audio/AudioRuntime.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QTextStream>
#include <QVariantMap>
#include <QtGlobal>

#include <utility>

#include "runtime/AuroraTypes.h"
#include "runtime/library/LocalLibraryRepository.h"

namespace {
QString libraryDatabasePath()
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty())
        dataLocation = QDir::home().filePath(QStringLiteral(".local/share/Aurora"));

    QDir directory;
    directory.mkpath(dataLocation);
    return QDir(dataLocation).filePath(QStringLiteral("library.sqlite"));
}

QList<QUrl> urlsFromSetting(const QVariant &value)
{
    QList<QUrl> urls;
    const QVariantList values = value.toList();
    urls.reserve(values.size());
    for (const QVariant &entry : values) {
        const QUrl url(entry.toString());
        if (url.isValid() && !url.isEmpty())
            urls.append(url);
    }
    return urls;
}

bool isRemoteAudioSource(const QUrl &url)
{
    const QString scheme = url.scheme().toCaseFolded();
    const QString path = url.path().toCaseFolded();
    return (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"))
        && !url.host().isEmpty()
        && !path.endsWith(QStringLiteral(".js"));
}

bool isPlaylistFile(const QFileInfo &fileInfo)
{
    const QString suffix = fileInfo.suffix().toCaseFolded();
    return suffix == QStringLiteral("m3u") || suffix == QStringLiteral("m3u8");
}

QString urlDedupeKey(const QUrl &url)
{
    return url.adjusted(QUrl::NormalizePathSegments | QUrl::RemovePassword)
        .toString(QUrl::RemovePassword);
}

QString queueMetadataKey(const QUrl &url)
{
    return urlDedupeKey(url);
}

QStringList inputEntriesFromText(const QString &sourceText)
{
    QStringList entries;
    const QStringList lines = sourceText.split(QRegularExpression(QStringLiteral("[\\r\\n]+")),
                                               Qt::SkipEmptyParts);
    const QRegularExpression urlExpression(QStringLiteral("https?://\\S+"));

    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        QRegularExpressionMatchIterator iterator = urlExpression.globalMatch(line);
        QStringList urls;
        while (iterator.hasNext()) {
            QString url = iterator.next().captured(0);
            while (!url.isEmpty() && QStringLiteral("。,.，);）]").contains(url.back()))
                url.chop(1);
            if (!url.isEmpty())
                urls.append(url);
        }

        if (!urls.isEmpty()) {
            entries.append(urls);
            continue;
        }

        entries.append(line);
    }

    entries.removeDuplicates();
    return entries;
}
}

AudioRuntime::AudioRuntime(QObject *parent)
    : QObject(parent)
{
    m_audioOutput.setVolume(0.72F);
    m_player.setAudioOutput(&m_audioOutput);
    m_player.setAudioBufferOutput(&m_audioBufferOutput);
    m_sessionPersistTimer.setSingleShot(true);
    m_sessionPersistTimer.setInterval(2000);
    connect(&m_sessionPersistTimer, &QTimer::timeout,
            this, &AudioRuntime::persistSession);
    m_restorePositionTimer.setSingleShot(true);
    m_restorePositionTimer.setInterval(150);
    connect(&m_restorePositionTimer, &QTimer::timeout,
            this, &AudioRuntime::applyPendingSessionPosition);

    connect(&m_audioBufferOutput, &QAudioBufferOutput::audioBufferReceived,
            &m_featureAnalyzer, &AudioFeatureAnalyzer::processBuffer);
    connect(&m_featureAnalyzer, &AudioFeatureAnalyzer::featuresChanged,
            this, &AudioRuntime::audioFeaturesChanged);
    connect(&m_featureAnalyzer, &AudioFeatureAnalyzer::availabilityChanged,
            this, &AudioRuntime::audioReactiveAvailabilityChanged);
    connect(&m_mediaCache, &MediaCacheService::cacheChanged,
            this, &AudioRuntime::mediaCacheChanged);

    connect(&m_player, &QMediaPlayer::sourceChanged, this, [this] { emit sourceChanged(); });
    connect(&m_player, &QMediaPlayer::durationChanged, this, [this] {
        emit durationChanged();
        applyPendingSessionPosition();
    });
    connect(&m_player, &QMediaPlayer::positionChanged, this,
            [this](qint64 currentPosition) {
        emit positionChanged();

        if (m_pendingRestorePosition >= 0) {
            const qint64 target = qBound<qint64>(
                0, m_pendingRestorePosition,
                duration() > 0 ? duration() : m_pendingRestorePosition);

            if (qAbs(currentPosition - target) <= 1500) {
                m_pendingRestorePosition = -1;
                m_restorePositionAttempts = 0;
                m_restorePositionTimer.stop();
            }
            return;
        }

        scheduleSessionPersist();
    });
    connect(&m_audioOutput, &QAudioOutput::volumeChanged, this, [this] { emit volumeChanged(); });
    connect(&m_player, &QMediaPlayer::seekableChanged, this,
            [this](bool seekable) {
        if (seekable)
            applyPendingSessionPosition();
    });
    connect(&m_player, &QMediaPlayer::metaDataChanged, this, &AudioRuntime::refreshTrackIdentity);

    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, [this] {
        m_featureAnalyzer.setPlaybackActive(playing());
        emit playbackStateChanged();
        emit semanticStateChanged();
    });

    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status) {
                emit mediaStatusChanged();
                emit semanticStateChanged();

                if (status == QMediaPlayer::LoadedMedia
                    || status == QMediaPlayer::BufferedMedia) {
                    refreshTrackIdentity();
                applyPendingSessionPosition();
                }

                if (status == QMediaPlayer::EndOfMedia && m_queue.moveNext()) {
                    emit queueChanged();
                    loadCurrent(true);
                    persistSession();
                }
            });

    connect(&m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &message) {
                m_restorePositionTimer.stop();
                m_restorePositionAttempts = 0;
                m_pendingRestorePosition = -1;
                setErrorString(message.isEmpty() ? tr("Unable to play this audio file.") : message);
                emit semanticStateChanged();
            });

    restoreSession();
}

AudioRuntime::~AudioRuntime()
{
    persistSession();
}

QUrl AudioRuntime::source() const
{
    return m_queue.currentUrl();
}

QString AudioRuntime::trackId() const
{
    return m_trackIdentity.trackId;
}

QString AudioRuntime::sourceId() const
{
    return m_trackIdentity.sourceId;
}

QString AudioRuntime::filePath() const
{
    return m_trackIdentity.filePath;
}

QString AudioRuntime::canonicalTitle() const
{
    return m_trackIdentity.canonicalTitle;
}

QString AudioRuntime::title() const
{
    return m_trackIdentity.title;
}

QString AudioRuntime::artist() const
{
    return m_trackIdentity.artist;
}

QString AudioRuntime::album() const
{
    return m_trackIdentity.album;
}

int AudioRuntime::trackNumber() const
{
    return m_trackIdentity.trackNumber;
}

QUrl AudioRuntime::artworkSource() const
{
    return m_trackIdentity.artworkSource;
}

QColor AudioRuntime::identityColor() const
{
    return m_trackIdentity.identityColor;
}

bool AudioRuntime::identityColorAvailable() const
{
    return m_trackIdentity.identityColor.isValid();
}

bool AudioRuntime::hasEmbeddedArtwork() const
{
    return m_trackIdentity.hasEmbeddedArtwork;
}

bool AudioRuntime::metadataAvailable() const
{
    return m_trackIdentity.metadataAvailable;
}

QString AudioRuntime::availability() const
{
    return m_trackIdentity.availability;
}

QString AudioRuntime::identityProvenance() const
{
    return m_trackIdentity.provenance;
}

qint64 AudioRuntime::duration() const
{
    return m_player.duration();
}

qint64 AudioRuntime::position() const
{
    return m_player.position();
}

qreal AudioRuntime::volume() const
{
    return m_audioOutput.volume();
}

bool AudioRuntime::playing() const
{
    return m_player.playbackState() == QMediaPlayer::PlayingState;
}

bool AudioRuntime::paused() const
{
    return m_player.playbackState() == QMediaPlayer::PausedState;
}

bool AudioRuntime::loading() const
{
    const auto status = m_player.mediaStatus();
    return status == QMediaPlayer::LoadingMedia
        || status == QMediaPlayer::BufferingMedia
        || status == QMediaPlayer::StalledMedia;
}

bool AudioRuntime::hasTrack() const
{
    return !m_queue.isEmpty();
}

int AudioRuntime::currentIndex() const
{
    return m_queue.currentIndex();
}

int AudioRuntime::queueCount() const
{
    return m_queue.count();
}

QString AudioRuntime::errorString() const
{
    return m_errorString;
}

QString AudioRuntime::mediaCacheDirectory() const
{
    return m_mediaCache.directory();
}

qint64 AudioRuntime::mediaCacheBytes() const
{
    return m_mediaCache.sizeBytes();
}

qint64 AudioRuntime::mediaCacheLimitBytes() const
{
    return m_mediaCache.limitBytes();
}

int AudioRuntime::coreExperienceState() const
{
    if (!m_errorString.isEmpty() || !hasTrack())
        return Aurora::CoreIdle;
    if (loading())
        return Aurora::CoreGathering;
    if (playing())
        return Aurora::CorePlaying;
    return Aurora::CorePaused;
}

qreal AudioRuntime::audioLevel() const
{
    return m_featureAnalyzer.level();
}

qreal AudioRuntime::bassEnergy() const
{
    return m_featureAnalyzer.bassEnergy();
}

qreal AudioRuntime::midEnergy() const
{
    return m_featureAnalyzer.midEnergy();
}

qreal AudioRuntime::highEnergy() const
{
    return m_featureAnalyzer.highEnergy();
}

qreal AudioRuntime::transientEnergy() const
{
    return m_featureAnalyzer.transientEnergy();
}

bool AudioRuntime::audioReactiveAvailable() const
{
    return m_featureAnalyzer.available();
}

void AudioRuntime::setQueue(const QVariantList &urls)
{
    const QList<QUrl> filtered = validAudioSources(AudioQueue::fromVariantList(urls));
    if (filtered.isEmpty()) {
        setErrorString(tr("No playable audio sources were selected."));
        return;
    }

    m_queueIdentityOverrides.clear();
    m_queue.setUrls(filtered);
    emit queueChanged();
    loadCurrent(true);
    persistSession();
}

void AudioRuntime::setQueueWithMetadata(const QVariantList &tracks)
{
    QHash<QString, LocalTrackIdentity> identityOverrides;
    const QList<QUrl> urls = urlsFromMetadataTracks(tracks, &identityOverrides);

    if (urls.isEmpty()) {
        setErrorString(tr("No playable online source tracks were resolved."));
        return;
    }

    m_queueIdentityOverrides = identityOverrides;
    m_queue.setUrls(urls);
    emit queueChanged();
    loadCurrent(true);
    persistSession();
}

void AudioRuntime::appendQueueWithMetadata(const QVariantList &tracks)
{
    QHash<QString, LocalTrackIdentity> identityOverrides;
    QList<QUrl> urls = urlsFromMetadataTracks(tracks, &identityOverrides);
    if (urls.isEmpty())
        return;

    QSet<QString> existing;
    for (const QUrl &url : m_queue.urls())
        existing.insert(queueMetadataKey(url));

    qsizetype writeIndex = 0;
    for (const QUrl &url : std::as_const(urls)) {
        if (existing.contains(queueMetadataKey(url)))
            continue;

        existing.insert(queueMetadataKey(url));
        urls[writeIndex++] = url;
        m_queueIdentityOverrides.insert(queueMetadataKey(url),
                                        identityOverrides.value(queueMetadataKey(url)));
    }
    urls.resize(writeIndex);
    if (urls.isEmpty())
        return;

    const bool wasEmpty = m_queue.isEmpty();
    m_queue.appendUrls(urls);
    emit queueChanged();

    if (wasEmpty)
        loadCurrent(true);
    persistSession();
}

void AudioRuntime::appendFiles(const QVariantList &urls)
{
    const QList<QUrl> filtered = validAudioSources(AudioQueue::fromVariantList(urls));
    if (filtered.isEmpty()) {
        setErrorString(tr("No playable audio sources were selected."));
        return;
    }

    m_queueIdentityOverrides.clear();
    const bool wasEmpty = m_queue.isEmpty();
    m_queue.appendUrls(filtered);
    emit queueChanged();

    if (wasEmpty)
        loadCurrent(true);
    persistSession();
}

void AudioRuntime::setQueueFromText(const QString &sourceText)
{
    const QList<QUrl> filtered = validAudioSources(urlsFromSourceText(sourceText));
    if (filtered.isEmpty()) {
        setErrorString(tr("Paste a playable audio URL or M3U playlist content."));
        return;
    }

    m_queueIdentityOverrides.clear();
    m_queue.setUrls(filtered);
    emit queueChanged();
    loadCurrent(true);
    persistSession();
}

void AudioRuntime::appendSourcesFromText(const QString &sourceText)
{
    const QList<QUrl> filtered = validAudioSources(urlsFromSourceText(sourceText));
    if (filtered.isEmpty()) {
        setErrorString(tr("Paste a playable audio URL or M3U playlist content."));
        return;
    }

    m_queueIdentityOverrides.clear();
    const bool wasEmpty = m_queue.isEmpty();
    m_queue.appendUrls(filtered);
    emit queueChanged();

    if (wasEmpty)
        loadCurrent(true);
    persistSession();
}

void AudioRuntime::clearQueue()
{
    m_sessionPersistTimer.stop();
    m_restorePositionTimer.stop();
    m_restorePositionAttempts = 0;
    m_pendingRestorePosition = -1;
    m_featureAnalyzer.reset();
    m_player.stop();
    m_queueIdentityOverrides.clear();
    m_queue.clear();
    m_player.setSource(QUrl());
    applyTrackIdentity({});
    setErrorString({});
    emit queueChanged();
    emit semanticStateChanged();
    persistSession();
}

void AudioRuntime::play()
{
    if (!hasTrack()) {
        setErrorString(tr("Choose local music before playing."));
        return;
    }

    setErrorString({});
    m_player.play();
}

void AudioRuntime::pause()
{
    m_player.pause();
    persistSession();
}

void AudioRuntime::togglePlayback()
{
    if (playing())
        pause();
    else
        play();
}

void AudioRuntime::stop()
{
    m_player.stop();
    persistSession();
}

void AudioRuntime::next()
{
    if (m_queue.moveNext()) {
        emit queueChanged();
        loadCurrent(true);
        persistSession();
    }
}

void AudioRuntime::previous()
{
    if (m_queue.movePrevious()) {
        emit queueChanged();
        loadCurrent(true);
        persistSession();
    }
}

void AudioRuntime::seekRelative(qint64 deltaMilliseconds)
{
    setPosition(position() + deltaMilliseconds);
}

void AudioRuntime::clearMediaCache()
{
    m_mediaCache.clear();
}

void AudioRuntime::setPosition(qint64 position)
{
    const qint64 upperBound = duration() > 0 ? duration() : position;
    m_player.setPosition(qBound<qint64>(0, position, upperBound));
}

void AudioRuntime::setVolume(qreal volume)
{
    m_audioOutput.setVolume(qBound<qreal>(0.0, volume, 1.0));
    persistSession();
}

QList<QUrl> AudioRuntime::validAudioSources(const QList<QUrl> &urls) const
{
    QList<QUrl> result;
    result.reserve(urls.size());
    QSet<QString> seen;
    QSet<QString> expandingPlaylists;

    const auto appendUnique = [&seen, &result](const QUrl &url) {
        const QString key = urlDedupeKey(url);
        if (seen.contains(key))
            return;

        seen.insert(key);
        result.append(url);
    };

    const auto appendValid = [this, &appendUnique, &expandingPlaylists](
        const QList<QUrl> &candidates,
        const QString &relativeBase,
        const auto &appendValidRef) -> void {
        for (const QUrl &url : candidates) {
            QUrl resolvedUrl = url;
            if (resolvedUrl.isRelative() && !relativeBase.isEmpty())
                resolvedUrl = QUrl::fromLocalFile(QDir(relativeBase).filePath(resolvedUrl.toString()));

            if (isRemoteAudioSource(resolvedUrl)) {
                appendUnique(resolvedUrl);
                continue;
            }

            const QFileInfo fileInfo(resolvedUrl.toLocalFile());
            if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable())
                continue;

            if (isPlaylistFile(fileInfo)) {
                const QString playlistPath = QDir::cleanPath(fileInfo.canonicalFilePath());
                if (playlistPath.isEmpty() || expandingPlaylists.contains(playlistPath))
                    continue;

                QFile file(playlistPath);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    continue;

                expandingPlaylists.insert(playlistPath);
                QTextStream stream(&file);
                appendValidRef(urlsFromSourceText(stream.readAll()),
                               QFileInfo(playlistPath).absolutePath(),
                               appendValidRef);
                expandingPlaylists.remove(playlistPath);
                continue;
            }

            appendUnique(QUrl::fromLocalFile(QDir::cleanPath(fileInfo.canonicalFilePath())));
        }
    };

    appendValid(urls, QString(), appendValid);

    return result;
}

QList<QUrl> AudioRuntime::urlsFromSourceText(const QString &sourceText) const
{
    QList<QUrl> urls;
    const QStringList entries = inputEntriesFromText(sourceText);
    urls.reserve(entries.size());

    for (const QString &entry : entries) {
        const QFileInfo localPath(entry);
        if (localPath.isAbsolute()) {
            urls.append(QUrl::fromLocalFile(entry));
            continue;
        }

        const QUrl rawUrl(entry);
        if (rawUrl.isRelative()) {
            urls.append(rawUrl);
            continue;
        }

        const QUrl url = QUrl::fromUserInput(entry);
        if (url.isValid() && !url.isEmpty())
            urls.append(url);
    }

    return urls;
}

QList<QUrl> AudioRuntime::urlsFromMetadataTracks(
    const QVariantList &tracks,
    QHash<QString, LocalTrackIdentity> *identityOverrides) const
{
    QList<QUrl> urls;
    urls.reserve(tracks.size());
    QSet<QString> seen;

    for (const QVariant &trackValue : tracks) {
        const QVariantMap track = trackValue.toMap();
        const QUrl url = QUrl::fromUserInput(track.value(QStringLiteral("url")).toString());
        if (!url.isValid() || url.isEmpty())
            continue;

        const QList<QUrl> filtered = validAudioSources({url});
        if (filtered.isEmpty())
            continue;

        const QUrl playableUrl = filtered.first();
        const QString key = queueMetadataKey(playableUrl);
        if (seen.contains(key))
            continue;

        seen.insert(key);
        urls.append(playableUrl);

        LocalTrackIdentity identity = LocalTrackIdentityResolver::fallbackFor(playableUrl);
        const QString title = track.value(QStringLiteral("title")).toString().simplified();
        const QString artist = track.value(QStringLiteral("artist")).toString().simplified();
        const QString album = track.value(QStringLiteral("album")).toString().simplified();
        const QUrl artworkUrl = QUrl::fromUserInput(track.value(QStringLiteral("artworkUrl")).toString());

        if (!title.isEmpty())
            identity.title = title;
        if (!artist.isEmpty())
            identity.artist = artist;
        if (!album.isEmpty())
            identity.album = album;
        if (artworkUrl.isValid() && !artworkUrl.isEmpty())
            identity.artworkSource = artworkUrl;

        identity.canonicalTitle = identity.title.toCaseFolded();
        identity.metadataAvailable = true;
        identity.provenance = QStringLiteral("Online source catalog · Resolver metadata");
        if (identityOverrides)
            identityOverrides->insert(key, identity);
    }

    return urls;
}

void AudioRuntime::loadCurrent(bool autoplay, qint64 initialPosition)
{
    const QUrl url = m_queue.currentUrl();
    if (url.isEmpty())
        return;

    m_sessionPersistTimer.stop();
    m_restorePositionTimer.stop();
    m_restorePositionAttempts = 0;
    m_pendingRestorePosition = initialPosition;
    setErrorString({});
    m_featureAnalyzer.reset();
    m_player.stop();
    const LocalTrackIdentity overrideIdentity = identityOverrideFor(url);
    applyTrackIdentity(overrideIdentity.trackId.isEmpty()
                           ? LocalTrackIdentityResolver::fallbackFor(url)
                           : overrideIdentity);
    const QUrl playbackSource = m_mediaCache.playbackSource(url);
    m_player.setSource(playbackSource);
    m_mediaCache.warm(url);
    emit semanticStateChanged();

    if (autoplay)
        m_player.play();
}

void AudioRuntime::applyPendingSessionPosition()
{
    if (m_pendingRestorePosition < 0)
        return;

    if (duration() <= 0 || !m_player.isSeekable())
        return;

    const qint64 restoredPosition = qBound<qint64>(
        0, m_pendingRestorePosition, duration());

    if (qAbs(position() - restoredPosition) <= 1500) {
        m_pendingRestorePosition = -1;
        m_restorePositionAttempts = 0;
        m_restorePositionTimer.stop();
        return;
    }

    if (m_restorePositionAttempts >= 20) {
        m_pendingRestorePosition = -1;
        m_restorePositionAttempts = 0;
        m_restorePositionTimer.stop();
        return;
    }

    ++m_restorePositionAttempts;
    m_player.setPosition(restoredPosition);

    if (m_pendingRestorePosition >= 0)
        m_restorePositionTimer.start();
}

void AudioRuntime::restoreSession()
{
    m_queueIdentityOverrides.clear();
    LocalLibraryRepository repository;
    if (!repository.open(libraryDatabasePath()))
        return;

    const QVariantMap session = repository.setting(QStringLiteral("playback.session.v1"));
    const QList<QUrl> urls = validAudioSources(urlsFromSetting(session.value(QStringLiteral("urls"))));
    if (urls.isEmpty())
        return;

    m_audioOutput.setVolume(qBound<qreal>(
        0.0,
        session.value(QStringLiteral("volume"), m_audioOutput.volume()).toReal(),
        1.0));
    m_queue.setUrls(urls);
    m_queue.moveTo(session.value(QStringLiteral("currentIndex"), 0).toInt());
    emit queueChanged();
    loadCurrent(false, qMax<qint64>(
        0, session.value(QStringLiteral("positionMs"), 0).toLongLong()));
}

LocalTrackIdentity AudioRuntime::identityOverrideFor(const QUrl &url) const
{
    const auto iterator = m_queueIdentityOverrides.constFind(queueMetadataKey(url));
    return iterator == m_queueIdentityOverrides.cend() ? LocalTrackIdentity{} : iterator.value();
}

void AudioRuntime::persistSession()
{
    m_sessionPersistTimer.stop();
    if (m_pendingRestorePosition >= 0)
        return;
    LocalLibraryRepository repository;
    if (!repository.open(libraryDatabasePath()))
        return;

    QVariantList urls;
    urls.reserve(m_queue.urls().size());
    for (const QUrl &url : m_queue.urls())
        urls.append(url.toString());

    QVariantMap session;
    session.insert(QStringLiteral("urls"), urls);
    session.insert(QStringLiteral("currentIndex"), m_queue.currentIndex());
    session.insert(QStringLiteral("volume"), m_audioOutput.volume());
    session.insert(QStringLiteral("positionMs"), position());
    repository.setSetting(QStringLiteral("playback.session.v1"), session);
}

void AudioRuntime::scheduleSessionPersist()
{
    if (m_queue.isEmpty()
        || m_pendingRestorePosition >= 0
        || m_sessionPersistTimer.isActive()) {
        return;
    }

    m_sessionPersistTimer.start();
}

void AudioRuntime::applyTrackIdentity(const LocalTrackIdentity &identity)
{
    if (m_trackIdentity == identity)
        return;

    m_trackIdentity = identity;
    emit trackChanged();
}

void AudioRuntime::refreshTrackIdentity()
{
    if (!hasTrack())
        return;

    LocalTrackIdentity identity = LocalTrackIdentityResolver::resolve(
        m_queue.currentUrl(),
        m_player.metaData());
    const LocalTrackIdentity overrideIdentity = identityOverrideFor(m_queue.currentUrl());
    if (!overrideIdentity.trackId.isEmpty()) {
        identity.title = overrideIdentity.title;
        identity.artist = overrideIdentity.artist;
        identity.album = overrideIdentity.album;
        identity.canonicalTitle = overrideIdentity.canonicalTitle;
        identity.artworkSource = overrideIdentity.artworkSource.isEmpty()
            ? identity.artworkSource
            : overrideIdentity.artworkSource;
        identity.metadataAvailable = true;
        identity.provenance = overrideIdentity.provenance;
    }

    applyTrackIdentity(identity);
}

void AudioRuntime::setErrorString(const QString &message)
{
    if (m_errorString == message)
        return;

    m_errorString = message;
    emit errorChanged();
}
