#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantList>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;

class LyricsService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentLine READ currentLine NOTIFY lyricsChanged)
    Q_PROPERTY(QString nextLine READ nextLine NOTIFY lyricsChanged)
    Q_PROPERTY(bool hasLyrics READ hasLyrics NOTIFY lyricsChanged)
    Q_PROPERTY(QString sourcePath READ sourcePath NOTIFY lyricsChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)

public:
    explicit LyricsService(QObject *parent = nullptr);

    [[nodiscard]] QString currentLine() const;
    [[nodiscard]] QString nextLine() const;
    [[nodiscard]] bool hasLyrics() const;
    [[nodiscard]] QString sourcePath() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString errorString() const;
    [[nodiscard]] QString cacheDirectory() const;

    Q_INVOKABLE void rememberTracks(const QVariantList &tracks);
    Q_INVOKABLE void warmForTracks(const QVariantList &tracks);
    Q_INVOKABLE bool loadForSource(const QUrl &source);
    Q_INVOKABLE bool loadFromText(const QString &text);
    Q_INVOKABLE void setPosition(qint64 positionMilliseconds);
    Q_INVOKABLE void clear();

signals:
    void lyricsChanged();
    void statusChanged();

private:
    struct Entry {
        qint64 timestamp = 0;
        QString text;
    };

    struct TrackHint {
        QString provider;
        QString songId;
    };

    static QVector<Entry> parseLrc(const QString &text);
    static QString normalizedSourceKey(const QUrl &source);
    void updateCurrentIndex(qint64 positionMilliseconds);
    void cancelRequest();
    void resetLyrics();
    bool loadLocalLyrics(const QUrl &source);
    bool loadCachedLyrics(const TrackHint &hint);
    void requestOnlineLyrics(
        const TrackHint &hint,
        int generation);
    void requestWarmOnlineLyrics(const TrackHint &hint);
    [[nodiscard]] QString cachedLyricsPath(const TrackHint &hint) const;
    void setLoading(bool loading);
    void setErrorString(const QString &message);

    QNetworkAccessManager *m_network = nullptr;
    QPointer<QNetworkReply> m_activeReply;
    QHash<QString, QPointer<QNetworkReply>> m_warmReplies;
    QHash<QString, TrackHint> m_trackHints;
    QVector<Entry> m_entries;
    QString m_sourcePath;
    QString m_errorString;
    int m_currentIndex = -1;
    int m_generation = 0;
    bool m_loading = false;
};
