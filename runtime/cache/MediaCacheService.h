#pragma once

#include <QObject>
#include <QHash>
#include <QPointer>
#include <QSet>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class MediaCacheService final : public QObject
{
    Q_OBJECT

public:
    explicit MediaCacheService(QObject *parent = nullptr);

    [[nodiscard]] QString directory() const;
    [[nodiscard]] qint64 sizeBytes() const;
    [[nodiscard]] qint64 limitBytes() const;

    QUrl playbackSource(const QUrl &source);
    void warm(
        const QUrl &source,
        const QString &catalogKey = QString());
    void reconcileCatalogKeys(const QSet<QString> &activeCatalogKeys);
    void clear();

signals:
    void cacheChanged();
    void mediaReady(
        const QUrl &originalSource,
        const QUrl &cachedSource);

private:
    [[nodiscard]] bool isRemoteMedia(const QUrl &source) const;
    [[nodiscard]] QString cachePathFor(const QUrl &source) const;
    [[nodiscard]] QString cacheKeyFor(const QUrl &source) const;
    void writeMetadata(
        const QString &mediaPath,
        const QUrl &source,
        const QString &catalogKey) const;
    [[nodiscard]] QString readCatalogKey(const QString &metadataPath) const;
    void prune();

    QNetworkAccessManager *m_network = nullptr;
    QHash<QString, QPointer<QNetworkReply>> m_activeReplies;
    QHash<QString, QString> m_pendingCatalogKeys;
    qint64 m_limitBytes = 1024LL * 1024LL * 1024LL;
};
