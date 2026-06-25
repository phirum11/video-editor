#ifndef MEDIAPOOLCONTROLLER_H
#define MEDIAPOOLCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>
#include <QSortFilterProxyModel>
#include "core/media/models/MediaListModel.h"

class MediaPoolController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAbstractItemModel* mediaModel READ mediaModel NOTIFY mediaModelChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)

public:
    explicit MediaPoolController(QObject *parent = nullptr);

    QAbstractItemModel* mediaModel() const;
    QString searchQuery() const;
    void setSearchQuery(const QString& query);

    Q_INVOKABLE void importMediaFiles();
    Q_INVOKABLE void importMediaPath(const QString& filePath);
    Q_INVOKABLE void importMediaFileUrl(const QUrl& fileUrl);
    Q_INVOKABLE bool removeMediaAt(int row);
    Q_INVOKABLE QVariantMap getMediaAt(int row);
    Q_INVOKABLE void openFileLocation(int row);
    Q_INVOKABLE bool renameMediaAt(int row, const QString& newName);

signals:
    void mediaModelChanged();
    void mediaRemoved(const QString& filePath);
    void searchQueryChanged();

private:
    bool addMediaPath(const QString& filePath);

    MediaListModel* m_mediaModel;
    QSortFilterProxyModel* m_proxyModel;
    QString m_searchQuery;
};

#endif // MEDIAPOOLCONTROLLER_H
