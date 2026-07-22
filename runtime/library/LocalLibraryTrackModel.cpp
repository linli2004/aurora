#include "runtime/library/LocalLibraryTrackModel.h"

#include <QFileInfo>

#include <utility>

LocalLibraryTrackModel::LocalLibraryTrackModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LocalLibraryTrackModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

QVariant LocalLibraryTrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return {};

    const LocalLibraryTrackRecord &record = m_records.at(index.row());
    switch (role) {
    case TrackIdRole:
        return record.trackId;
    case SourceIdRole:
        return record.sourceId;
    case TitleRole:
        return record.title;
    case ArtistRole:
        return record.artist;
    case AlbumRole:
        return record.album;
    case FilePathRole:
        return record.filePath;
    case FileNameRole:
        return QFileInfo(record.filePath).fileName();
    case UrlRole:
        return QUrl::fromLocalFile(record.filePath);
    case AvailabilityRole:
        return record.availability;
    default:
        return {};
    }
}

QHash<int, QByteArray> LocalLibraryTrackModel::roleNames() const
{
    return {
        {TrackIdRole, "trackId"},
        {SourceIdRole, "sourceId"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {AlbumRole, "album"},
        {FilePathRole, "filePath"},
        {FileNameRole, "fileName"},
        {UrlRole, "url"},
        {AvailabilityRole, "availability"},
    };
}

void LocalLibraryTrackModel::setRecords(QList<LocalLibraryTrackRecord> records)
{
    beginResetModel();
    m_records = std::move(records);
    endResetModel();
}

QVariantList LocalLibraryTrackModel::urls() const
{
    QVariantList result;
    result.reserve(m_records.size());
    for (const LocalLibraryTrackRecord &record : m_records)
        result.append(QUrl::fromLocalFile(record.filePath));
    return result;
}

QVariantList LocalLibraryTrackModel::urlsStartingAt(int row) const
{
    if (m_records.isEmpty())
        return {};

    const int start = row >= 0 && row < m_records.size() ? row : 0;
    QVariantList result;
    result.reserve(m_records.size());

    for (int index = start; index < m_records.size(); ++index)
        result.append(QUrl::fromLocalFile(m_records.at(index).filePath));
    for (int index = 0; index < start; ++index)
        result.append(QUrl::fromLocalFile(m_records.at(index).filePath));

    return result;
}
