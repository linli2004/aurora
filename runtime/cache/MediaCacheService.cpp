#include "runtime/cache/MediaCacheService.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
constexpr int mediaTimeoutMs = 30000;
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

void MediaCacheService::warm(const QUrl &source)
{
    if (!isRemoteMedia(source))
        return;

    const QString key = cacheKeyFor(source);
    const QString finalPath = cachePathFor(source);
    if (QFileInfo::exists(finalPath) || m_activeReplies.contains(key))
        return;

    QDir().mkpath(directory());

    QNetworkRequest request(source);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Aurora-Music-Framework/1.0");

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
            [this, reply, file, key] {
        if (reply->bytesAvailable() > 0) {
            const QByteArray tail = reply->readAll();
            if (!tail.isEmpty())
                file->write(tail);
        }

        const bool success = reply->error() == QNetworkReply::NoError
            && file->commit();

        if (!success)
            file->cancelWriting();

        m_activeReplies.remove(key);
        reply->deleteLater();

        if (success) {
            prune();
            emit cacheChanged();
        }
    });
}

void MediaCacheService::clear()
{
    const auto replies = m_activeReplies.values();
    m_activeReplies.clear();

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

        const qint64 size = file.size();
        if (QFile::remove(file.absoluteFilePath())) {
            total -= size;
            changed = true;
        }
    }

    if (changed)
        emit cacheChanged();
}
