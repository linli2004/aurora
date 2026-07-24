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
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool resolving READ resolving NOTIFY resolvingChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)

public:
    explicit MusicSourceRegistry(QObject *parent = nullptr);

    [[nodiscard]] QVariantList sources() const;
    [[nodiscard]] QStringList sourceNames() const;
    [[nodiscard]] int sourceCount() const;
    [[nodiscard]] bool busy() const;
    [[nodiscard]] bool resolving() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] QString errorString() const;

    Q_INVOKABLE void importFromText(const QString &sourceText);
    Q_INVOKABLE void resolveFromText(const QString &requestText);
    Q_INVOKABLE void resolveDemoTrack();
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

signals:
    void sourcesChanged();
    void busyChanged();
    void resolvingChanged();
    void statusChanged();
    void musicUrlResolved(const QString &url);

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
    void setResolving(bool resolving);
    void setStatusText(const QString &message);
    void setErrorString(const QString &message);

    QNetworkAccessManager m_network;
    QList<MusicSourceRecord> m_sources;
    std::unique_ptr<QJSEngine> m_scriptEngine;
    ResolveRequest m_resolveRequest;
    int m_pendingRequests = 0;
    int m_lastBatchImported = 0;
    int m_resolveGeneration = 0;
    bool m_busy = false;
    bool m_resolving = false;
    QString m_statusText;
    QString m_errorString;
};
