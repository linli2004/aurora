#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QUrl>

#include "runtime/memory/MomentRepository.h"

class MomentService final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasMoment READ hasMoment NOTIFY momentsChanged)
    Q_PROPERTY(int momentCount READ momentCount NOTIFY momentsChanged)
    Q_PROPERTY(QString latestMomentId READ latestMomentId NOTIFY momentsChanged)
    Q_PROPERTY(QString latestTrackId READ latestTrackId NOTIFY momentsChanged)
    Q_PROPERTY(QString latestTitle READ latestTitle NOTIFY momentsChanged)
    Q_PROPERTY(QString latestArtist READ latestArtist NOTIFY momentsChanged)
    Q_PROPERTY(QString latestAlbum READ latestAlbum NOTIFY momentsChanged)
    Q_PROPERTY(QUrl latestArtworkSource READ latestArtworkSource NOTIFY momentsChanged)
    Q_PROPERTY(QColor latestIdentityColor READ latestIdentityColor NOTIFY momentsChanged)
    Q_PROPERTY(QString latestPeriodLabel READ latestPeriodLabel NOTIFY momentsChanged)
    Q_PROPERTY(QString latestConfirmedMeaning READ latestConfirmedMeaning NOTIFY momentsChanged)
    Q_PROPERTY(bool latestAvailable READ latestAvailable NOTIFY momentsChanged)
    Q_PROPERTY(QUrl latestUrl READ latestUrl NOTIFY momentsChanged)
    Q_PROPERTY(QString lastStatus READ lastStatus NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit MomentService(QObject *parent = nullptr);

    [[nodiscard]] bool hasMoment() const;
    [[nodiscard]] int momentCount() const;
    [[nodiscard]] QString latestMomentId() const;
    [[nodiscard]] QString latestTrackId() const;
    [[nodiscard]] QString latestTitle() const;
    [[nodiscard]] QString latestArtist() const;
    [[nodiscard]] QString latestAlbum() const;
    [[nodiscard]] QUrl latestArtworkSource() const;
    [[nodiscard]] QColor latestIdentityColor() const;
    [[nodiscard]] QString latestPeriodLabel() const;
    [[nodiscard]] QString latestConfirmedMeaning() const;
    [[nodiscard]] bool latestAvailable() const;
    [[nodiscard]] QUrl latestUrl() const;
    [[nodiscard]] QString lastStatus() const;
    [[nodiscard]] QString errorString() const;

    Q_INVOKABLE bool keepCurrentMoment(
        const QString &trackId,
        const QString &sourceId,
        const QString &filePath,
        const QString &title,
        const QString &artist,
        const QString &album,
        const QUrl &artworkSource,
        const QColor &identityColor,
        const QString &confirmedMeaning = {});
    Q_INVOKABLE void refresh();

signals:
    void momentsChanged();
    void statusChanged();
    void errorChanged();

private:
    [[nodiscard]] QString periodLabel(const QDateTime &localTime) const;
    void setLastStatus(const QString &status);
    void setErrorString(const QString &error);

    QString m_databasePath;
    MomentRepository m_repository;
    std::optional<MomentRecord> m_latestMoment;
    int m_momentCount = 0;
    QString m_resolvedPlayablePath;
    QString m_lastStatus;
    QString m_errorString;
};
