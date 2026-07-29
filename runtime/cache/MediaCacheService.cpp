#include "runtime/cache/MediaCacheService.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace {
constexpr int mediaTimeoutMs = 60000;
constexpr qint64 maximumSingleMediaBytes = 256LL * 1024LL * 1024LL;

QString cacheRoot()
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (root.isEmpty())
        root = QDir::home().filePath(QStringLiteral(".cache/Aurora"));
    QDir directory;
    directory.mkpath(root);
    return root;
}

QString safeSuffix(const QUrl &source)
{
    const QString suffix = QFileInfo(source.path()).suffix().toCaseFolded();
    static const QStringList supported = {
        QStringLiteral("mp3"),
        QStringLiteral("m4a"),
        QStringLiteral("aac"),
        QStringLiteral("flac"),
        QStringLiteral("ogg"),
        QStringLiteral("opus"),
        QStringLiteral("wav")
    };
    return supported.contains(suffix) ? suffix : QStringLiteral("mp3");
}

bool isMetadataFile(const QFileInfo &file)
{
    return file.suffix().toCaseFolded() == QStringLiteral("json");
}
}

MediaCacheService::MediaCacheService(QObject *parent)
    : QObject(parent),
      m_network(new QNetworkAccessManager(this))
{
    QSettings settings;
    const QString key = QStringLiteral("cache/media/maxBytes");
    if (!settings.contains(key))
        settings.setValue(key, m_limitBytes);

    m_limitBytes = std::clamp<qint64>(
        settings.value(key, m_limitBytes).toLongLong(),
        128LL * 1024LL * 1024LL,
        8LL * 1024LL * 1024LL * 1024LL);

    QDir().mkpath(directory());
    prune();
}

QString MediaCacheService::directory() const
{
    return QDir(cacheRoot()).filePath(QStringLiteral("media"));
}

qint64 MediaCacheService::sizeBytes() const
{
    qint64 total = 0;
    const QFileInfoList files = QDir(directory()).entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &file : files)
        total += file.size();
    return total;
}

qint64 MediaCacheService::limitBytes() const
{
    return m_limitBytes;
}

QUrl MediaCacheService::playbackSource(const QUrl &source)
{
    if (!isRemoteMedia(source))
        return source;

    const QString path = cachePathFor(source);
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile() || info.size() <= 0)
        return source;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
        file.setFileTime(QDateTime::currentDateTimeUtc(), QFileDevice::FileModificationTime);

    return QUrl::fromLocalFile(path);
}

void MediaCacheService::warm(
    const QUrl &source,
    const QString &catalogKey)
{
    if (!isRemoteMedia(source))
        return;

    const QString key = cacheKeyFor(source);
    const QString normalizedCatalogKey = catalogKey.trimmed();
    if (!normalizedCatalogKey.isEmpty())
        m_pendingCatalogKeys.insert(key, normalizedCatalogKey);

    const QString finalPath = cachePathFor(source);
    if (QFileInfo::exists(finalPath)) {
        writeMetadata(finalPath, source, normalizedCatalogKey);
        m_pendingCatalogKeys.remove(key);
        return;
    }

    if (m_activeReplies.contains(key))
        return;

    QDir().mkpath(directory());

    QNetworkRequest request(source);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    // AUR-SOURCE-ARTWORK-RECOVERY-PACK-01:MEDIA-HEADERS
    request.setRawHeader(
        "User-Agent",
        "Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 Aurora/1.0");
    request.setRawHeader(
        "Accept",
        "audio/*,application/octet-stream;q=0.9,*/*;q=0.5");
    request.setRawHeader("Accept-Encoding", "identity");
    request.setRawHeader("Referer", "https://music.163.com/");

    QNetworkReply *reply = m_network->get(request);
    m_activeReplies.insert(key, reply);

    QSaveFile *file = new QSaveFile(finalPath);
    file->setParent(reply);
    if (!file->open(QIODevice::WriteOnly)) {
        m_activeReplies.remove(key);
        reply->abort();
        reply->deleteLater();
        return;
    }

    connect(reply, &QNetworkReply::readyRead, this, [reply, file] {
        const QByteArray bytes = reply->readAll();
        if (!bytes.isEmpty() && file->write(bytes) != bytes.size())
            reply->abort();
    });

    connect(reply, &QNetworkReply::downloadProgress,
            this, [reply](qint64 received, qint64 total) {
        if (received > maximumSingleMediaBytes
            || total > maximumSingleMediaBytes) {
            reply->abort();
        }
    });

    QTimer::singleShot(mediaTimeoutMs, reply, [reply] {
        if (reply->isRunning())
            reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, file, key, source, finalPath] {
        if (reply->bytesAvailable() > 0) {
            const QByteArray tail = reply->readAll();
            if (!tail.isEmpty())
                file->write(tail);
        }

        const QString contentType =
            reply->header(
                QNetworkRequest::ContentTypeHeader)
                .toString()
                .toCaseFolded();
        const bool rejectedPayload =
            contentType.contains(QStringLiteral("text/html"))
            || contentType.contains(QStringLiteral("application/json"));

        bool success =
            reply->error() == QNetworkReply::NoError
            && !rejectedPayload
            && file->commit();

        if (success
            && QFileInfo(finalPath).size() < 1024) {
            QFile::remove(finalPath);
            success = false;
        }

        if (!success)
            file->cancelWriting();

        const QString catalogKey = m_pendingCatalogKeys.take(key);
        m_activeReplies.remove(key);
        reply->deleteLater();
        if (success) {
            writeMetadata(finalPath, source, catalogKey);
            prune();
            emit cacheChanged();
            // AUR-SOURCE-ARTWORK-RECOVERY-PACK-01:MEDIA-READY
            emit mediaReady(
                source,
                QUrl::fromLocalFile(finalPath));
        }
    });
}

void MediaCacheService::reconcileCatalogKeys(
    const QSet<QString> &activeCatalogKeys)
{
    if (activeCatalogKeys.isEmpty())
        return;

    QDir cacheDirectory(directory());
    const QFileInfoList files = cacheDirectory.entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot);

    bool changed = false;
    for (const QFileInfo &file : files) {
        if (isMetadataFile(file))
            continue;

        const QString metadataPath =
            file.absoluteFilePath() + QStringLiteral(".json");
        const QString catalogKey = readCatalogKey(metadataPath);
        if (catalogKey.isEmpty() || activeCatalogKeys.contains(catalogKey))
            continue;

        changed = QFile::remove(file.absoluteFilePath()) || changed;
        changed = QFile::remove(metadataPath) || changed;
    }

    for (const QFileInfo &file : files) {
        if (!isMetadataFile(file))
            continue;

        QString mediaPath = file.absoluteFilePath();
        mediaPath.chop(5);
        if (!QFileInfo::exists(mediaPath))
            changed = QFile::remove(file.absoluteFilePath()) || changed;
    }

    if (changed)
        emit cacheChanged();
}

void MediaCacheService::clear()
{
    const auto replies = m_activeReplies.values();
    m_activeReplies.clear();
    m_pendingCatalogKeys.clear();

    for (const QPointer<QNetworkReply> &guard : replies) {
        QNetworkReply *reply = guard.data();
        if (!reply)
            continue;

        QObject::disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }

    QDir cacheDirectory(directory());
    const QFileInfoList files = cacheDirectory.entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot);
    bool changed = false;
    for (const QFileInfo &file : files)
        changed = QFile::remove(file.absoluteFilePath()) || changed;

    if (changed)
        emit cacheChanged();
}

bool MediaCacheService::isRemoteMedia(const QUrl &source) const
{
    const QString scheme = source.scheme().toCaseFolded();
    return (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"))
        && !source.host().isEmpty();
}

QString MediaCacheService::cachePathFor(const QUrl &source) const
{
    return QDir(directory()).filePath(
        cacheKeyFor(source)
        + QLatin1Char('.')
        + safeSuffix(source));
}

QString MediaCacheService::cacheKeyFor(const QUrl &source) const
{
    const QString normalized = source.adjusted(
        QUrl::NormalizePathSegments | QUrl::RemovePassword)
        .toString(QUrl::RemovePassword);
    return QString::fromLatin1(
        QCryptographicHash::hash(normalized.toUtf8(), QCryptographicHash::Sha256)
            .toHex());
}

void MediaCacheService::writeMetadata(
    const QString &mediaPath,
    const QUrl &source,
    const QString &catalogKey) const
{
    if (catalogKey.trimmed().isEmpty())
        return;

    QJsonObject root;
    root.insert(QStringLiteral("catalogKey"), catalogKey.trimmed());
    root.insert(
        QStringLiteral("source"),
        source.adjusted(
                  QUrl::NormalizePathSegments | QUrl::RemovePassword)
            .toString(QUrl::RemovePassword));
    root.insert(
        QStringLiteral("updatedAt"),
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    QSaveFile file(mediaPath + QStringLiteral(".json"));
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    file.commit();
}

QString MediaCacheService::readCatalogKey(
    const QString &metadataPath) const
{
    QFile file(metadataPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return {};

    const QJsonObject root =
        QJsonDocument::fromJson(file.readAll()).object();
    return root.value(QStringLiteral("catalogKey"))
        .toString()
        .trimmed();
}

void MediaCacheService::prune()
{
    QFileInfoList files = QDir(directory()).entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Time | QDir::Reversed);

    qint64 total = 0;
    for (const QFileInfo &file : files)
        total += file.size();

    bool changed = false;
    for (const QFileInfo &file : files) {
        if (total <= m_limitBytes)
            break;

        if (isMetadataFile(file))
            continue;

        const qint64 size = file.size();
        if (QFile::remove(file.absoluteFilePath())) {
            total -= size;
            changed = true;
        }

        const QString metadataPath =
            file.absoluteFilePath() + QStringLiteral(".json");
        const QFileInfo metadataInfo(metadataPath);
        if (metadataInfo.exists()
            && QFile::remove(metadataPath)) {
            total -= metadataInfo.size();
            changed = true;
        }
    }

    if (changed)
        emit cacheChanged();
}
