#include "runtime/sources/MusicSourceRegistry.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSettings>
#include <QTimer>
#include <QUrlQuery>
#include <QVariantMap>

namespace {
constexpr auto settingsKey = "music/sources/v1";

QString nowUtc()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

bool isRemoteScriptUrl(const QUrl &url)
{
    const QString scheme = url.scheme().toCaseFolded();
    const QString path = url.path().toCaseFolded();
    return (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"))
        && !url.host().isEmpty()
        && path.endsWith(QStringLiteral(".js"));
}

bool looksLikeScriptText(const QString &text)
{
    return text.contains(QStringLiteral("@name"))
        && text.contains(QStringLiteral("EVENT_NAMES"));
}

QString metadataValue(const QString &script, const QString &key)
{
    const QRegularExpression expression(
        QStringLiteral("@%1\\s+([^*\\r\\n]+)").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = expression.match(script.left(2048));
    return match.hasMatch() ? match.captured(1).simplified() : QString();
}

QUrl normalizedScriptUrl(const QUrl &url)
{
    return url.adjusted(QUrl::NormalizePathSegments | QUrl::RemovePassword);
}

QVariantMap recordToMap(const MusicSourceRecord &record)
{
    QVariantMap map;
    map.insert(QStringLiteral("sourceId"), record.sourceId);
    map.insert(QStringLiteral("name"), record.name);
    map.insert(QStringLiteral("version"), record.version);
    map.insert(QStringLiteral("scriptUrl"), record.scriptUrl.toString());
    map.insert(QStringLiteral("importedAt"), record.importedAt);
    return map;
}

QVariantMap onlineTrackToMap(const MusicSourceRegistry::OnlineTrack &track)
{
    QVariantMap map;
    map.insert(QStringLiteral("source"), track.source);
    map.insert(QStringLiteral("songId"), track.songId);
    map.insert(QStringLiteral("title"), track.title);
    map.insert(QStringLiteral("artist"), track.artist);
    map.insert(QStringLiteral("album"), track.album);
    map.insert(QStringLiteral("artworkUrl"), track.artworkUrl);
    map.insert(QStringLiteral("requestText"),
               QStringLiteral("%1:%2 128k").arg(track.source, track.songId));
    return map;
}

QVariantMap resolvedOnlineTrackToMap(
    const MusicSourceRegistry::OnlineTrack &track,
    const QString &musicUrl)
{
    QVariantMap map = onlineTrackToMap(track);
    map.insert(QStringLiteral("url"), musicUrl);
    return map;
}

QList<MusicSourceRegistry::OnlineTrack> defaultOnlineTracks()
{
    return {
        {QStringLiteral("wy"), QStringLiteral("33894312"), QStringLiteral("失眠"), QStringLiteral("Suki刘舒妤"), QStringLiteral("Ladies Night"), {}},
        {QStringLiteral("wy"), QStringLiteral("1973665667"), QStringLiteral("海屿你"), QStringLiteral("马也_Crabbit"), QStringLiteral("海屿你"), {}},
        {QStringLiteral("wy"), QStringLiteral("3399839173"), QStringLiteral("甲乙丙丁"), QStringLiteral("李佳薇"), QStringLiteral("甲乙丙丁"), {}},
        {QStringLiteral("wy"), QStringLiteral("1303464858"), QStringLiteral("于是"), QStringLiteral("郑润泽"), QStringLiteral("于是"), {}},
        {QStringLiteral("wy"), QStringLiteral("3382908505"), QStringLiteral("玻璃"), QStringLiteral("Gareth.T"), QStringLiteral("玻璃"), {}},
        {QStringLiteral("wy"), QStringLiteral("3404238777"), QStringLiteral("周旋"), QStringLiteral("王以太 / 艾热 AIR"), QStringLiteral("太热爱"), {}},
        {QStringLiteral("wy"), QStringLiteral("1827600686"), QStringLiteral("还是会想你"), QStringLiteral("林达浪 / h3R3"), QStringLiteral("还是会想你"), {}},
        {QStringLiteral("wy"), QStringLiteral("488249475"), QStringLiteral("哪里都是你"), QStringLiteral("队长"), QStringLiteral("哪里都是你"), {}},
        {QStringLiteral("wy"), QStringLiteral("27747329"), QStringLiteral("坠落"), QStringLiteral("蔡健雅"), QStringLiteral("天使与魔鬼的对话"), {}},
        {QStringLiteral("wy"), QStringLiteral("31654343"), QStringLiteral("不将就"), QStringLiteral("李荣浩"), QStringLiteral("有理想"), {}},
    };
}

MusicSourceRecord mapToRecord(const QVariantMap &map)
{
    MusicSourceRecord record;
    record.sourceId = map.value(QStringLiteral("sourceId")).toString();
    record.name = map.value(QStringLiteral("name")).toString();
    record.version = map.value(QStringLiteral("version")).toString();
    record.scriptUrl = QUrl(map.value(QStringLiteral("scriptUrl")).toString());
    record.importedAt = map.value(QStringLiteral("importedAt")).toString();
    return record;
}

QUrl sourceUrlFromRecord(const MusicSourceRecord &record)
{
    return record.scriptUrl.adjusted(QUrl::NormalizePathSegments | QUrl::RemovePassword);
}

bool isReadableLocalScript(const QUrl &url)
{
    if (!url.isLocalFile())
        return false;

    const QFileInfo info(url.toLocalFile());
    return info.exists() && info.isFile() && info.isReadable();
}

QJSValue jsonValueToScriptValue(QJSEngine *engine, const QByteArray &bytes)
{
    const QJsonDocument document = QJsonDocument::fromJson(bytes);
    if (document.isObject())
        return engine->toScriptValue(document.object().toVariantMap());
    if (document.isArray())
        return engine->toScriptValue(document.array().toVariantList());
    return QJSValue(QString::fromUtf8(bytes));
}

QVariantMap jsObjectToMap(const QJSValue &value)
{
    return value.toVariant().toMap();
}

QString jsValueText(const QJSValue &value)
{
    QString text;
    if (value.isError())
        text = value.property(QStringLiteral("message")).toString();

    const QJSValue message = value.property(QStringLiteral("message"));
    if (text.isEmpty() && message.isString())
        text = message.toString();

    if (text.isEmpty())
        text = value.toString();
    if (text.isEmpty() || text == QStringLiteral("undefined") || text == QStringLiteral("null"))
        text = QStringLiteral("LX source rejected without an error message.");

    const QJSValue lineNumberValue = value.property(QStringLiteral("lineNumber"));
    const int lineNumber = lineNumberValue.isNumber() ? lineNumberValue.toInt() : 0;
    if (lineNumber > 0)
        text += QStringLiteral(" at line %1").arg(lineNumber);

    const QJSValue stackValue = value.property(QStringLiteral("stack"));
    const QString stack = stackValue.isString() ? stackValue.toString() : QString();
    if (!stack.isEmpty())
        text += QStringLiteral(" (%1)").arg(stack);

    return text;
}

QString jsStringLiteral(const QString &value)
{
    return QString::fromUtf8(
        QJsonDocument(QJsonArray { value }).toJson(QJsonDocument::Compact))
        .mid(1)
        .chopped(1);
}

QString scriptForQJSEngine(QString script)
{
    const QRegularExpression nullishExpression(QStringLiteral(
        R"(([A-Za-z_$][A-Za-z0-9_$]*(?:\.[A-Za-z_$][A-Za-z0-9_$]*|\[[^\]\n]+\])*)\s*\?\?\s*([^;\n,)}]+))"));

    while (script.contains(QStringLiteral("??"))) {
        const QRegularExpressionMatch match = nullishExpression.match(script);
        if (!match.hasMatch())
            break;

        const QString replacement = QStringLiteral("__auroraNullish(%1, %2)")
                                        .arg(match.captured(1).trimmed(),
                                             match.captured(2).trimmed());
        script.replace(match.capturedStart(), match.capturedLength(), replacement);
    }

    return script;
}

QString scriptStringConstant(const QString &script, const QString &name)
{
    const QRegularExpression expression(
        QStringLiteral(R"(const\s+%1\s*=\s*["']([^"']*)["'])")
            .arg(QRegularExpression::escape(name)));
    const QRegularExpressionMatch match = expression.match(script);
    return match.hasMatch() ? match.captured(1).trimmed() : QString();
}

QString jsonErrorText(const QJsonObject &object)
{
    const QString message = object.value(QStringLiteral("msg")).toString();
    if (!message.isEmpty())
        return message;

    const QString alternateMessage = object.value(QStringLiteral("message")).toString();
    return alternateMessage.isEmpty() ? QStringLiteral("unknown error") : alternateMessage;
}

QString artistsText(const QJsonArray &artists)
{
    QStringList names;
    for (const QJsonValue &artistValue : artists) {
        const QString name = artistValue.toObject().value(QStringLiteral("name")).toString().simplified();
        if (!name.isEmpty())
            names.append(name);
    }
    return names.join(QStringLiteral(" / "));
}

QString normalizedBufferEncoding(QString encoding)
{
    encoding = encoding.trimmed().toLower();
    encoding.remove(QLatin1Char('-'));
    encoding.remove(QLatin1Char('_'));
    if (encoding.isEmpty() || encoding == QStringLiteral("undefined"))
        return QStringLiteral("utf8");
    if (encoding == QStringLiteral("utf8") || encoding == QStringLiteral("utf"))
        return QStringLiteral("utf8");
    if (encoding == QStringLiteral("base64"))
        return QStringLiteral("base64");
    if (encoding == QStringLiteral("hex"))
        return QStringLiteral("hex");
    if (encoding == QStringLiteral("latin1") || encoding == QStringLiteral("binary")
        || encoding == QStringLiteral("ascii")) {
        return QStringLiteral("latin1");
    }
    return QStringLiteral("utf8");
}

QByteArray bytesFromLeadingHex(const QString &value)
{
    const QByteArray input = value.toLatin1();
    QByteArray bytes;
    bytes.reserve(input.size() / 2);

    const auto hexValue = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return 10 + c - 'a';
        if (c >= 'A' && c <= 'F')
            return 10 + c - 'A';
        return -1;
    };

    for (qsizetype i = 0; i + 1 < input.size(); i += 2) {
        const int high = hexValue(input.at(i));
        const int low = hexValue(input.at(i + 1));
        if (high < 0 || low < 0)
            break;
        bytes.append(static_cast<char>((high << 4) | low));
    }

    return bytes;
}

QByteArray scriptBufferBytesFromText(const QString &value, const QString &encoding)
{
    const QString normalizedEncoding = normalizedBufferEncoding(encoding);
    if (normalizedEncoding == QStringLiteral("hex"))
        return bytesFromLeadingHex(value);
    if (normalizedEncoding == QStringLiteral("base64"))
        return QByteArray::fromBase64(value.toLatin1());
    if (normalizedEncoding == QStringLiteral("latin1"))
        return value.toLatin1();
    return value.toUtf8();
}

QString scriptBufferTextFromBytes(const QByteArray &bytes, const QString &encoding)
{
    const QString normalizedEncoding = normalizedBufferEncoding(encoding);
    if (normalizedEncoding == QStringLiteral("hex"))
        return QString::fromLatin1(bytes.toHex());
    if (normalizedEncoding == QStringLiteral("base64"))
        return QString::fromLatin1(bytes.toBase64());
    if (normalizedEncoding == QStringLiteral("latin1"))
        return QString::fromLatin1(bytes);
    return QString::fromUtf8(bytes);
}
}

MusicSourceRegistry::MusicSourceRegistry(QObject *parent)
    : QObject(parent)
{
    m_onlineTracks = defaultOnlineTracks();
    loadSources();
}

QVariantList MusicSourceRegistry::sources() const
{
    QVariantList values;
    values.reserve(m_sources.size());
    for (const MusicSourceRecord &record : m_sources)
        values.append(recordToMap(record));
    return values;
}

QStringList MusicSourceRegistry::sourceNames() const
{
    QStringList names;
    names.reserve(m_sources.size());
    for (const MusicSourceRecord &record : m_sources)
        names.append(record.name);
    return names;
}

int MusicSourceRegistry::sourceCount() const
{
    return m_sources.size();
}

QVariantList MusicSourceRegistry::onlineTracks() const
{
    QVariantList values;
    values.reserve(m_onlineTracks.size());
    for (const OnlineTrack &track : m_onlineTracks)
        values.append(onlineTrackToMap(track));
    return values;
}

int MusicSourceRegistry::onlineTrackCount() const
{
    return m_onlineTracks.size();
}

bool MusicSourceRegistry::busy() const
{
    return m_busy;
}

bool MusicSourceRegistry::catalogBusy() const
{
    return m_catalogBusy;
}

bool MusicSourceRegistry::resolving() const
{
    return m_resolving;
}

QString MusicSourceRegistry::statusText() const
{
    return m_statusText;
}

QString MusicSourceRegistry::errorString() const
{
    return m_errorString;
}

void MusicSourceRegistry::importFromText(const QString &sourceText)
{
    if (m_busy)
        return;

    setErrorString({});
    m_lastBatchImported = 0;

    const QStringList entries = entriesFromText(sourceText);
    if (entries.isEmpty()) {
        setErrorString(tr("Paste an LX custom source script URL or script content."));
        return;
    }

    int syncImported = 0;
    for (const QString &entry : entries) {
        const QUrl url = QUrl::fromUserInput(entry);
        if (isRemoteScriptUrl(url)) {
            importRemoteScript(url);
            continue;
        }

        const QFileInfo localFile(entry);
        if (localFile.exists() && localFile.isFile() && localFile.isReadable()) {
            QFile file(localFile.canonicalFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)
                    && importScript(QString::fromUtf8(file.readAll()),
                                    QUrl::fromLocalFile(localFile.canonicalFilePath()))) {
                ++syncImported;
            }
            continue;
        }

        if (looksLikeScriptText(entry) && importScript(entry, {}))
            ++syncImported;
    }

    if (syncImported > 0) {
        saveSources();
        setStatusText(tr("Imported %n music source(s).", nullptr, syncImported));
        emit sourcesChanged();
    }

    if (m_pendingRequests > 0) {
        setBusy(true);
        return;
    }

    if (syncImported == 0)
        setErrorString(tr("No LX custom source script was found."));
}

void MusicSourceRegistry::loadOnlineTracks()
{
    if (m_catalogBusy)
        return;

    setCatalogBusy(true);
    setErrorString({});
    setStatusText(tr("Loading online source tracks."));

    QNetworkRequest request(QUrl(QStringLiteral("https://music.163.com/api/playlist/detail?id=3778678")));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 Aurora/0.5.3");
    QNetworkReply *reply = m_network.get(request);
    QTimer::singleShot(12000, reply, [reply]() {
        if (reply->isRunning())
            reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            setCatalogBusy(false);
            setErrorString(tr("Unable to load online source tracks: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        reply->deleteLater();

        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            setCatalogBusy(false);
            setErrorString(tr("Online source tracks returned invalid JSON."));
            return;
        }

        const QJsonArray tracks = document.object()
                                      .value(QStringLiteral("result"))
                                      .toObject()
                                      .value(QStringLiteral("tracks"))
                                      .toArray();
        QList<OnlineTrack> loadedTracks;
        loadedTracks.reserve(qMin(tracks.size(), 60));
        for (const QJsonValue &trackValue : tracks) {
            const QJsonObject trackObject = trackValue.toObject();
            const QString songId = QString::number(static_cast<qint64>(
                trackObject.value(QStringLiteral("id")).toDouble()));
            const QString title = trackObject.value(QStringLiteral("name")).toString().simplified();
            if (songId == QStringLiteral("0") || title.isEmpty())
                continue;

            const QJsonObject albumObject = trackObject.value(QStringLiteral("album")).toObject();
            loadedTracks.append(OnlineTrack {
                QStringLiteral("wy"),
                songId,
                title,
                artistsText(trackObject.value(QStringLiteral("artists")).toArray()),
                albumObject.value(QStringLiteral("name")).toString().simplified(),
                albumObject.value(QStringLiteral("picUrl")).toString().trimmed(),
            });

            if (loadedTracks.size() >= 60)
                break;
        }

        if (!loadedTracks.isEmpty()) {
            m_onlineTracks = loadedTracks;
            emit onlineTracksChanged();
            setStatusText(tr("Loaded %n online source track(s).", nullptr, m_onlineTracks.size()));
        }

        setCatalogBusy(false);
    });
}

void MusicSourceRegistry::resolveFromText(const QString &requestText)
{
    if (m_resolving)
        return;

    const std::optional<ResolveRequest> request = parseResolveRequest(requestText);
    if (!request.has_value()) {
        setErrorString(tr("Use source:songId, for example wy:33894312 128k."));
        return;
    }

    m_resolvingPlaylist = false;
    m_pendingOnlineTrack.reset();
    m_playlistRequests.clear();
    m_playlistTracks.clear();
    m_playlistResolvedUrls.clear();
    m_playlistResolvedTracks.clear();
    startResolveRequest(request.value());
}

void MusicSourceRegistry::resolveDemoTrack()
{
    resolveFromText(QStringLiteral("wy:33894312 128k"));
}

void MusicSourceRegistry::resolveOnlineTrackAt(int index)
{
    if (m_resolving)
        return;

    const std::optional<ResolveRequest> request = resolveRequestForOnlineTrack(index);
    if (!request.has_value()) {
        setErrorString(tr("Select an online source track."));
        return;
    }

    m_resolvingPlaylist = false;
    m_pendingOnlineTrack = m_onlineTracks.at(index);
    m_playlistRequests.clear();
    m_playlistTracks.clear();
    m_playlistResolvedUrls.clear();
    m_playlistResolvedTracks.clear();
    startResolveRequest(request.value());
}

void MusicSourceRegistry::resolveOnlineTracksFrom(int index)
{
    if (m_resolving)
        return;

    if (index < 0 || index >= m_onlineTracks.size())
        index = 0;

    m_playlistRequests.clear();
    m_playlistTracks.clear();
    m_playlistResolvedUrls.clear();
    m_playlistResolvedTracks.clear();
    for (int i = index; i < m_onlineTracks.size(); ++i) {
        const std::optional<ResolveRequest> request = resolveRequestForOnlineTrack(i);
        if (request.has_value()) {
            m_playlistRequests.append(request.value());
            m_playlistTracks.append(m_onlineTracks.at(i));
        }

        if (m_playlistRequests.size() >= 24)
            break;
    }

    if (m_playlistRequests.isEmpty()) {
        setErrorString(tr("No online source tracks are ready."));
        return;
    }

    m_resolvingPlaylist = true;
    m_pendingOnlineTrack.reset();
    continuePlaylistResolve();
}

void MusicSourceRegistry::clearSources()
{
    if (m_sources.isEmpty())
        return;

    m_sources.clear();
    saveSources();
    setStatusText(tr("Music sources cleared."));
    setErrorString({});
    emit sourcesChanged();
}

QStringList MusicSourceRegistry::entriesFromText(const QString &sourceText)
{
    QStringList entries;
    const QStringList lines = sourceText.split(QRegularExpression(QStringLiteral("[\\r\\n]+")),
                                               Qt::SkipEmptyParts);
    const QRegularExpression urlExpression(QStringLiteral("https?://\\S+"));

    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        const QRegularExpressionMatchIterator matches = urlExpression.globalMatch(line);
        QStringList urls;
        QRegularExpressionMatchIterator iterator = matches;
        while (iterator.hasNext()) {
            QString url = iterator.next().captured(0);
            while (!url.isEmpty() && QStringLiteral("。,.，);）]").contains(url.back()))
                url.chop(1);
            if (!url.isEmpty())
                urls.append(url);
        }

        if (!urls.isEmpty()) {
            entries.append(urls);
            continue;
        }

        entries.append(line);
    }

    entries.removeDuplicates();
    return entries;
}

std::optional<MusicSourceRecord> MusicSourceRegistry::parseScriptMetadata(
    const QString &script,
    const QUrl &scriptUrl)
{
    MusicSourceRecord record;
    record.name = metadataValue(script, QStringLiteral("name"));
    if (record.name.isEmpty())
        return std::nullopt;

    record.version = metadataValue(script, QStringLiteral("version"));
    record.scriptUrl = normalizedScriptUrl(scriptUrl);
    record.sourceId = sourceIdForUrl(record.scriptUrl);
    record.importedAt = nowUtc();
    return record;
}

QString MusicSourceRegistry::sourceIdForUrl(const QUrl &scriptUrl)
{
    const QByteArray identity = scriptUrl.isEmpty()
        ? QByteArray("inline-script")
        : normalizedScriptUrl(scriptUrl).toEncoded();
    const QByteArray digest = QCryptographicHash::hash(
        QByteArray("lx-source-script-v1|") + identity,
        QCryptographicHash::Sha256);
    return QStringLiteral("source:lx-script:v1:") + QString::fromLatin1(digest.toHex());
}

void MusicSourceRegistry::loadSources()
{
    const QString raw = QSettings().value(QString::fromLatin1(settingsKey)).toString();
    const QJsonDocument document = QJsonDocument::fromJson(raw.toUtf8());
    if (!document.isArray())
        return;

    for (const QJsonValue &value : document.array()) {
        if (!value.isObject())
            continue;

        const MusicSourceRecord record = mapToRecord(value.toObject().toVariantMap());
        if (record.sourceId.isEmpty() || record.name.isEmpty())
            continue;

        m_sources.append(record);
    }
}

void MusicSourceRegistry::saveSources() const
{
    QJsonArray values;
    for (const MusicSourceRecord &record : m_sources)
        values.append(QJsonObject::fromVariantMap(recordToMap(record)));

    QSettings().setValue(
        QString::fromLatin1(settingsKey),
        QString::fromUtf8(QJsonDocument(values).toJson(QJsonDocument::Compact)));
}

bool MusicSourceRegistry::importScript(const QString &script, const QUrl &scriptUrl)
{
    const std::optional<MusicSourceRecord> parsed = parseScriptMetadata(script, scriptUrl);
    if (!parsed.has_value())
        return false;

    const MusicSourceRecord record = parsed.value();
    for (MusicSourceRecord &existing : m_sources) {
        if (existing.sourceId == record.sourceId || existing.scriptUrl == record.scriptUrl) {
            existing.name = record.name;
            existing.version = record.version;
            existing.importedAt = record.importedAt;
            return true;
        }
    }

    m_sources.append(record);
    return true;
}

std::optional<MusicSourceRegistry::ResolveRequest> MusicSourceRegistry::parseResolveRequest(
    const QString &requestText) const
{
    const QString text = requestText.simplified();
    if (text.isEmpty())
        return ResolveRequest {
            QStringLiteral("wy"),
            QStringLiteral("33894312"),
            QStringLiteral("128k"),
        };

    const QRegularExpression expression(QStringLiteral(
        R"(^([A-Za-z]{2,4})\s*[:/, ]\s*([A-Za-z0-9_.-]+)(?:\s*[:/, ]\s*([A-Za-z0-9]+))?$)"));
    const QRegularExpressionMatch match = expression.match(text);
    if (!match.hasMatch())
        return std::nullopt;

    ResolveRequest request;
    request.source = match.captured(1).toLower();
    request.songId = match.captured(2);
    request.quality = match.captured(3).isEmpty()
        ? QStringLiteral("128k")
        : match.captured(3);
    return request;
}

std::optional<MusicSourceRegistry::ResolveRequest> MusicSourceRegistry::resolveRequestForOnlineTrack(
    int index) const
{
    if (index < 0 || index >= m_onlineTracks.size())
        return std::nullopt;

    const OnlineTrack &track = m_onlineTracks.at(index);
    if (track.source.isEmpty() || track.songId.isEmpty())
        return std::nullopt;

    return ResolveRequest {
        track.source,
        track.songId,
        QStringLiteral("128k"),
    };
}

void MusicSourceRegistry::startResolveRequest(const ResolveRequest &request)
{
    m_resolveRequest = request;
    m_scriptEngine.reset();
    ++m_resolveGeneration;
    setErrorString({});
    setStatusText(tr("Resolving %1:%2").arg(m_resolveRequest.source, m_resolveRequest.songId));
    setResolving(true);
    resolveWithSourceAt(0);
}

void MusicSourceRegistry::continuePlaylistResolve()
{
    if (!m_resolvingPlaylist)
        return;

    if (m_playlistRequests.isEmpty()) {
        m_resolvingPlaylist = false;
        m_pendingOnlineTrack.reset();
        setResolving(false);
        setErrorString({});
        setStatusText(tr("Resolved %n online source track(s).", nullptr, m_playlistResolvedUrls.size()));
        if (!m_playlistResolvedTracks.isEmpty())
            emit musicTracksResolved(m_playlistResolvedTracks);
        else
            emit musicUrlsResolved(m_playlistResolvedUrls);
        return;
    }

    const ResolveRequest request = m_playlistRequests.takeFirst();
    m_pendingOnlineTrack = m_playlistTracks.isEmpty()
        ? std::optional<OnlineTrack> {}
        : std::optional<OnlineTrack> { m_playlistTracks.takeFirst() };
    startResolveRequest(request);
}

void MusicSourceRegistry::resolveWithSourceAt(int index)
{
    if (!m_resolving)
        return;

    if (index >= m_sources.size()) {
        resolveBuiltInFallbackSource(
            m_errorString.isEmpty()
                ? tr("No imported source could resolve this song.")
                : m_errorString);
        return;
    }

    loadScriptForResolve(m_sources.at(index), index);
}

void MusicSourceRegistry::loadScriptForResolve(const MusicSourceRecord &record, int index)
{
    const QUrl scriptUrl = sourceUrlFromRecord(record);
    if (isReadableLocalScript(scriptUrl)) {
        QFile file(scriptUrl.toLocalFile());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            evaluateScriptForResolve(QString::fromUtf8(file.readAll()), record, index);
            return;
        }
    }

    if (!isRemoteScriptUrl(scriptUrl)) {
        resolveWithSourceAt(index + 1);
        return;
    }

    QNetworkRequest request(scriptUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, record, index]() {
        const bool ok = reply->error() == QNetworkReply::NoError;
        const QString script = ok ? QString::fromUtf8(reply->readAll()) : QString();
        reply->deleteLater();

        if (!ok || script.isEmpty()) {
            resolveWithSourceAt(index + 1);
            return;
        }

        evaluateScriptForResolve(script, record, index);
    });
}

void MusicSourceRegistry::evaluateScriptForResolve(
    const QString &script,
    const MusicSourceRecord &record,
    int index)
{
    if (resolveKnownDirectSource(record, script, index))
        return;

    const int generation = m_resolveGeneration;
    m_scriptEngine = std::make_unique<QJSEngine>();

    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
    QJSValue bridge = m_scriptEngine->newQObject(this);
    m_scriptEngine->globalObject().setProperty(QStringLiteral("_auroraBridge"), bridge);

    const QString prelude = QStringLiteral(R"(
        var globalThis = this;
        function __auroraNullish(left, right) {
            return left === null || left === undefined ? right : left;
        }
        globalThis.console = {
            log: function(){},
            error: function(){},
            group: function(){},
            groupEnd: function(){}
        };
        if (typeof Promise !== "undefined") {
            if (typeof Promise.any !== "function") {
                Promise.any = function(promises) {
                    return new Promise(function(resolve, reject) {
                        var list = Array.prototype.slice.call(promises || []);
                        var pending = list.length;
                        var errors = [];
                        if (pending === 0) {
                            reject(new Error("All promises were rejected"));
                            return;
                        }
                        list.forEach(function(promise, index) {
                            Promise.resolve(promise).then(
                                resolve,
                                function(error) {
                                    errors[index] = error;
                                    pending -= 1;
                                    if (pending === 0)
                                        reject(errors[0] || new Error("All promises were rejected"));
                                }
                            );
                        });
                    });
                };
            }
            if (typeof Promise.prototype.finally !== "function") {
                Promise.prototype.finally = function(onFinally) {
                    return this.then(
                        function(value) {
                            if (typeof onFinally === "function")
                                onFinally();
                            return value;
                        },
                        function(error) {
                            if (typeof onFinally === "function")
                                onFinally();
                            throw error;
                        }
                    );
                };
            }
        }
        globalThis._auroraHandlers = {};
        globalThis.lx = {
            EVENT_NAMES: {
                inited: "inited",
                request: "request",
                updateAlert: "updateAlert"
            },
            request: function(url, options, callback) {
                return _auroraBridge.scriptRequest(url, options || {}, callback);
            },
            on: function(eventName, handler) {
                globalThis._auroraHandlers[eventName] = handler;
            },
            send: function(eventName, payload) {
                return _auroraBridge.scriptSend(eventName, payload);
            },
            env: "desktop",
            version: "0.5.3",
            currentScriptInfo: {
                name: %1,
                version: %2,
                rawScript: %3
            },
            utils: {
                crypto: {
                    md5: function(value) {
                        if (value && value.__auroraBufferBase64 !== undefined)
                            return _auroraBridge.scriptMd5FromBase64(String(value.__auroraBufferBase64));
                        return _auroraBridge.scriptMd5(String(value));
                    }
                },
                buffer: {
                    from: function(value, encoding) {
                        return {
                            __auroraBufferBase64: _auroraBridge.scriptBufferFromBase64(
                                String(value),
                                String(encoding || "utf8"))
                        };
                    },
                    bufToString: function(value, encoding) {
                        if (value && value.__auroraBufferBase64 !== undefined) {
                            return _auroraBridge.scriptBufferToStringFromBase64(
                                String(value.__auroraBufferBase64),
                                String(encoding || "utf8"));
                        }
                        return _auroraBridge.scriptBufferToStringFromBase64(
                            _auroraBridge.scriptBufferFromBase64(String(value), "utf8"),
                            String(encoding || "utf8"));
                    }
                }
            }
        };
    )")
        .arg(jsStringLiteral(record.name),
             jsStringLiteral(record.version),
             jsStringLiteral(script));

    QJSValue preludeResult = m_scriptEngine->evaluate(prelude);
    if (preludeResult.isError()) {
        setErrorString(tr("LX resolver host failed: %1").arg(jsValueText(preludeResult)));
        resolveWithSourceAt(index + 1);
        return;
    }

    QJSValue scriptResult = m_scriptEngine->evaluate(
        scriptForQJSEngine(script),
        record.scriptUrl.toString());
    if (scriptResult.isError()) {
        setErrorString(tr("LX source script failed: %1").arg(jsValueText(scriptResult)));
        resolveWithSourceAt(index + 1);
        return;
    }

    QTimer::singleShot(12000, this, [this, generation, index]() {
        if (m_resolving && generation == m_resolveGeneration)
            resolveWithSourceAt(index + 1);
    });
}

bool MusicSourceRegistry::resolveKnownDirectSource(
    const MusicSourceRecord &,
    const QString &script,
    int index)
{
    const QString apiUrl = scriptStringConstant(script, QStringLiteral("API_URL"));
    if (apiUrl.isEmpty())
        return false;

    QUrl requestUrl;
    int successCode = 0;
    if (apiUrl.contains(QStringLiteral("lxmusicapi.onrender.com"))) {
        requestUrl = QUrl(QStringLiteral("%1/url/%2/%3/%4")
                              .arg(apiUrl,
                                   QString::fromLatin1(QUrl::toPercentEncoding(m_resolveRequest.source)),
                                   QString::fromLatin1(QUrl::toPercentEncoding(m_resolveRequest.songId)),
                                   QString::fromLatin1(QUrl::toPercentEncoding(m_resolveRequest.quality))));
        successCode = 0;
    } else if (apiUrl.contains(QStringLiteral("api.ikunshare.com"))) {
        requestUrl = QUrl(apiUrl + QStringLiteral("/url"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("source"), m_resolveRequest.source);
        query.addQueryItem(QStringLiteral("songId"), m_resolveRequest.songId);
        query.addQueryItem(QStringLiteral("quality"), m_resolveRequest.quality);
        requestUrl.setQuery(query);
        successCode = 200;
    } else {
        return false;
    }

    QNetworkRequest request(requestUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("User-Agent", "lx-music-desktop/0.5.3");

    const QString apiKey = scriptStringConstant(script, QStringLiteral("API_KEY"));
    if (!apiKey.isEmpty())
        request.setRawHeader("X-Request-Key", apiKey.toUtf8());

    const int generation = m_resolveGeneration;
    QNetworkReply *reply = m_network.get(request);
    QTimer::singleShot(12000, reply, [reply]() {
        if (reply->isRunning())
            reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, generation, index, successCode]() {
        if (!m_resolving || generation != m_resolveGeneration) {
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            setErrorString(tr("Direct source request failed: %1").arg(reply->errorString()));
            reply->deleteLater();
            resolveWithSourceAt(index + 1);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        reply->deleteLater();

        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            setErrorString(tr("Direct source returned invalid JSON."));
            resolveWithSourceAt(index + 1);
            return;
        }

        const QJsonObject body = document.object();
        const int code = body.value(QStringLiteral("code")).toInt(-1);
        const QString musicUrl = body.value(QStringLiteral("url")).toString().trimmed();
        if (code == successCode && !musicUrl.isEmpty()) {
            finishCurrentResolver(musicUrl);
            return;
        }

        setErrorString(tr("Direct source rejected playback: %1").arg(jsonErrorText(body)));
        resolveWithSourceAt(index + 1);
    });

    return true;
}

void MusicSourceRegistry::resolveBuiltInFallbackSource(const QString &previousError)
{
    if (!m_resolving)
        return;

    setStatusText(tr("Resolving with built-in fallback %1:%2")
                      .arg(m_resolveRequest.source, m_resolveRequest.songId));

    const QUrl requestUrl(QStringLiteral("https://lxmusicapi.onrender.com/url/%1/%2/%3")
                              .arg(QString::fromLatin1(QUrl::toPercentEncoding(m_resolveRequest.source)),
                                   QString::fromLatin1(QUrl::toPercentEncoding(m_resolveRequest.songId)),
                                   QString::fromLatin1(QUrl::toPercentEncoding(m_resolveRequest.quality))));

    QNetworkRequest request(requestUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("User-Agent", "lx-music-desktop/0.5.3");
    request.setRawHeader("X-Request-Key", "share-v3");

    const int generation = m_resolveGeneration;
    QNetworkReply *reply = m_network.get(request);
    QTimer::singleShot(12000, reply, [reply]() {
        if (reply->isRunning())
            reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, generation, previousError]() {
        if (!m_resolving || generation != m_resolveGeneration) {
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            const QString fallbackError = tr("Built-in fallback request failed: %1")
                                              .arg(reply->errorString());
            reply->deleteLater();
            failCurrentResolver(previousError.isEmpty()
                                    ? fallbackError
                                    : tr("%1; %2").arg(previousError, fallbackError));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        reply->deleteLater();

        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            const QString fallbackError = tr("Built-in fallback returned invalid JSON.");
            failCurrentResolver(previousError.isEmpty()
                                    ? fallbackError
                                    : tr("%1; %2").arg(previousError, fallbackError));
            return;
        }

        const QJsonObject body = document.object();
        const QString musicUrl = body.value(QStringLiteral("url")).toString().trimmed();
        if (body.value(QStringLiteral("code")).toInt(-1) == 0 && !musicUrl.isEmpty()) {
            finishCurrentResolver(musicUrl);
            return;
        }

        const QString fallbackError = tr("Built-in fallback rejected playback: %1")
                                          .arg(jsonErrorText(body));
        failCurrentResolver(previousError.isEmpty()
                                ? fallbackError
                                : tr("%1; %2").arg(previousError, fallbackError));
    });
}

void MusicSourceRegistry::requestCurrentMusicUrl()
{
    if (!m_resolving || !m_scriptEngine)
        return;

    QJSValue handlers = m_scriptEngine->globalObject().property(QStringLiteral("_auroraHandlers"));
    QJSValue requestHandler = handlers.property(QStringLiteral("request"));
    if (!requestHandler.isCallable()) {
        failCurrentResolver(tr("LX source did not register a playback resolver."));
        return;
    }

    QJSValue request = m_scriptEngine->newObject();
    request.setProperty(QStringLiteral("source"), m_resolveRequest.source);
    request.setProperty(QStringLiteral("action"), QStringLiteral("musicUrl"));

    QJSValue info = m_scriptEngine->newObject();
    info.setProperty(QStringLiteral("type"), m_resolveRequest.quality);

    QJSValue musicInfo = m_scriptEngine->newObject();
    musicInfo.setProperty(QStringLiteral("songmid"), m_resolveRequest.songId);
    musicInfo.setProperty(QStringLiteral("hash"), m_resolveRequest.songId);
    info.setProperty(QStringLiteral("musicInfo"), musicInfo);
    request.setProperty(QStringLiteral("info"), info);

    QJSValue promise = requestHandler.call({request});
    if (promise.isError()) {
        scriptReject(promise);
        return;
    }

    m_scriptEngine->globalObject().setProperty(QStringLiteral("_auroraPromise"), promise);
    const QJSValue hookResult = m_scriptEngine->evaluate(QStringLiteral(R"(
        if (_auroraPromise && typeof _auroraPromise.then === "function") {
            _auroraPromise.then(
                function(value) { _auroraBridge.scriptResolve(value); },
                function(error) { _auroraBridge.scriptReject(error); }
            );
        } else {
            _auroraBridge.scriptResolve(_auroraPromise);
        }
    )"));
    if (hookResult.isError())
        scriptReject(hookResult);
}

void MusicSourceRegistry::failCurrentResolver(const QString &message)
{
    ++m_resolveGeneration;
    if (QJSEngine *engine = m_scriptEngine.release())
        engine->deleteLater();
    m_pendingOnlineTrack.reset();

    if (m_resolvingPlaylist && !m_playlistRequests.isEmpty()) {
        setErrorString(message);
        setStatusText(tr("Skipping unavailable online track."));
        QTimer::singleShot(0, this, &MusicSourceRegistry::continuePlaylistResolve);
        return;
    }

    if (m_resolvingPlaylist && !m_playlistResolvedUrls.isEmpty()) {
        m_resolvingPlaylist = false;
        setResolving(false);
        setErrorString({});
        setStatusText(tr("Resolved %n online source track(s).", nullptr, m_playlistResolvedUrls.size()));
        if (!m_playlistResolvedTracks.isEmpty())
            emit musicTracksResolved(m_playlistResolvedTracks);
        else
            emit musicUrlsResolved(m_playlistResolvedUrls);
        return;
    }

    m_resolvingPlaylist = false;
    setResolving(false);
    setErrorString(message);
}

void MusicSourceRegistry::finishCurrentResolver(const QString &musicUrl)
{
    ++m_resolveGeneration;
    if (QJSEngine *engine = m_scriptEngine.release())
        engine->deleteLater();

    QVariantMap resolvedTrack;
    if (m_pendingOnlineTrack.has_value())
        resolvedTrack = resolvedOnlineTrackToMap(m_pendingOnlineTrack.value(), musicUrl);
    m_pendingOnlineTrack.reset();

    if (m_resolvingPlaylist) {
        m_playlistResolvedUrls.append(musicUrl);
        if (!resolvedTrack.isEmpty())
            m_playlistResolvedTracks.append(resolvedTrack);
        if (!m_playlistRequests.isEmpty()) {
            QTimer::singleShot(0, this, &MusicSourceRegistry::continuePlaylistResolve);
            return;
        }

        m_resolvingPlaylist = false;
        setResolving(false);
        setErrorString({});
        setStatusText(tr("Resolved %n online source track(s).", nullptr, m_playlistResolvedUrls.size()));
        if (!m_playlistResolvedTracks.isEmpty())
            emit musicTracksResolved(m_playlistResolvedTracks);
        else
            emit musicUrlsResolved(m_playlistResolvedUrls);
        return;
    }

    setResolving(false);
    setErrorString({});
    setStatusText(tr("Resolved playback URL."));
    if (!resolvedTrack.isEmpty())
        emit musicTracksResolved(QVariantList { resolvedTrack });
    else
        emit musicUrlResolved(musicUrl);
}

void MusicSourceRegistry::importRemoteScript(const QUrl &scriptUrl)
{
    ++m_pendingRequests;

    QNetworkRequest request(scriptUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, scriptUrl]() {
        const bool networkOk = reply->error() == QNetworkReply::NoError;
        const QString script = networkOk ? QString::fromUtf8(reply->readAll()) : QString();
        reply->deleteLater();

        const bool imported = networkOk && importScript(script, scriptUrl);
        if (!networkOk)
            setErrorString(tr("Unable to download one music source script."));
        else if (!imported)
            setErrorString(tr("The downloaded script is not a valid LX music source."));

        finishRemoteImport(imported);
    });
}

void MusicSourceRegistry::finishRemoteImport(bool imported)
{
    if (imported)
        ++m_lastBatchImported;

    m_pendingRequests = qMax(0, m_pendingRequests - 1);
    if (m_pendingRequests > 0)
        return;

    setBusy(false);

    if (m_lastBatchImported > 0) {
        saveSources();
        setStatusText(tr("Imported %n music source(s).", nullptr, m_lastBatchImported));
        emit sourcesChanged();
    } else if (m_errorString.isEmpty()) {
        setErrorString(tr("No LX custom source script was imported."));
    }
}

void MusicSourceRegistry::setBusy(bool busy)
{
    if (m_busy == busy)
        return;

    m_busy = busy;
    emit busyChanged();
}

void MusicSourceRegistry::setCatalogBusy(bool busy)
{
    if (m_catalogBusy == busy)
        return;

    m_catalogBusy = busy;
    emit catalogBusyChanged();
}

void MusicSourceRegistry::setResolving(bool resolving)
{
    if (m_resolving == resolving)
        return;

    m_resolving = resolving;
    emit resolvingChanged();
}

void MusicSourceRegistry::setStatusText(const QString &message)
{
    if (m_statusText == message)
        return;

    m_statusText = message;
    emit statusChanged();
}

void MusicSourceRegistry::setErrorString(const QString &message)
{
    if (m_errorString == message)
        return;

    m_errorString = message;
    emit statusChanged();
}

void MusicSourceRegistry::scriptSend(const QString &eventName, const QJSValue &)
{
    if (eventName == QStringLiteral("inited"))
        QTimer::singleShot(0, this, &MusicSourceRegistry::requestCurrentMusicUrl);
}

void MusicSourceRegistry::scriptRequest(
    const QString &url,
    const QJSValue &options,
    const QJSValue &callback)
{
    if (!m_scriptEngine || !callback.isCallable())
        return;

    QNetworkRequest request { QUrl(url) };
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    const QVariantMap headers = jsObjectToMap(options.property(QStringLiteral("headers")));
    for (auto iterator = headers.cbegin(); iterator != headers.cend(); ++iterator)
        request.setRawHeader(iterator.key().toUtf8(), iterator.value().toString().toUtf8());

    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() mutable {
        if (!m_scriptEngine) {
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            callback.call({QJSValue(reply->errorString()), QJSValue(QJSValue::NullValue)});
            reply->deleteLater();
            return;
        }

        QJSValue response = m_scriptEngine->newObject();
        response.setProperty(
            QStringLiteral("statusCode"),
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        response.setProperty(QStringLiteral("body"), jsonValueToScriptValue(m_scriptEngine.get(), reply->readAll()));
        callback.call({QJSValue(QJSValue::NullValue), response});
        reply->deleteLater();
    });
}

void MusicSourceRegistry::scriptResolve(const QJSValue &value)
{
    const QString musicUrl = value.toString().trimmed();
    const QUrl url = QUrl::fromUserInput(musicUrl);
    if (!url.isValid() || url.scheme().isEmpty()) {
        scriptReject(QJSValue(QStringLiteral("Resolved value is not a playable URL.")));
        return;
    }

    finishCurrentResolver(musicUrl);
}

void MusicSourceRegistry::scriptReject(const QJSValue &value)
{
    failCurrentResolver(jsValueText(value));
}

QString MusicSourceRegistry::scriptMd5(const QString &value) const
{
    const QByteArray digest = QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Md5);
    return QString::fromLatin1(digest.toHex());
}

QString MusicSourceRegistry::scriptMd5FromBase64(const QString &base64Value) const
{
    const QByteArray bytes = QByteArray::fromBase64(base64Value.toLatin1());
    const QByteArray digest = QCryptographicHash::hash(bytes, QCryptographicHash::Md5);
    return QString::fromLatin1(digest.toHex());
}

QString MusicSourceRegistry::scriptBufferFromBase64(
    const QString &value,
    const QString &encoding) const
{
    return QString::fromLatin1(scriptBufferBytesFromText(value, encoding).toBase64());
}

QString MusicSourceRegistry::scriptBufferToStringFromBase64(
    const QString &base64Value,
    const QString &encoding) const
{
    return scriptBufferTextFromBytes(
        QByteArray::fromBase64(base64Value.toLatin1()),
        encoding);
}
