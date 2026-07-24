#pragma once

#include <optional>
#include <memory>

#include <QObject>
#include <QUrl>
#include <QVariantList>

#include <QNetworkAccessManager>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>

struct MusicSourceRecord
{
    QString sourceId;
    QString name;
    QString version;
    QUrl scriptUrl;
    QString importedAt;
};

class MusicSourceRegistry final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList sources READ sources NOTIFY sourcesChanged)
    Q_PROPERTY(QStringList sourceNames READ sourceNames NOTIFY sourcesChanged)
    Q_PROPERTY(int sourceCount READ sourceCount NOTIFY sourcesChanged)
    Q_PROPERTY(QVariantList onlineTracks READ onlineTracks NOTIFY onlineTracksChanged)
    Q_PROPERTY(int onlineTrackCount READ onlineTrackCount NOTIFY onlineTracksChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool catalogBusy READ catalogBusy NOTIFY catalogBusyChanged)
    Q_PROPERTY(bool resolving READ resolving NOTIFY resolvingChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)

public:
    explicit MusicSourceRegistry(QObject *parent = nullptr);

    [[nodiscard]] QVariantList sources() const;
    [[nodiscard]] QStringList sourceNames() const;
    [[nodiscard]] int sourceCount() const;
    [[nodiscard]] QVariantList onlineTracks() const;
    [[nodiscard]] int onlineTrackCount() const;
    [[nodiscard]] bool busy() const;
    [[nodiscard]] bool catalogBusy() const;
    [[nodiscard]] bool resolving() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] QString errorString() const;

    Q_INVOKABLE void importFromText(const QString &sourceText);
    Q_INVOKABLE void loadOnlineTracks();
    Q_INVOKABLE void resolveFromText(const QString &requestText);
    Q_INVOKABLE void resolveDemoTrack();
    Q_INVOKABLE void resolveOnlineTrackAt(int index);
    Q_INVOKABLE void resolveOnlineTracksFrom(int index);
    Q_INVOKABLE void clearSources();

    [[nodiscard]] static QStringList entriesFromText(const QString &sourceText);
    [[nodiscard]] static std::optional<MusicSourceRecord> parseScriptMetadata(
        const QString &script,
        const QUrl &scriptUrl);
    [[nodiscard]] static QString sourceIdForUrl(const QUrl &scriptUrl);

    // Internal bridge used by the LX-compatible script sandbox.
    Q_INVOKABLE void scriptSend(const QString &eventName, const QJSValue &payload);
    Q_INVOKABLE void scriptRequest(
        const QString &url,
        const QJSValue &options,
        const QJSValue &callback);
    Q_INVOKABLE void scriptResolve(const QJSValue &value);
    Q_INVOKABLE void scriptReject(const QJSValue &value);
    Q_INVOKABLE QString scriptMd5(const QString &value) const;

    struct OnlineTrack
    {
        QString source;
        QString songId;
        QString title;
        QString artist;
        QString album;
        QString artworkUrl;
    };

signals:
    void sourcesChanged();
    void onlineTracksChanged();
    void busyChanged();
    void catalogBusyChanged();
    void resolvingChanged();
    void statusChanged();
    void musicUrlResolved(const QString &url);
    void musicUrlsResolved(const QStringList &urls);

private:
    struct ResolveRequest
    {
        QString source;
        QString songId;
        QString quality;
    };

    void loadSources();
    void saveSources() const;
    bool importScript(const QString &script, const QUrl &scriptUrl);
    void importRemoteScript(const QUrl &scriptUrl);
    void finishRemoteImport(bool imported);
    [[nodiscard]] std::optional<ResolveRequest> parseResolveRequest(
        const QString &requestText) const;
    [[nodiscard]] std::optional<ResolveRequest> resolveRequestForOnlineTrack(int index) const;
    void startResolveRequest(const ResolveRequest &request);
    void continuePlaylistResolve();
    void resolveWithSourceAt(int index);
    void loadScriptForResolve(const MusicSourceRecord &record, int index);
    void evaluateScriptForResolve(
        const QString &script,
        const MusicSourceRecord &record,
        int index);
    bool resolveKnownDirectSource(
        const MusicSourceRecord &record,
        const QString &script,
        int index);
    void resolveBuiltInFallbackSource(const QString &previousError);
    void requestCurrentMusicUrl();
    void failCurrentResolver(const QString &message);
    void finishCurrentResolver(const QString &musicUrl);
    void setBusy(bool busy);
    void setCatalogBusy(bool busy);
    void setResolving(bool resolving);
    void setStatusText(const QString &message);
    void setErrorString(const QString &message);

    QNetworkAccessManager m_network;
    QList<MusicSourceRecord> m_sources;
    QList<OnlineTrack> m_onlineTracks;
    std::unique_ptr<QJSEngine> m_scriptEngine;
    ResolveRequest m_resolveRequest;
    QList<ResolveRequest> m_playlistRequests;
    QStringList m_playlistResolvedUrls;
    int m_pendingRequests = 0;
    int m_lastBatchImported = 0;
    int m_resolveGeneration = 0;
    bool m_busy = false;
    bool m_catalogBusy = false;
    bool m_resolving = false;
    bool m_resolvingPlaylist = false;
    QString m_statusText;
    QString m_errorString;
};
