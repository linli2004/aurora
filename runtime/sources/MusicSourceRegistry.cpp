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
