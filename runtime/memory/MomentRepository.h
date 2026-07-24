#pragma once

#include <QDateTime>
#include <QSqlDatabase>
#include <QString>

#include <optional>

struct MomentRecord
{
    QString momentId;
    QString trackId;
    QString sourceId;
    QString sourcePath;
    QString title;
    QString artist;
    QString album;
    QString artworkUrl;
    QString identityColor;
    QString periodLabel;
    QString confirmedMeaning;
    QDateTime createdAt;
};

class MomentRepository final
{
public:
    explicit MomentRepository(QString connectionName = {});
    ~MomentRepository();

    [[nodiscard]] bool open(const QString &databasePath);
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] int count() const;
    [[nodiscard]] std::optional<MomentRecord> latestMoment() const;
    [[nodiscard]] QString resolvedPlayablePath(const MomentRecord &moment) const;

    bool initializeSchema();
    bool keepMoment(const MomentRecord &moment);

private:
    [[nodiscard]] bool execSchema(const QString &sql);
    [[nodiscard]] QString provenanceJson() const;

    QString m_connectionName;
    QSqlDatabase m_database;
    QString m_lastError;
};
