#pragma once

#include <QColor>
#include <QImage>
#include <QMediaMetaData>
#include <QString>
#include <QUrl>

#include <initializer_list>

struct LocalTrackIdentity
{
    QString title;
    QString artist;
    QString album;
    int trackNumber = 0;
    QUrl artworkSource;
    QColor identityColor;
    bool hasEmbeddedArtwork = false;
    bool metadataAvailable = false;
    QString provenance;

    [[nodiscard]] bool operator==(const LocalTrackIdentity &other) const = default;
};

class LocalTrackIdentityResolver final
{
public:
    [[nodiscard]] static LocalTrackIdentity fallbackFor(const QUrl &source);
    [[nodiscard]] static LocalTrackIdentity resolve(
        const QUrl &source,
        const QMediaMetaData &metaData,
        const QString &artworkCacheRoot = {});

private:
    [[nodiscard]] static QString normalizedText(const QString &value);
    [[nodiscard]] static QString firstMetadataText(
        const QMediaMetaData &metaData,
        std::initializer_list<QMediaMetaData::Key> keys);
    [[nodiscard]] static QImage embeddedArtwork(const QMediaMetaData &metaData);
    [[nodiscard]] static QUrl cacheArtwork(
        const QUrl &source,
        const QImage &image,
        const QString &artworkCacheRoot);
    [[nodiscard]] static QColor representativeColor(const QImage &image);
};
