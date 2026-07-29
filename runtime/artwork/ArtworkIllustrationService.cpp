#include "runtime/artwork/ArtworkIllustrationService.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QTimer>
#include <QVector>
#include <QVariantMap>

#include <algorithm>
#include <cmath>

namespace {
constexpr int artworkTimeoutMs = 20000;
constexpr qint64 maximumArtworkDownloadBytes = 16LL * 1024LL * 1024LL;
constexpr qint64 maximumArtworkCacheBytes = 256LL * 1024LL * 1024LL;
constexpr qint64 maximumIllustrationCacheBytes = 384LL * 1024LL * 1024LL;

QString cacheRoot()
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (root.isEmpty())
        root = QDir::home().filePath(QStringLiteral(".cache/Aurora"));
    QDir directory;
    directory.mkpath(root);
    return root;
}

QString hashText(const QString &value)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool isRemoteArtwork(const QUrl &url)
{
    const QString scheme = url.scheme().toCaseFolded();
    return (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"))
        && !url.host().isEmpty();
}

QString readablePathForUrl(const QUrl &url)
{
    if (url.isLocalFile())
        return url.toLocalFile();
    if (url.scheme() == QStringLiteral("qrc"))
        return QStringLiteral(":") + url.path();
    if (url.scheme().isEmpty())
        return url.toString();
    return {};
}

int bayer4(int x, int y)
{
    static constexpr int matrix[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}
    };
    return matrix[y & 3][x & 3];
}
}

ArtworkIllustrationService::ArtworkIllustrationService(QObject *parent)
    : QObject(parent),
      m_network(new QNetworkAccessManager(this))
{
}

QUrl ArtworkIllustrationService::illustrationSource() const
{
    return m_illustrationSource;
}

bool ArtworkIllustrationService::ready() const
{
    return m_ready;
}

bool ArtworkIllustrationService::loading() const
{
    return m_loading;
}

bool ArtworkIllustrationService::fallbackRequired() const
{
    return !m_ready;
}

qreal ArtworkIllustrationService::focalX() const
{
    return m_focalX;
}

qreal ArtworkIllustrationService::focalY() const
{
    return m_focalY;
}

qreal ArtworkIllustrationService::edgeDensity() const
{
    return m_edgeDensity;
}

qreal ArtworkIllustrationService::contrast() const
{
    return m_contrast;
}

QVariantList ArtworkIllustrationService::particleSamples() const
{
    return m_particleSamples;
}

QString ArtworkIllustrationService::errorString() const
{
    return m_errorString;
}

QString ArtworkIllustrationService::cacheDirectory() const
{
    return QDir(cacheRoot()).filePath(QStringLiteral("illustrations"));
}

void ArtworkIllustrationService::load(
    const QUrl &artworkSource,
    const QString &trackIdentity)
{
    const QUrl normalized = artworkSource.adjusted(
        QUrl::NormalizePathSegments | QUrl::RemovePassword);
    const QString identity = trackIdentity.trimmed();

    if (normalized == m_currentArtworkSource
        && identity == m_currentTrackIdentity
        && (m_ready || m_loading)) {
        return;
    }

    ++m_generation;
    cancelActiveRequest();

    m_currentArtworkSource = normalized;
    m_currentTrackIdentity = identity;
    m_illustrationSource = QUrl{};
    m_ready = false;
    m_loading = false;
    m_focalX = 0.5;
    m_focalY = 0.5;
    m_edgeDensity = 0.0;
    m_contrast = 0.0;
    m_particleSamples.clear();
    m_errorString.clear();
    emit stateChanged();

    if (!normalized.isValid() || normalized.isEmpty())
        return;

    if (isRemoteArtwork(normalized)) {
        beginRemoteLoad(normalized, identity);
        return;
    }

    if (!loadLocalOrResource(normalized, identity))
        setErrorString(tr("Album artwork could not be read."));
}

void ArtworkIllustrationService::clear()
{
    ++m_generation;
    cancelActiveRequest();

    const bool changed = m_ready
        || m_loading
        || !m_illustrationSource.isEmpty()
        || !m_errorString.isEmpty()
        || !m_currentArtworkSource.isEmpty();

    m_illustrationSource = QUrl{};
    m_currentArtworkSource = QUrl{};
    m_currentTrackIdentity.clear();
    m_errorString.clear();
    m_focalX = 0.5;
    m_focalY = 0.5;
    m_edgeDensity = 0.0;
    m_contrast = 0.0;
    m_particleSamples.clear();
    m_ready = false;
    m_loading = false;

    if (changed)
        emit stateChanged();
}

void ArtworkIllustrationService::cancelActiveRequest()
{
    QNetworkReply *reply = m_activeReply.data();
    if (!reply)
        return;

    m_activeReply.clear();
    QObject::disconnect(reply, nullptr, this, nullptr);
    reply->abort();
    reply->deleteLater();
}

void ArtworkIllustrationService::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    emit stateChanged();
}

void ArtworkIllustrationService::setErrorString(const QString &message)
{
    if (m_errorString == message && !m_loading)
        return;
    m_errorString = message;
    m_loading = false;
    m_ready = false;
    m_illustrationSource = QUrl{};
    m_particleSamples.clear();
    emit stateChanged();
}

bool ArtworkIllustrationService::loadLocalOrResource(
    const QUrl &source,
    const QString &trackIdentity)
{
    const QString path = readablePathForUrl(source);
    if (path.isEmpty())
        return false;

    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull())
        return false;

    return processImage(image, source, trackIdentity);
}

void ArtworkIllustrationService::beginRemoteLoad(
    const QUrl &source,
    const QString &trackIdentity)
{
    const QString cachedOriginal = originalCachePath(source);
    if (QFileInfo::exists(cachedOriginal)) {
        QImageReader reader(cachedOriginal);
        reader.setAutoTransform(true);
        const QImage cachedImage = reader.read();
        if (!cachedImage.isNull()
            && processImage(cachedImage, source, trackIdentity)) {
            return;
        }
        QFile::remove(cachedOriginal);
    }

    setLoading(true);
    const int generation = m_generation;

    QNetworkRequest request(source);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    // AUR-SOURCE-ARTWORK-RECOVERY-PACK-01:ARTWORK-HEADERS
    request.setAttribute(
        QNetworkRequest::CacheLoadControlAttribute,
        QNetworkRequest::PreferCache);
    request.setRawHeader(
        "User-Agent",
        "Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 Aurora/1.0");
    request.setRawHeader(
        "Accept",
        "image/avif,image/webp,image/apng,image/*,*/*;q=0.8");
    request.setRawHeader(
        "Accept-Language",
        "zh-CN,zh;q=0.9,en;q=0.7");
    request.setRawHeader(
        "Referer",
        "https://music.163.com/");

    QNetworkReply *reply = m_network->get(request);
    m_activeReply = reply;

    connect(reply, &QNetworkReply::downloadProgress,
            this, [reply](qint64 received, qint64) {
        if (received > maximumArtworkDownloadBytes)
            reply->abort();
    });

    QTimer::singleShot(artworkTimeoutMs, reply, [reply] {
        if (reply->isRunning())
            reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, generation, source, trackIdentity, cachedOriginal] {
        if (m_activeReply == reply)
            m_activeReply.clear();

        const bool current = generation == m_generation
            && source == m_currentArtworkSource
            && trackIdentity == m_currentTrackIdentity;

        const QByteArray bytes = reply->error() == QNetworkReply::NoError
            ? reply->readAll()
            : QByteArray();
        reply->deleteLater();

        if (!current)
            return;

        if (bytes.isEmpty() || bytes.size() > maximumArtworkDownloadBytes) {
            setErrorString(tr("Album artwork download failed."));
            return;
        }

        QImage image;
        if (!image.loadFromData(bytes)) {
            setErrorString(tr("Album artwork format is unsupported."));
            return;
        }

        QDir().mkpath(QFileInfo(cachedOriginal).absolutePath());
        image.save(cachedOriginal, "PNG");
        pruneCacheDirectory(
            QFileInfo(cachedOriginal).absolutePath(),
            maximumArtworkCacheBytes);

        if (!processImage(image, source, trackIdentity))
            setErrorString(tr("Album artwork illustration could not be generated."));
    });
}

bool ArtworkIllustrationService::processImage(
    const QImage &sourceImage,
    const QUrl &source,
    const QString &trackIdentity)
{
    if (sourceImage.isNull())
        return false;

    const QSize bounded = sourceImage.size().boundedTo(QSize(1280, 1280));
    if (bounded.isEmpty())
        return false;

    QImage image = sourceImage.convertToFormat(QImage::Format_RGB32);
    if (image.size() != bounded) {
        image = image.scaled(
            bounded,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    }

    const int width = image.width();
    const int height = image.height();
    if (width < 8 || height < 8)
        return false;

    QVector<int> luminance(width * height);
    double sum = 0.0;
    double sumSquares = 0.0;

    for (int y = 0; y < height; ++y) {
        const QRgb *row =
            reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < width; ++x) {
            const int value = qGray(row[x]);
            luminance[y * width + x] = value;
            sum += value;
            sumSquares += static_cast<double>(value) * value;
        }
    }

    const double count = static_cast<double>(width) * height;
    const double mean = sum / count;
    const double variance =
        std::max(0.0, sumSquares / count - mean * mean);
    const double deviation = std::sqrt(variance);
    const double contrastScale =
        86.0 / std::max(30.0, deviation * 1.35);

    QVector<quint8> featureMask(width * height, 0);
    QVector<quint8> toneMap(width * height, 248);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            const auto at = [&](int dx, int dy) {
                return luminance[(y + dy) * width + (x + dx)];
            };

            const int gx =
                -at(-1, -1) + at(1, -1)
                -2 * at(-1, 0) + 2 * at(1, 0)
                -at(-1, 1) + at(1, 1);
            const int gy =
                -at(-1, -1) - 2 * at(0, -1) - at(1, -1)
                +at(-1, 1) + 2 * at(0, 1) + at(1, 1);

            const int edge = std::min(
                255,
                (std::abs(gx) + std::abs(gy)) / 5);
            const int localAverage =
                (at(-1, 0) + at(1, 0)
                 + at(0, -1) + at(0, 1)) / 4;
            const int localContrast =
                std::abs(at(0, 0) - localAverage);

            const int normalized = std::clamp(
                qRound(
                    128.0
                    + (at(0, 0) - mean)
                      * contrastScale),
                0,
                255);
            const int darkness = 255 - normalized;

            int tone = 248;
            if (normalized < 62)
                tone = 34;
            else if (normalized < 104)
                tone = 76;
            else if (normalized < 146)
                tone = 132;
            else if (normalized < 184)
                tone = 190;

            if (edge > 18) {
                const int edgeTone = std::clamp(
                    22 + (255 - edge) / 3,
                    22,
                    112);
                tone = std::min(tone, edgeTone);
            }

            if (localContrast > 18 && normalized < 205) {
                tone = std::min(
                    tone,
                    std::clamp(
                        156 - localContrast * 2,
                        42,
                        156));
            }

            const int feature = std::clamp(
                qRound(
                    edge * 0.78
                    + localContrast * 1.35
                    + darkness * 0.24),
                0,
                255);

            toneMap[y * width + x] =
                static_cast<quint8>(tone);
            featureMask[y * width + x] =
                static_cast<quint8>(feature);
        }
    }

    QImage result(
        width,
        height,
        QImage::Format_ARGB32_Premultiplied);
    result.fill(qRgba(250, 246, 237, 255));

    double weightedX = 0.0;
    double weightedY = 0.0;
    double totalWeight = 0.0;
    qint64 visiblePixels = 0;

    for (int y = 1; y < height - 1; ++y) {
        QRgb *target =
            reinterpret_cast<QRgb *>(result.scanLine(y));

        for (int x = 1; x < width - 1; ++x) {
            const int tone =
                toneMap[y * width + x];
            const int feature =
                featureMask[y * width + x];

            target[x] = qRgba(
                tone,
                tone,
                std::max(18, tone - 4),
                255);

            if (tone < 225)
                ++visiblePixels;

            if (feature > 30) {
                const double weight =
                    feature
                    + (255 - tone) * 0.34;
                weightedX += x * weight;
                weightedY += y * weight;
                totalWeight += weight;
            }
        }
    }

    QVariantList particles;
    particles.reserve(1800);

    quint32 identitySeed = 2166136261u;
    const QString seedText =
        source.toString()
        + QLatin1Char('|')
        + trackIdentity;

    for (const QChar character : seedText) {
        identitySeed ^= character.unicode();
        identitySeed *= 16777619u;
    }

    const int sampleStride = std::max(
        2,
        qRound(std::sqrt(
            static_cast<double>(width)
            * height
            / 30000.0)));

    for (int y = sampleStride / 2;
         y < height && particles.size() < 1800;
         y += sampleStride) {
        for (int x = sampleStride / 2;
             x < width && particles.size() < 1800;
             x += sampleStride) {
            const int feature =
                featureMask[y * width + x];
            const int tone =
                toneMap[y * width + x];

            if (feature < 62 || tone > 220)
                continue;

            quint32 mixed = identitySeed;
            mixed ^=
                static_cast<quint32>(x)
                * 374761393u;
            mixed =
                (mixed << 13)
                | (mixed >> 19);
            mixed ^=
                static_cast<quint32>(y)
                * 668265263u;
            mixed *= 2246822519u;

            const int acceptance =
                22 + feature * 62 / 255;
            if (static_cast<int>(mixed % 100u)
                >= acceptance) {
                continue;
            }

            QVariantMap particle;
            particle.insert(
                QStringLiteral("x"),
                (static_cast<qreal>(x) + 0.5)
                    / width);
            particle.insert(
                QStringLiteral("y"),
                (static_cast<qreal>(y) + 0.5)
                    / height);
            particle.insert(
                QStringLiteral("alpha"),
                std::clamp(
                    feature / 255.0,
                    0.22,
                    1.0));
            particle.insert(
                QStringLiteral("size"),
                1.0
                    + std::clamp(
                        feature / 255.0,
                        0.0,
                        1.0)
                        * 2.45
                    + static_cast<qreal>(
                        (mixed >> 8) & 7u)
                        * 0.18);
            particle.insert(
                QStringLiteral("drift"),
                static_cast<qreal>(
                    (mixed >> 16) & 1023u)
                    / 1023.0);
            particle.insert(
                QStringLiteral("tone"),
                static_cast<qreal>(tone) / 255.0);
            particles.append(particle);
        }
    }

    const QString outputPath =
        illustrationCachePath(
            source,
            trackIdentity);

    QDir().mkpath(
        QFileInfo(outputPath).absolutePath());

    if (!result.save(outputPath, "PNG"))
        return false;

    m_focalX = totalWeight > 0.0
        ? std::clamp(
            weightedX / totalWeight / width,
            0.10,
            0.90)
        : 0.5;
    m_focalY = totalWeight > 0.0
        ? std::clamp(
            weightedY / totalWeight / height,
            0.10,
            0.90)
        : 0.5;
    m_edgeDensity = std::clamp(
        static_cast<double>(visiblePixels)
            / count
            * 2.15,
        0.0,
        1.0);
    m_contrast = std::clamp(
        0.52 + deviation / 92.0,
        0.0,
        1.0);
    m_particleSamples = particles;
    m_illustrationSource =
        QUrl::fromLocalFile(outputPath);
    m_errorString.clear();
    m_loading = false;
    m_ready = true;
    emit stateChanged();

    pruneCacheDirectory(
        QFileInfo(outputPath).absolutePath(),
        maximumIllustrationCacheBytes);
    return true;
}

QString ArtworkIllustrationService::originalCachePath(const QUrl &source) const
{
    const QString directory = QDir(cacheRoot()).filePath(QStringLiteral("artwork-remote"));
    return QDir(directory).filePath(hashText(source.toString()) + QStringLiteral(".png"));
}

QString ArtworkIllustrationService::illustrationCachePath(
    const QUrl &source,
    const QString &trackIdentity) const
{
    const QString directory = QDir(cacheRoot()).filePath(QStringLiteral("illustrations"));
    const QString key = source.toString() + QLatin1Char('|') + trackIdentity
        + QStringLiteral("|artwork-mirror-v3-shards");
    return QDir(directory).filePath(hashText(key) + QStringLiteral(".png"));
}

void ArtworkIllustrationService::pruneCacheDirectory(
    const QString &directoryPath,
    qint64 maximumBytes) const
{
    QDir directory(directoryPath);
    const QFileInfoList files = directory.entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Time | QDir::Reversed);

    qint64 total = 0;
    for (const QFileInfo &file : files)
        total += file.size();

    for (const QFileInfo &file : files) {
        if (total <= maximumBytes)
            break;
        const qint64 size = file.size();
        if (QFile::remove(file.absoluteFilePath()))
            total -= size;
    }
}
