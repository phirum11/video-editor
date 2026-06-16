#ifndef MEDIAPOOLCONTROLLER_H
#define MEDIAPOOLCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>
#include "core/media/models/MediaListModel.h"

class MediaPoolController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAbstractListModel* mediaModel READ mediaModel NOTIFY mediaModelChanged)

public:
    explicit MediaPoolController(QObject *parent = nullptr);

    QAbstractListModel* mediaModel() const;

    Q_INVOKABLE void importMediaFiles();
    Q_INVOKABLE void importMediaPath(const QString& filePath);
    Q_INVOKABLE void importMediaFileUrl(const QUrl& fileUrl);
    Q_INVOKABLE bool removeMediaAt(int row);
    Q_INVOKABLE void openFileLocation(int row);
    Q_INVOKABLE bool renameMediaAt(int row, const QString& newName);

signals:
    void mediaModelChanged();
    void mediaRemoved(const QString& filePath);

private:
    bool addMediaPath(const QString& filePath);

    MediaListModel* m_mediaModel;
};

#endif // MEDIAPOOLCONTROLLER_H
