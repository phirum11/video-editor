#include "core/media/models/MediaListModel.h"

MediaListModel::MediaListModel(QObject* parent) : QAbstractListModel(parent) {}

void MediaListModel::addMediaItem(std::shared_ptr<MediaItem> item) {
    beginInsertRows(QModelIndex(), static_cast<int>(m_items.size()), static_cast<int>(m_items.size()));
    m_items.push_back(std::move(item));
    endInsertRows();
}

bool MediaListModel::removeMediaItem(int row) {
    if (row < 0 || row >= static_cast<int>(m_items.size())) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    m_customNames.remove(row);
    // Shift remaining custom names down
    QHash<int, QString> newCustomNames;
    for (auto it = m_customNames.begin(); it != m_customNames.end(); ++it) {
        if (it.key() > row) {
            newCustomNames[it.key() - 1] = it.value();
        } else {
            newCustomNames[it.key()] = it.value();
        }
    }
    m_customNames = newCustomNames;
    endRemoveRows();
    return true;
}

int MediaListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_items.size());
}

bool MediaListModel::renameMediaItem(int row, const QString& newName) {
    if (row < 0 || row >= static_cast<int>(m_items.size())) {
        return false;
    }
    m_customNames[row] = newName;
    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, {NameRole});
    return true;
}

QVariant MediaListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_items.size()))
        return QVariant();

    const auto& item = m_items[index.row()];
    switch (role) {
        case NameRole: return m_customNames.contains(index.row()) ? m_customNames[index.row()] : item->fileName();
        case FilePathRole: return item->filePath();
        case DurationRole: return item->durationSeconds();
        case HasVideoRole: return item->hasVideo();
        case HasAudioRole: return item->hasAudio();
    }
    return QVariant();
}

QHash<int, QByteArray> MediaListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[FilePathRole] = "filePath";
    roles[DurationRole] = "duration";
    roles[HasVideoRole] = "hasVideo";
    roles[HasAudioRole] = "hasAudio";
    return roles;
}
