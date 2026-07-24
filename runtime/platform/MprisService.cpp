#include "runtime/platform/MprisService.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>

#include "runtime/audio/AudioRuntime.h"

namespace {

constexpr auto kMprisServiceName = "org.mpris.MediaPlayer2.aurora";
constexpr auto kMprisObjectPath = "/org/mpris/MediaPlayer2";
constexpr auto kMprisPlayerInterface = "org.mpris.MediaPlayer2.Player";
constexpr auto kDbusPropertiesInterface = "org.freedesktop.DBus.Properties";
constexpr auto kCurrentTrackObjectPath = "/org/mpris/MediaPlayer2/Track/current";
constexpr auto kNoTrackObjectPath = "/org/mpris/MediaPlayer2/TrackList/NoTrack";

qlonglong toMprisTime(qint64 milliseconds)
{
    return qMax<qlonglong>(0, milliseconds) * 1000;
}

qint64 fromMprisTime(qlonglong microseconds)
{
    return qMax<qint64>(0, microseconds / 1000);
}

QString playbackStatus(const AudioRuntime *audioRuntime)
{
    if (!audioRuntime || !audioRuntime->hasTrack())
        return QStringLiteral("Stopped");

    if (audioRuntime->playing())
        return QStringLiteral("Playing");

    return QStringLiteral("Paused");
}

QDBusObjectPath trackObjectPath(const AudioRuntime *audioRuntime)
{
    if (!audioRuntime || !audioRuntime->hasTrack())
        return QDBusObjectPath(QString::fromLatin1(kNoTrackObjectPath));

    return QDBusObjectPath(QString::fromLatin1(kCurrentTrackObjectPath));
}

QVariantMap metadataFor(const AudioRuntime *audioRuntime)
{
    QVariantMap metadata;
    metadata.insert(QStringLiteral("mpris:trackid"),
                    QVariant::fromValue(trackObjectPath(audioRuntime)));

    if (!audioRuntime || !audioRuntime->hasTrack())
        return metadata;

    const QString title = audioRuntime->title().trimmed();
    metadata.insert(QStringLiteral("xesam:title"),
                    title.isEmpty() ? QObject::tr("Unknown track") : title);

    const QString artist = audioRuntime->artist().trimmed();
    if (!artist.isEmpty())
        metadata.insert(QStringLiteral("xesam:artist"), QStringList{artist});

    const QString album = audioRuntime->album().trimmed();
    if (!album.isEmpty())
        metadata.insert(QStringLiteral("xesam:album"), album);

    if (audioRuntime->trackNumber() > 0)
        metadata.insert(QStringLiteral("xesam:trackNumber"), audioRuntime->trackNumber());

    if (audioRuntime->duration() > 0)
        metadata.insert(QStringLiteral("mpris:length"), toMprisTime(audioRuntime->duration()));

    const QUrl artwork = audioRuntime->artworkSource();
    if (artwork.isValid() && !artwork.isEmpty())
        metadata.insert(QStringLiteral("mpris:artUrl"), artwork.toString());

    const QString filePath = audioRuntime->filePath();
    if (!filePath.isEmpty())
        metadata.insert(QStringLiteral("xesam:url"), QUrl::fromLocalFile(filePath).toString());

    return metadata;
}

class MprisRootAdaptor final : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_PROPERTY(bool CanQuit READ CanQuit)
    Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE SetFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)
    Q_PROPERTY(bool CanRaise READ CanRaise)
    Q_PROPERTY(bool HasTrackList READ HasTrackList)
    Q_PROPERTY(QString Identity READ Identity)
    Q_PROPERTY(QString DesktopEntry READ DesktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)

public:
    explicit MprisRootAdaptor(QObject *parent)
        : QDBusAbstractAdaptor(parent)
    {
        setAutoRelaySignals(true);
    }

    [[nodiscard]] bool CanQuit() const { return false; }
    [[nodiscard]] bool Fullscreen() const { return false; }
    void SetFullscreen(bool fullscreen) { Q_UNUSED(fullscreen); }
    [[nodiscard]] bool CanSetFullscreen() const { return false; }
    [[nodiscard]] bool CanRaise() const { return false; }
    [[nodiscard]] bool HasTrackList() const { return false; }
    [[nodiscard]] QString Identity() const { return QStringLiteral("Aurora"); }
    [[nodiscard]] QString DesktopEntry() const { return QStringLiteral("aurora"); }

    [[nodiscard]] QStringList SupportedUriSchemes() const
    {
        return {QStringLiteral("file")};
    }

    [[nodiscard]] QStringList SupportedMimeTypes() const
    {
        return {
            QStringLiteral("audio/mpeg"),
            QStringLiteral("audio/mp4"),
            QStringLiteral("audio/flac"),
            QStringLiteral("audio/ogg"),
            QStringLiteral("audio/x-wav"),
            QStringLiteral("audio/wav")
        };
    }

public slots:
    void Raise() {}
    void Quit() {}
};

class MprisPlayerAdaptor final : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
    Q_PROPERTY(QString LoopStatus READ LoopStatus WRITE SetLoopStatus)
    Q_PROPERTY(double Rate READ Rate WRITE SetRate)
    Q_PROPERTY(bool Shuffle READ Shuffle WRITE SetShuffle)
    Q_PROPERTY(QVariantMap Metadata READ Metadata)
    Q_PROPERTY(double Volume READ Volume WRITE SetVolume)
    Q_PROPERTY(qlonglong Position READ Position)
    Q_PROPERTY(double MinimumRate READ MinimumRate)
    Q_PROPERTY(double MaximumRate READ MaximumRate)
    Q_PROPERTY(bool CanGoNext READ CanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
    Q_PROPERTY(bool CanPlay READ CanPlay)
    Q_PROPERTY(bool CanPause READ CanPause)
    Q_PROPERTY(bool CanSeek READ CanSeek)
    Q_PROPERTY(bool CanControl READ CanControl)

public:
    explicit MprisPlayerAdaptor(AudioRuntime *audioRuntime, QObject *parent)
        : QDBusAbstractAdaptor(parent)
        , m_audioRuntime(audioRuntime)
    {
        setAutoRelaySignals(true);
    }

    [[nodiscard]] QString PlaybackStatus() const
    {
        return playbackStatus(m_audioRuntime);
    }

    [[nodiscard]] QString LoopStatus() const
    {
        return QStringLiteral("Playlist");
    }

    void SetLoopStatus(const QString &loopStatus)
    {
        Q_UNUSED(loopStatus);
    }

    [[nodiscard]] double Rate() const { return 1.0; }
    void SetRate(double rate) { Q_UNUSED(rate); }

    [[nodiscard]] bool Shuffle() const { return false; }
    void SetShuffle(bool shuffle) { Q_UNUSED(shuffle); }

    [[nodiscard]] QVariantMap Metadata() const
    {
        return metadataFor(m_audioRuntime);
    }

    [[nodiscard]] double Volume() const
    {
        return m_audioRuntime ? m_audioRuntime->volume() : 0.0;
    }

    void SetVolume(double volume)
    {
        if (m_audioRuntime)
            m_audioRuntime->setVolume(volume);
    }

    [[nodiscard]] qlonglong Position() const
    {
        return m_audioRuntime ? toMprisTime(m_audioRuntime->position()) : 0;
    }

    [[nodiscard]] double MinimumRate() const { return 1.0; }
    [[nodiscard]] double MaximumRate() const { return 1.0; }

    [[nodiscard]] bool CanGoNext() const
    {
        return m_audioRuntime && m_audioRuntime->queueCount() > 1;
    }

    [[nodiscard]] bool CanGoPrevious() const
    {
        return m_audioRuntime && m_audioRuntime->queueCount() > 1;
    }

    [[nodiscard]] bool CanPlay() const
    {
        return m_audioRuntime && m_audioRuntime->hasTrack();
    }

    [[nodiscard]] bool CanPause() const
    {
        return m_audioRuntime && m_audioRuntime->hasTrack();
    }

    [[nodiscard]] bool CanSeek() const
    {
        return m_audioRuntime && m_audioRuntime->hasTrack() && m_audioRuntime->duration() > 0;
    }

    [[nodiscard]] bool CanControl() const { return true; }

public slots:
    void Next()
    {
        if (m_audioRuntime)
            m_audioRuntime->next();
    }

    void Previous()
    {
        if (m_audioRuntime)
            m_audioRuntime->previous();
    }

    void Pause()
    {
        if (m_audioRuntime)
            m_audioRuntime->pause();
    }

    void PlayPause()
    {
        if (m_audioRuntime)
            m_audioRuntime->togglePlayback();
    }

    void Stop()
    {
        if (m_audioRuntime)
            m_audioRuntime->stop();
    }

    void Play()
    {
        if (m_audioRuntime)
            m_audioRuntime->play();
    }

    void Seek(qlonglong offset)
    {
        if (!m_audioRuntime || !CanSeek())
            return;

        m_audioRuntime->setPosition(m_audioRuntime->position() + fromMprisTime(offset));
        emit Seeked(toMprisTime(m_audioRuntime->position()));
    }

    void SetPosition(const QDBusObjectPath &trackId, qlonglong position)
    {
        if (!m_audioRuntime || !CanSeek())
            return;

        if (trackId.path() != QString::fromLatin1(kCurrentTrackObjectPath))
            return;

        m_audioRuntime->setPosition(fromMprisTime(position));
        emit Seeked(toMprisTime(m_audioRuntime->position()));
    }

    void OpenUri(const QString &uri)
    {
        Q_UNUSED(uri);
    }

signals:
    void Seeked(qlonglong position);

private:
    AudioRuntime *m_audioRuntime = nullptr;
};

} // namespace

MprisService::MprisService(AudioRuntime *audioRuntime, QObject *parent)
    : QObject(parent)
    , m_audioRuntime(audioRuntime)
{
    new MprisRootAdaptor(this);
    new MprisPlayerAdaptor(audioRuntime, this);

    registerService();

    if (!m_audioRuntime)
        return;

    connect(m_audioRuntime, &AudioRuntime::playbackStateChanged, this, [this] {
        emitPropertiesChanged(QString::fromLatin1(kMprisPlayerInterface),
                              {{QStringLiteral("PlaybackStatus"), playbackStatus(m_audioRuntime)}});
    });

    connect(m_audioRuntime, &AudioRuntime::trackChanged, this, [this] {
        emitPropertiesChanged(QString::fromLatin1(kMprisPlayerInterface),
                              {{QStringLiteral("Metadata"), metadataFor(m_audioRuntime)}});
    });

    connect(m_audioRuntime, &AudioRuntime::durationChanged, this, [this] {
        emitPropertiesChanged(QString::fromLatin1(kMprisPlayerInterface),
                              {{QStringLiteral("Metadata"), metadataFor(m_audioRuntime)},
                               {QStringLiteral("CanSeek"),
                                m_audioRuntime && m_audioRuntime->hasTrack() && m_audioRuntime->duration() > 0}});
    });

    connect(m_audioRuntime, &AudioRuntime::queueChanged, this, [this] {
        emitPropertiesChanged(QString::fromLatin1(kMprisPlayerInterface),
                              {{QStringLiteral("CanGoNext"), m_audioRuntime && m_audioRuntime->queueCount() > 1},
                               {QStringLiteral("CanGoPrevious"), m_audioRuntime && m_audioRuntime->queueCount() > 1},
                               {QStringLiteral("CanPlay"), m_audioRuntime && m_audioRuntime->hasTrack()},
                               {QStringLiteral("CanPause"), m_audioRuntime && m_audioRuntime->hasTrack()},
                               {QStringLiteral("Metadata"), metadataFor(m_audioRuntime)}});
    });

    connect(m_audioRuntime, &AudioRuntime::volumeChanged, this, [this] {
        emitPropertiesChanged(QString::fromLatin1(kMprisPlayerInterface),
                              {{QStringLiteral("Volume"), m_audioRuntime ? m_audioRuntime->volume() : 0.0}});
    });
}

bool MprisService::registered() const
{
    return m_registered;
}

QString MprisService::errorString() const
{
    return m_errorString;
}

void MprisService::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.isConnected()) {
        m_errorString = tr("D-Bus session bus is not available.");
        return;
    }

    if (!bus.registerService(QString::fromLatin1(kMprisServiceName))) {
        m_errorString = bus.lastError().message();
        if (m_errorString.isEmpty())
            m_errorString = tr("Unable to register Aurora MPRIS service.");
        return;
    }

    if (!bus.registerObject(QString::fromLatin1(kMprisObjectPath),
                            this,
                            QDBusConnection::ExportAdaptors)) {
        m_errorString = bus.lastError().message();
        if (m_errorString.isEmpty())
            m_errorString = tr("Unable to register Aurora MPRIS object.");
        bus.unregisterService(QString::fromLatin1(kMprisServiceName));
        return;
    }

    m_registered = true;
}

void MprisService::emitPropertiesChanged(const QString &interfaceName,
                                         const QVariantMap &changedProperties)
{
    if (!m_registered || changedProperties.isEmpty())
        return;

    QDBusMessage message = QDBusMessage::createSignal(
        QString::fromLatin1(kMprisObjectPath),
        QString::fromLatin1(kDbusPropertiesInterface),
        QStringLiteral("PropertiesChanged"));
    message << interfaceName << changedProperties << QStringList{};

    QDBusConnection::sessionBus().send(message);
}

#include "MprisService.moc"
