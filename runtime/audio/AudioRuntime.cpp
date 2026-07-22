#include "runtime/audio/AudioRuntime.h"

#include <QFileInfo>
#include <QtGlobal>

#include "runtime/AuroraTypes.h"

AudioRuntime::AudioRuntime(QObject *parent)
    : QObject(parent)
{
    m_audioOutput.setVolume(0.72F);
    m_player.setAudioOutput(&m_audioOutput);
    m_player.setAudioBufferOutput(&m_audioBufferOutput);

    connect(&m_audioBufferOutput, &QAudioBufferOutput::audioBufferReceived,
            &m_featureAnalyzer, &AudioFeatureAnalyzer::processBuffer);
    connect(&m_featureAnalyzer, &AudioFeatureAnalyzer::featuresChanged,
            this, &AudioRuntime::audioFeaturesChanged);
    connect(&m_featureAnalyzer, &AudioFeatureAnalyzer::availabilityChanged,
            this, &AudioRuntime::audioReactiveAvailabilityChanged);

    connect(&m_player, &QMediaPlayer::sourceChanged, this, [this] { emit sourceChanged(); });
    connect(&m_player, &QMediaPlayer::durationChanged, this, [this] { emit durationChanged(); });
    connect(&m_player, &QMediaPlayer::positionChanged, this, [this] { emit positionChanged(); });
    connect(&m_audioOutput, &QAudioOutput::volumeChanged, this, [this] { emit volumeChanged(); });
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
                }

                if (status == QMediaPlayer::EndOfMedia && m_queue.moveNext()) {
                    emit queueChanged();
                    loadCurrent(true);
                }
            });

    connect(&m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &message) {
                setErrorString(message.isEmpty() ? tr("Unable to play this audio file.") : message);
                emit semanticStateChanged();
            });
}

QUrl AudioRuntime::source() const
{
    return m_player.source();
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
    const QList<QUrl> filtered = validLocalFiles(AudioQueue::fromVariantList(urls));
    if (filtered.isEmpty()) {
        setErrorString(tr("No readable local audio files were selected."));
        return;
    }

    m_queue.setUrls(filtered);
    emit queueChanged();
    loadCurrent(true);
}

void AudioRuntime::appendFiles(const QVariantList &urls)
{
    const QList<QUrl> filtered = validLocalFiles(AudioQueue::fromVariantList(urls));
    if (filtered.isEmpty()) {
        setErrorString(tr("No readable local audio files were selected."));
        return;
    }

    const bool wasEmpty = m_queue.isEmpty();
    m_queue.appendUrls(filtered);
    emit queueChanged();

    if (wasEmpty)
        loadCurrent(true);
}

void AudioRuntime::clearQueue()
{
    m_featureAnalyzer.reset();
    m_player.stop();
    m_player.setSource(QUrl());
    m_queue.clear();
    applyTrackIdentity({});
    setErrorString({});
    emit queueChanged();
    emit semanticStateChanged();
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
}

void AudioRuntime::next()
{
    if (m_queue.moveNext()) {
        emit queueChanged();
        loadCurrent(true);
    }
}

void AudioRuntime::previous()
{
    if (m_queue.movePrevious()) {
        emit queueChanged();
        loadCurrent(true);
    }
}

void AudioRuntime::seekRelative(qint64 deltaMilliseconds)
{
    setPosition(position() + deltaMilliseconds);
}

void AudioRuntime::setPosition(qint64 position)
{
    const qint64 upperBound = duration() > 0 ? duration() : position;
    m_player.setPosition(qBound<qint64>(0, position, upperBound));
}

void AudioRuntime::setVolume(qreal volume)
{
    m_audioOutput.setVolume(qBound<qreal>(0.0, volume, 1.0));
}

QList<QUrl> AudioRuntime::validLocalFiles(const QList<QUrl> &urls) const
{
    QList<QUrl> result;
    result.reserve(urls.size());

    for (const QUrl &url : urls) {
        if (!url.isLocalFile())
            continue;

        const QFileInfo fileInfo(url.toLocalFile());
        if (fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable())
            result.append(QUrl::fromLocalFile(fileInfo.canonicalFilePath()));
    }

    return result;
}

void AudioRuntime::loadCurrent(bool autoplay)
{
    const QUrl url = m_queue.currentUrl();
    if (url.isEmpty())
        return;

    setErrorString({});
    m_featureAnalyzer.reset();
    m_player.stop();
    applyTrackIdentity(LocalTrackIdentityResolver::fallbackFor(url));
    m_player.setSource(url);
    emit semanticStateChanged();

    if (autoplay)
        m_player.play();
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

    applyTrackIdentity(LocalTrackIdentityResolver::resolve(
        m_queue.currentUrl(),
        m_player.metaData()));
}

void AudioRuntime::setErrorString(const QString &message)
{
    if (m_errorString == message)
        return;

    m_errorString = message;
    emit errorChanged();
}
