#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QUrl>

#include "runtime/library/LocalLibraryRepository.h"

class LocalLibraryTrackModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        TrackIdRole = Qt::UserRole + 1,
        SourceIdRole,
        TitleRole,
        ArtistRole,
        AlbumRole,
        FilePathRole,
        FileNameRole,
        UrlRole,
        AvailabilityRole,
    };
    Q_ENUM(Role)

    explicit LocalLibraryTrackModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setRecords(QList<LocalLibraryTrackRecord> records);
    [[nodiscard]] QVariantList urls() const;
    [[nodiscard]] QVariantList urlsStartingAt(int row) const;

private:
    QList<LocalLibraryTrackRecord> m_records;
};
