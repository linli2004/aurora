#pragma once

#include <QObject>
#include <QHash>
#include <QPointer>
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
    void warm(const QUrl &source);
    void clear();

signals:
    void cacheChanged();

private:
    [[nodiscard]] bool isRemoteMedia(const QUrl &source) const;
    [[nodiscard]] QString cachePathFor(const QUrl &source) const;
    [[nodiscard]] QString cacheKeyFor(const QUrl &source) const;
    void prune();

    QNetworkAccessManager *m_network = nullptr;
    QHash<QString, QPointer<QNetworkReply>> m_activeReplies;
    qint64 m_limitBytes = 1024LL * 1024LL * 1024LL;
};
