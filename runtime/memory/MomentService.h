#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>

#include "runtime/memory/MomentRepository.h"

class MomentService final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasMoment READ hasMoment NOTIFY momentsChanged)
    Q_PROPERTY(int momentCount READ momentCount NOTIFY momentsChanged)
    Q_PROPERTY(QVariantList momentItems READ momentItems NOTIFY momentsChanged)

    Q_PROPERTY(QString latestMomentId READ latestMomentId NOTIFY momentsChanged)
    Q_PROPERTY(QString latestTrackId READ latestTrackId NOTIFY momentsChanged)
    Q_PROPERTY(QString latestHeading READ latestHeading NOTIFY momentsChanged)
    Q_PROPERTY(QString latestTitle READ latestTitle NOTIFY momentsChanged)
    Q_PROPERTY(QString latestArtist READ latestArtist NOTIFY momentsChanged)
    Q_PROPERTY(QString latestAlbum READ latestAlbum NOTIFY momentsChanged)
    Q_PROPERTY(QUrl latestArtworkSource READ latestArtworkSource NOTIFY momentsChanged)
    Q_PROPERTY(QColor latestIdentityColor READ latestIdentityColor NOTIFY momentsChanged)
    Q_PROPERTY(QString latestPeriodLabel READ latestPeriodLabel NOTIFY momentsChanged)
    Q_PROPERTY(QString latestConfirmedMeaning READ latestConfirmedMeaning NOTIFY momentsChanged)
    Q_PROPERTY(bool latestAvailable READ latestAvailable NOTIFY momentsChanged)
    Q_PROPERTY(QUrl latestUrl READ latestUrl NOTIFY momentsChanged)

    Q_PROPERTY(QString selectedMomentId READ selectedMomentId NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedTrackId READ selectedTrackId NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedHeading READ selectedHeading NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedTitle READ selectedTitle NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedArtist READ selectedArtist NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedAlbum READ selectedAlbum NOTIFY selectionChanged)
    Q_PROPERTY(QUrl selectedArtworkSource READ selectedArtworkSource NOTIFY selectionChanged)
    Q_PROPERTY(QColor selectedIdentityColor READ selectedIdentityColor NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedPeriodLabel READ selectedPeriodLabel NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedConfirmedMeaning READ selectedConfirmedMeaning NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedCreatedLabel READ selectedCreatedLabel NOTIFY selectionChanged)
    Q_PROPERTY(bool selectedAvailable READ selectedAvailable NOTIFY selectionChanged)
    Q_PROPERTY(QUrl selectedUrl READ selectedUrl NOTIFY selectionChanged)

    Q_PROPERTY(QString lastStatus READ lastStatus NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit MomentService(QObject *parent = nullptr);

    [[nodiscard]] bool hasMoment() const;
    [[nodiscard]] int momentCount() const;
    [[nodiscard]] QVariantList momentItems() const;

    [[nodiscard]] QString latestMomentId() const;
    [[nodiscard]] QString latestTrackId() const;
    [[nodiscard]] QString latestHeading() const;
    [[nodiscard]] QString latestTitle() const;
    [[nodiscard]] QString latestArtist() const;
    [[nodiscard]] QString latestAlbum() const;
    [[nodiscard]] QUrl latestArtworkSource() const;
    [[nodiscard]] QColor latestIdentityColor() const;
    [[nodiscard]] QString latestPeriodLabel() const;
    [[nodiscard]] QString latestConfirmedMeaning() const;
    [[nodiscard]] bool latestAvailable() const;
    [[nodiscard]] QUrl latestUrl() const;

    [[nodiscard]] QString selectedMomentId() const;
    [[nodiscard]] QString selectedTrackId() const;
    [[nodiscard]] QString selectedHeading() const;
    [[nodiscard]] QString selectedTitle() const;
    [[nodiscard]] QString selectedArtist() const;
    [[nodiscard]] QString selectedAlbum() const;
    [[nodiscard]] QUrl selectedArtworkSource() const;
    [[nodiscard]] QColor selectedIdentityColor() const;
    [[nodiscard]] QString selectedPeriodLabel() const;
    [[nodiscard]] QString selectedConfirmedMeaning() const;
    [[nodiscard]] QString selectedCreatedLabel() const;
    [[nodiscard]] bool selectedAvailable() const;
    [[nodiscard]] QUrl selectedUrl() const;

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
    Q_INVOKABLE bool selectMoment(const QString &momentId);
    Q_INVOKABLE bool updateConfirmedMeaning(
        const QString &momentId,
        const QString &confirmedMeaning);
    Q_INVOKABLE void refresh();

signals:
    void momentsChanged();
    void selectionChanged();
    void statusChanged();
    void errorChanged();

private:
    [[nodiscard]] QString periodLabel(const QDateTime &localTime) const;
    [[nodiscard]] QString headingForPeriod(const QString &period) const;
    [[nodiscard]] QVariantMap momentItem(
        const MomentRecord &moment,
        const QString &resolvedPlayablePath) const;
    void setSelectedMoment(
        const std::optional<MomentRecord> &moment,
        const QString &resolvedPlayablePath);
    void setLastStatus(const QString &status);
    void setErrorString(const QString &error);

    QString m_databasePath;
    MomentRepository m_repository;
    QList<MomentRecord> m_moments;
    QVariantList m_momentItems;
    std::optional<MomentRecord> m_latestMoment;
    std::optional<MomentRecord> m_selectedMoment;
    int m_momentCount = 0;
    QString m_latestPlayablePath;
    QString m_selectedPlayablePath;
    QString m_lastStatus;
    QString m_errorString;
};
