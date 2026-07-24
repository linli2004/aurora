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

    if (text.isEmpty())
        text = value.property(QStringLiteral("message")).toString();

    if (text.isEmpty())
        text = value.toString();

    const int lineNumber = value.property(QStringLiteral("lineNumber")).toInt();
    if (lineNumber > 0)
        text += QStringLiteral(" at line %1").arg(lineNumber);

    const QString stack = value.property(QStringLiteral("stack")).toString();
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
}

MusicSourceRegistry::MusicSourceRegistry(QObject *parent)
    : QObject(parent)
{
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

bool MusicSourceRegistry::busy() const
{
    return m_busy;
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

void MusicSourceRegistry::resolveFromText(const QString &requestText)
{
    if (m_resolving)
        return;

    if (m_sources.isEmpty()) {
        setErrorString(tr("Import an LX music source before resolving playback."));
        return;
    }

    const std::optional<ResolveRequest> request = parseResolveRequest(requestText);
    if (!request.has_value()) {
        setErrorString(tr("Use source:songId, for example wy:33894312 128k."));
        return;
    }

    m_resolveRequest = request.value();
    m_scriptEngine.reset();
    ++m_resolveGeneration;
    setErrorString({});
    setStatusText(tr("Resolving %1:%2").arg(m_resolveRequest.source, m_resolveRequest.songId));
    setResolving(true);
    resolveWithSourceAt(0);
}

void MusicSourceRegistry::resolveDemoTrack()
{
    resolveFromText(QStringLiteral("wy:33894312 128k"));
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

void MusicSourceRegistry::resolveWithSourceAt(int index)
{
    if (!m_resolving)
        return;

    if (index >= m_sources.size()) {
        failCurrentResolver(
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
                    md5: function(value) { return _auroraBridge.scriptMd5(String(value)); }
                },
                buffer: {
                    from: function(value) { return String(value); },
                    bufToString: function(value) { return String(value); }
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
    setResolving(false);
    setErrorString(message);
}

void MusicSourceRegistry::finishCurrentResolver(const QString &musicUrl)
{
    ++m_resolveGeneration;
    if (QJSEngine *engine = m_scriptEngine.release())
        engine->deleteLater();
    setResolving(false);
    setErrorString({});
    setStatusText(tr("Resolved playback URL."));
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
