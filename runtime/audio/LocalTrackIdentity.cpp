#include "runtime/audio/LocalTrackIdentity.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QStandardPaths>
#include <QtGlobal>

#include <cmath>

namespace {
QString defaultArtworkCacheRoot()
{
    const QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheLocation.isEmpty())
        return {};
    return QDir(cacheLocation).filePath(QStringLiteral("artwork"));
}
}

LocalTrackIdentity LocalTrackIdentityResolver::fallbackFor(const QUrl &source)
{
    LocalTrackIdentity identity;
    const QFileInfo fileInfo(source.toLocalFile());
    identity.title = normalizedText(fileInfo.completeBaseName());
    if (identity.title.isEmpty())
        identity.title = QStringLiteral("Unknown track");
    identity.artist = QStringLiteral("Local audio");
    identity.provenance = QStringLiteral("Filename fallback · Generated identity");
    return identity;
}

LocalTrackIdentity LocalTrackIdentityResolver::resolve(
    const QUrl &source,
    const QMediaMetaData &metaData,
    const QString &artworkCacheRoot)
{
    LocalTrackIdentity identity = fallbackFor(source);

    const QString metadataTitle = normalizedText(metaData.stringValue(QMediaMetaData::Title));
    if (!metadataTitle.isEmpty())
        identity.title = metadataTitle;

    const QString metadataArtist = firstMetadataText(
        metaData,
        {
            QMediaMetaData::ContributingArtist,
            QMediaMetaData::LeadPerformer,
            QMediaMetaData::Author,
            QMediaMetaData::AlbumArtist,
        });
    if (!metadataArtist.isEmpty())
        identity.artist = metadataArtist;

    identity.album = normalizedText(metaData.stringValue(QMediaMetaData::AlbumTitle));
    identity.trackNumber = qMax(0, metaData.value(QMediaMetaData::TrackNumber).toInt());

    const QImage artwork = embeddedArtwork(metaData);
    if (!artwork.isNull()) {
        identity.artworkSource = cacheArtwork(source, artwork, artworkCacheRoot);
        identity.identityColor = representativeColor(artwork);
        identity.hasEmbeddedArtwork = !identity.artworkSource.isEmpty();
    }

    identity.metadataAvailable = !metadataTitle.isEmpty()
        || !metadataArtist.isEmpty()
        || !identity.album.isEmpty()
        || identity.trackNumber > 0
        || !artwork.isNull();

    if (identity.hasEmbeddedArtwork)
        identity.provenance = QStringLiteral("Embedded artwork · Local metadata");
    else if (identity.metadataAvailable)
        identity.provenance = QStringLiteral("Local metadata · Generated identity");

    return identity;
}

QString LocalTrackIdentityResolver::normalizedText(const QString &value)
{
    return value.simplified();
}

QString LocalTrackIdentityResolver::firstMetadataText(
    const QMediaMetaData &metaData,
    std::initializer_list<QMediaMetaData::Key> keys)
{
    for (const QMediaMetaData::Key key : keys) {
        const QVariant rawValue = metaData.value(key);
        QString value;

        if (rawValue.metaType() == QMetaType::fromType<QStringList>()) {
            QStringList normalizedValues;
            const QStringList values = rawValue.toStringList();
            normalizedValues.reserve(values.size());

            for (const QString &entry : values) {
                const QString normalizedEntry = normalizedText(entry);
                if (!normalizedEntry.isEmpty())
                    normalizedValues.append(normalizedEntry);
            }

            value = normalizedValues.join(QStringLiteral(", "));
        } else {
            value = normalizedText(rawValue.toString());
        }

        if (!value.isEmpty())
            return value;
    }
    return {};
}

QImage LocalTrackIdentityResolver::embeddedArtwork(const QMediaMetaData &metaData)
{
    QImage image = qvariant_cast<QImage>(metaData.value(QMediaMetaData::CoverArtImage));
    if (image.isNull())
        image = qvariant_cast<QImage>(metaData.value(QMediaMetaData::ThumbnailImage));
    return image;
}

QUrl LocalTrackIdentityResolver::cacheArtwork(
    const QUrl &source,
    const QImage &image,
    const QString &artworkCacheRoot)
{
    if (image.isNull())
        return {};

    const QString rootPath = artworkCacheRoot.isEmpty()
        ? defaultArtworkCacheRoot()
        : artworkCacheRoot;
    if (rootPath.isEmpty())
        return {};

    QDir directory;
    if (!directory.mkpath(rootPath))
        return {};

    const QFileInfo sourceInfo(source.toLocalFile());
    QByteArray identityKey = source.adjusted(QUrl::NormalizePathSegments).toEncoded();
    identityKey.append('|');
    identityKey.append(QByteArray::number(sourceInfo.size()));
    identityKey.append('|');
    identityKey.append(QByteArray::number(sourceInfo.lastModified().toMSecsSinceEpoch()));

    const QString fileName = QString::fromLatin1(
        QCryptographicHash::hash(identityKey, QCryptographicHash::Sha256).toHex())
        + QStringLiteral(".png");
    const QString filePath = QDir(rootPath).filePath(fileName);

    if (!QFileInfo::exists(filePath) && !image.save(filePath, "PNG"))
        return {};

    return QUrl::fromLocalFile(filePath);
}

QColor LocalTrackIdentityResolver::representativeColor(const QImage &image)
{
    if (image.isNull())
        return {};

    const QImage sample = image.convertToFormat(QImage::Format_ARGB32)
                              .scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;
    double weightSum = 0.0;

    for (int y = 0; y < sample.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgb *>(sample.constScanLine(y));
        for (int x = 0; x < sample.width(); ++x) {
            const QColor color = QColor::fromRgba(line[x]);
            if (color.alpha() < 32)
                continue;

            const double saturationWeight = 0.35 + color.hsvSaturationF();
            const double lightnessWeight = 0.35 + (1.0 - std::abs(color.lightnessF() - 0.55));
            const double weight = saturationWeight * lightnessWeight * color.alphaF();
            red += color.redF() * weight;
            green += color.greenF() * weight;
            blue += color.blueF() * weight;
            weightSum += weight;
        }
    }

    if (weightSum <= 0.0)
        return {};

    QColor result;
    result.setRgbF(
        qBound(0.0, red / weightSum, 1.0),
        qBound(0.0, green / weightSum, 1.0),
        qBound(0.0, blue / weightSum, 1.0));

    if (result.lightnessF() < 0.28)
        result = result.lighter(145);
    if (result.hsvSaturationF() < 0.16)
        result.setHsvF(0.62, 0.28, qMax(0.55, result.valueF()));

    return result;
}
