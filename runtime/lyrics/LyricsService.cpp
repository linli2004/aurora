#include "runtime/lyrics/LyricsService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>

#include <algorithm>

namespace {
constexpr int lyricsTimeoutMs = 9000;
constexpr qint64 maximumLyricsBytes = 2LL * 1024LL * 1024LL;

qint64 fractionToMilliseconds(const QString &fraction)
{
    if (fraction.isEmpty())
        return 0;
    if (fraction.size() == 1)
        return fraction.toLongLong() * 100;
    if (fraction.size() == 2)
        return fraction.toLongLong() * 10;
    return fraction.left(3).toLongLong();
}

QString cacheRoot()
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (root.isEmpty())
        root = QDir::home().filePath(QStringLiteral(".cache/Aurora"));
    QDir directory;
    directory.mkpath(root);
    return root;
}
}

LyricsService::LyricsService(QObject *parent)
    : QObject(parent),
      m_network(new QNetworkAccessManager(this))
{
    QDir().mkpath(cacheDirectory());
}

QString LyricsService::currentLine() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size())
        return {};
    return m_entries.at(m_currentIndex).text;
}

QString LyricsService::nextLine() const
{
    const int nextIndex = m_currentIndex + 1;
    if (nextIndex < 0 || nextIndex >= m_entries.size())
        return {};
    return m_entries.at(nextIndex).text;
}

bool LyricsService::hasLyrics() const
{
    return !m_entries.isEmpty();
}

QString LyricsService::sourcePath() const
{
    return m_sourcePath;
}

bool LyricsService::loading() const
{
    return m_loading;
}

QString LyricsService::errorString() const
{
    return m_errorString;
}

QString LyricsService::cacheDirectory() const
{
    return QDir(cacheRoot()).filePath(QStringLiteral("lyrics"));
}

void LyricsService::rememberTracks(const QVariantList &tracks)
{
    for (const QVariant &trackValue : tracks) {
        const QVariantMap track = trackValue.toMap();
        const QUrl url(track.value(QStringLiteral("url")).toString());
        const QString key = normalizedSourceKey(url);
        if (key.isEmpty())
            continue;

        TrackHint hint;
        hint.provider = track.value(QStringLiteral("source")).toString().trimmed();
        hint.songId = track.value(QStringLiteral("songId")).toString().trimmed();
        if (!hint.provider.isEmpty() && !hint.songId.isEmpty())
            m_trackHints.insert(key, hint);
    }
}

bool LyricsService::loadForSource(const QUrl &source)
{
    ++m_generation;
    cancelRequest();
    resetLyrics();
    setErrorString({});

    if (!source.isValid() || source.isEmpty())
        return false;

    if (source.isLocalFile() || source.scheme().isEmpty())
        return loadLocalLyrics(source);

    const QString scheme = source.scheme().toCaseFolded();
    if (scheme != QStringLiteral("http") && scheme != QStringLiteral("https"))
        return false;

    const TrackHint hint = m_trackHints.value(normalizedSourceKey(source));
    if (hint.provider.isEmpty() || hint.songId.isEmpty()) {
        setErrorString(tr("This online source did not provide lyric metadata."));
        return false;
    }

    if (loadCachedLyrics(hint))
        return true;

    requestOnlineLyrics(hint, m_generation);
    return true;
}

bool LyricsService::loadFromText(const QString &text)
{
    const QVector<Entry> entries = parseLrc(text);
    if (entries.isEmpty()) {
        resetLyrics();
        return false;
    }

    m_entries = entries;
    m_currentIndex = -1;
    emit lyricsChanged();
    return true;
}

void LyricsService::setPosition(qint64 positionMilliseconds)
{
    updateCurrentIndex(std::max<qint64>(0, positionMilliseconds));
}

void LyricsService::clear()
{
    ++m_generation;
    cancelRequest();
    resetLyrics();
    setErrorString({});
}

QString LyricsService::normalizedSourceKey(const QUrl &source)
{
    if (!source.isValid() || source.isEmpty())
        return {};
    return source.adjusted(QUrl::NormalizePathSegments | QUrl::RemovePassword)
        .toString(QUrl::RemovePassword);
}

void LyricsService::cancelRequest()
{
    QNetworkReply *reply = m_activeReply.data();
    if (!reply)
        return;

    m_activeReply.clear();
    QObject::disconnect(reply, nullptr, this, nullptr);
    reply->abort();
    reply->deleteLater();
    setLoading(false);
}

void LyricsService::resetLyrics()
{
    if (m_entries.isEmpty() && m_sourcePath.isEmpty() && m_currentIndex == -1)
        return;

    m_entries.clear();
    m_sourcePath.clear();
    m_currentIndex = -1;
    emit lyricsChanged();
}

bool LyricsService::loadLocalLyrics(const QUrl &source)
{
    const QString mediaPath = source.isLocalFile()
        ? source.toLocalFile()
        : source.toString();

    const QFileInfo mediaInfo(mediaPath);
    if (!mediaInfo.exists() || !mediaInfo.isFile())
        return false;

    const QString basePath = mediaInfo.dir().filePath(mediaInfo.completeBaseName());
    const QStringList candidates = {
        basePath + QStringLiteral(".lrc"),
        mediaInfo.absoluteFilePath() + QStringLiteral(".lrc")
    };

    for (const QString &candidate : candidates) {
        QFile file(candidate);
        if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        if (!loadFromText(QString::fromUtf8(file.readAll())))
            continue;

        m_sourcePath = QFileInfo(candidate).absoluteFilePath();
        emit lyricsChanged();
        return true;
    }

    setErrorString(tr("No sidecar LRC file was found."));
    return false;
}

bool LyricsService::loadCachedLyrics(const TrackHint &hint)
{
    const QString path = cachedLyricsPath(hint);
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    if (!loadFromText(QString::fromUtf8(file.readAll()))) {
        file.close();
        QFile::remove(path);
        return false;
    }

    m_sourcePath = path;
    emit lyricsChanged();
    return true;
}

void LyricsService::requestOnlineLyrics(
    const TrackHint &hint,
    int generation)
{
    if (hint.provider != QStringLiteral("wy")) {
        setErrorString(tr("Online lyrics are not supported for this provider yet."));
        return;
    }

    const QUrl endpoint(QStringLiteral(
        "https://music.163.com/api/song/lyric?id=%1&lv=1&kv=1&tv=-1")
        .arg(hint.songId));

    QNetworkRequest request(endpoint);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 Aurora/1.0");
    request.setRawHeader("Referer", "https://music.163.com/");

    QNetworkReply *reply = m_network->get(request);
    m_activeReply = reply;
    setLoading(true);

    connect(reply, &QNetworkReply::downloadProgress,
            this, [reply](qint64 received, qint64 total) {
        if (received > maximumLyricsBytes || total > maximumLyricsBytes)
            reply->abort();
    });

    QTimer::singleShot(lyricsTimeoutMs, reply, [reply] {
        if (reply->isRunning())
            reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, generation, hint] {
        if (m_activeReply == reply)
            m_activeReply.clear();

        const bool current = generation == m_generation;
        const QByteArray bytes = reply->error() == QNetworkReply::NoError
            ? reply->readAll()
            : QByteArray();
        reply->deleteLater();

        if (!current)
            return;

        setLoading(false);
        if (bytes.isEmpty() || bytes.size() > maximumLyricsBytes) {
            setErrorString(tr("Online lyrics could not be downloaded."));
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(bytes);
        const QJsonObject root = document.object();
        const QString lrc = root.value(QStringLiteral("lrc"))
            .toObject()
            .value(QStringLiteral("lyric"))
            .toString();

        if (!loadFromText(lrc)) {
            setErrorString(tr("This track has no timed lyrics."));
            return;
        }

        const QString path = cachedLyricsPath(hint);
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile cache(path);
        if (cache.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            cache.write(lrc.toUtf8());

        m_sourcePath = path;
        m_errorString.clear();
        emit lyricsChanged();
        emit statusChanged();
    });
}

QString LyricsService::cachedLyricsPath(const TrackHint &hint) const
{
    QString safeProvider = hint.provider;
    safeProvider.remove(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")));
    QString safeSongId = hint.songId;
    safeSongId.remove(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")));
    return QDir(cacheDirectory()).filePath(
        safeProvider + QLatin1Char('-') + safeSongId + QStringLiteral(".lrc"));
}

void LyricsService::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    emit statusChanged();
}

void LyricsService::setErrorString(const QString &message)
{
    if (m_errorString == message)
        return;
    m_errorString = message;
    emit statusChanged();
}

QVector<LyricsService::Entry> LyricsService::parseLrc(const QString &text)
{
    static const QRegularExpression timestampExpression(
        QStringLiteral(R"(\[(\d{1,3}):(\d{2})(?:[\.:](\d{1,3}))?\])"));
    static const QRegularExpression offsetExpression(
        QStringLiteral(R"(\[offset:([+-]?\d+)\])"),
        QRegularExpression::CaseInsensitiveOption);

    qint64 offset = 0;
    const QRegularExpressionMatch offsetMatch = offsetExpression.match(text);
    if (offsetMatch.hasMatch())
        offset = offsetMatch.captured(1).toLongLong();

    QVector<Entry> entries;
    const QStringList lines = text.split(
        QRegularExpression(QStringLiteral("[\r\n]+")));
    for (const QString &rawLine : lines) {
        QRegularExpressionMatchIterator iterator =
            timestampExpression.globalMatch(rawLine);
        QVector<qint64> timestamps;
        while (iterator.hasNext()) {
            const QRegularExpressionMatch match = iterator.next();
            const qint64 minutes = match.captured(1).toLongLong();
            const qint64 seconds = match.captured(2).toLongLong();
            const qint64 fraction = fractionToMilliseconds(match.captured(3));
            timestamps.append(std::max<qint64>(
                0,
                minutes * 60000 + seconds * 1000 + fraction + offset));
        }

        if (timestamps.isEmpty())
            continue;

        QString line = rawLine;
        line.remove(timestampExpression);
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        for (const qint64 timestamp : timestamps)
            entries.append({timestamp, line});
    }

    std::stable_sort(
        entries.begin(),
        entries.end(),
        [](const Entry &left, const Entry &right) {
            return left.timestamp < right.timestamp;
        });
    return entries;
}

void LyricsService::updateCurrentIndex(qint64 positionMilliseconds)
{
    int resolvedIndex = -1;
    for (int index = 0; index < m_entries.size(); ++index) {
        if (m_entries.at(index).timestamp > positionMilliseconds)
            break;
        resolvedIndex = index;
    }

    if (resolvedIndex == m_currentIndex)
        return;

    m_currentIndex = resolvedIndex;
    emit lyricsChanged();
}
