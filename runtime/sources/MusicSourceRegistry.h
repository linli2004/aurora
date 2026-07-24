#pragma once

#include <optional>

#include <QObject>
#include <QUrl>
#include <QVariantList>

#include <QNetworkAccessManager>

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
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)

public:
    explicit MusicSourceRegistry(QObject *parent = nullptr);

    [[nodiscard]] QVariantList sources() const;
    [[nodiscard]] QStringList sourceNames() const;
    [[nodiscard]] int sourceCount() const;
    [[nodiscard]] bool busy() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] QString errorString() const;

    Q_INVOKABLE void importFromText(const QString &sourceText);
    Q_INVOKABLE void clearSources();

    [[nodiscard]] static QStringList entriesFromText(const QString &sourceText);
    [[nodiscard]] static std::optional<MusicSourceRecord> parseScriptMetadata(
        const QString &script,
        const QUrl &scriptUrl);
    [[nodiscard]] static QString sourceIdForUrl(const QUrl &scriptUrl);

signals:
    void sourcesChanged();
    void busyChanged();
    void statusChanged();

private:
    void loadSources();
    void saveSources() const;
    bool importScript(const QString &script, const QUrl &scriptUrl);
    void importRemoteScript(const QUrl &scriptUrl);
    void finishRemoteImport(bool imported);
    void setBusy(bool busy);
    void setStatusText(const QString &message);
    void setErrorString(const QString &message);

    QNetworkAccessManager m_network;
    QList<MusicSourceRecord> m_sources;
    int m_pendingRequests = 0;
    int m_lastBatchImported = 0;
    bool m_busy = false;
    QString m_statusText;
    QString m_errorString;
};
