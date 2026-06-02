#pragma once

#include <QAbstractListModel>
#include <memory>
#include <vector>
#include "MediaItem.h"

class MediaListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum MediaRoles {
        NameRole = Qt::UserRole + 1,
        FilePathRole,
        DurationRole,
        HasVideoRole,
        HasAudioRole
    };

    explicit MediaListModel(QObject* parent = nullptr);

    void addMediaItem(std::shared_ptr<MediaItem> item);
    bool removeMediaItem(int row);
    bool renameMediaItem(int row, const QString& newName);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<std::shared_ptr<MediaItem>> m_items;
    QHash<int, QString> m_customNames;
};
