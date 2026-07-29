#pragma once

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantList>

class QImage;
class QNetworkAccessManager;
class QNetworkReply;

class ArtworkIllustrationService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl illustrationSource READ illustrationSource NOTIFY stateChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY stateChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY stateChanged)
    Q_PROPERTY(bool fallbackRequired READ fallbackRequired NOTIFY stateChanged)
    Q_PROPERTY(qreal focalX READ focalX NOTIFY stateChanged)
    Q_PROPERTY(qreal focalY READ focalY NOTIFY stateChanged)
    Q_PROPERTY(qreal edgeDensity READ edgeDensity NOTIFY stateChanged)
    Q_PROPERTY(qreal contrast READ contrast NOTIFY stateChanged)
    Q_PROPERTY(QVariantList particleSamples READ particleSamples NOTIFY stateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY stateChanged)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)

public:
    explicit ArtworkIllustrationService(QObject *parent = nullptr);

    [[nodiscard]] QUrl illustrationSource() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] bool fallbackRequired() const;
    [[nodiscard]] qreal focalX() const;
    [[nodiscard]] qreal focalY() const;
    [[nodiscard]] qreal edgeDensity() const;
    [[nodiscard]] qreal contrast() const;
    [[nodiscard]] QVariantList particleSamples() const;
    [[nodiscard]] QString errorString() const;
    [[nodiscard]] QString cacheDirectory() const;

    Q_INVOKABLE void load(const QUrl &artworkSource, const QString &trackIdentity);
    Q_INVOKABLE void clear();

signals:
    void stateChanged();

private:
    void cancelActiveRequest();
    void setLoading(bool loading);
    void setErrorString(const QString &message);
    bool loadLocalOrResource(const QUrl &source, const QString &trackIdentity);
    void beginRemoteLoad(const QUrl &source, const QString &trackIdentity);
    bool processImage(
        const QImage &sourceImage,
        const QUrl &source,
        const QString &trackIdentity);
    [[nodiscard]] QString originalCachePath(const QUrl &source) const;
    [[nodiscard]] QString illustrationCachePath(
        const QUrl &source,
        const QString &trackIdentity) const;
    void pruneCacheDirectory(const QString &directoryPath, qint64 maximumBytes) const;

    QNetworkAccessManager *m_network = nullptr;
    QPointer<QNetworkReply> m_activeReply;
    QUrl m_illustrationSource;
    QUrl m_currentArtworkSource;
    QString m_currentTrackIdentity;
    QString m_errorString;
    qreal m_focalX = 0.5;
    qreal m_focalY = 0.5;
    qreal m_edgeDensity = 0.0;
    qreal m_contrast = 0.0;
    QVariantList m_particleSamples;
    int m_generation = 0;
    bool m_ready = false;
    bool m_loading = false;
};
