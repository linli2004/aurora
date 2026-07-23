#pragma once

#include <QObject>
#include <QUrl>
#include <QVariantList>

#include <QAudioBufferOutput>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QTimer>

#include "runtime/audio/AudioFeatureAnalyzer.h"
#include "runtime/audio/AudioQueue.h"
#include "runtime/audio/LocalTrackIdentity.h"

class AudioRuntime final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged)
    Q_PROPERTY(QString trackId READ trackId NOTIFY trackChanged)
    Q_PROPERTY(QString sourceId READ sourceId NOTIFY trackChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY trackChanged)
    Q_PROPERTY(QString canonicalTitle READ canonicalTitle NOTIFY trackChanged)
    Q_PROPERTY(QString title READ title NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY trackChanged)
    Q_PROPERTY(QString album READ album NOTIFY trackChanged)
    Q_PROPERTY(int trackNumber READ trackNumber NOTIFY trackChanged)
    Q_PROPERTY(QUrl artworkSource READ artworkSource NOTIFY trackChanged)
    Q_PROPERTY(QColor identityColor READ identityColor NOTIFY trackChanged)
    Q_PROPERTY(bool identityColorAvailable READ identityColorAvailable NOTIFY trackChanged)
    Q_PROPERTY(bool hasEmbeddedArtwork READ hasEmbeddedArtwork NOTIFY trackChanged)
    Q_PROPERTY(bool metadataAvailable READ metadataAvailable NOTIFY trackChanged)
    Q_PROPERTY(QString availability READ availability NOTIFY trackChanged)
    Q_PROPERTY(QString identityProvenance READ identityProvenance NOTIFY trackChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playbackStateChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY playbackStateChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY mediaStatusChanged)
    Q_PROPERTY(bool hasTrack READ hasTrack NOTIFY queueChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY queueChanged)
    Q_PROPERTY(int queueCount READ queueCount NOTIFY queueChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(int coreExperienceState READ coreExperienceState NOTIFY semanticStateChanged)
    Q_PROPERTY(qreal audioLevel READ audioLevel NOTIFY audioFeaturesChanged)
    Q_PROPERTY(qreal bassEnergy READ bassEnergy NOTIFY audioFeaturesChanged)
    Q_PROPERTY(qreal midEnergy READ midEnergy NOTIFY audioFeaturesChanged)
    Q_PROPERTY(qreal highEnergy READ highEnergy NOTIFY audioFeaturesChanged)
    Q_PROPERTY(qreal transientEnergy READ transientEnergy NOTIFY audioFeaturesChanged)
    Q_PROPERTY(bool audioReactiveAvailable READ audioReactiveAvailable NOTIFY audioReactiveAvailabilityChanged)

public:
    explicit AudioRuntime(QObject *parent = nullptr);
    ~AudioRuntime() override;

    [[nodiscard]] QUrl source() const;
    [[nodiscard]] QString trackId() const;
    [[nodiscard]] QString sourceId() const;
    [[nodiscard]] QString filePath() const;
    [[nodiscard]] QString canonicalTitle() const;
    [[nodiscard]] QString title() const;
    [[nodiscard]] QString artist() const;
    [[nodiscard]] QString album() const;
    [[nodiscard]] int trackNumber() const;
    [[nodiscard]] QUrl artworkSource() const;
    [[nodiscard]] QColor identityColor() const;
    [[nodiscard]] bool identityColorAvailable() const;
    [[nodiscard]] bool hasEmbeddedArtwork() const;
    [[nodiscard]] bool metadataAvailable() const;
    [[nodiscard]] QString availability() const;
    [[nodiscard]] QString identityProvenance() const;
    [[nodiscard]] qint64 duration() const;
    [[nodiscard]] qint64 position() const;
    [[nodiscard]] qreal volume() const;
    [[nodiscard]] bool playing() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] bool hasTrack() const;
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] int queueCount() const;
    [[nodiscard]] QString errorString() const;
    [[nodiscard]] int coreExperienceState() const;
    [[nodiscard]] qreal audioLevel() const;
    [[nodiscard]] qreal bassEnergy() const;
    [[nodiscard]] qreal midEnergy() const;
    [[nodiscard]] qreal highEnergy() const;
    [[nodiscard]] qreal transientEnergy() const;
    [[nodiscard]] bool audioReactiveAvailable() const;

    Q_INVOKABLE void setQueue(const QVariantList &urls);
    Q_INVOKABLE void appendFiles(const QVariantList &urls);
    Q_INVOKABLE void clearQueue();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seekRelative(qint64 deltaMilliseconds);

public slots:
    void setPosition(qint64 position);
    void setVolume(qreal volume);

signals:
    void sourceChanged();
    void trackChanged();
    void durationChanged();
    void positionChanged();
    void volumeChanged();
    void playbackStateChanged();
    void mediaStatusChanged();
    void queueChanged();
    void errorChanged();
    void semanticStateChanged();
    void audioFeaturesChanged();
    void audioReactiveAvailabilityChanged();

private:
    QList<QUrl> validLocalFiles(const QList<QUrl> &urls) const;
    void loadCurrent(bool autoplay, qint64 initialPosition = -1);
    void applyPendingSessionPosition();
    void applyTrackIdentity(const LocalTrackIdentity &identity);
    void refreshTrackIdentity();
    void restoreSession();
    void persistSession();
    void scheduleSessionPersist();
    void setErrorString(const QString &message);

    QMediaPlayer m_player;
    QAudioOutput m_audioOutput;
    QAudioBufferOutput m_audioBufferOutput;
    AudioFeatureAnalyzer m_featureAnalyzer;
    QTimer m_sessionPersistTimer;
    QTimer m_restorePositionTimer;
    AudioQueue m_queue;
    LocalTrackIdentity m_trackIdentity;
    QString m_errorString;
    qint64 m_pendingRestorePosition = -1;
    int m_restorePositionAttempts = 0;
};
