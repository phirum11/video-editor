#include "MediaPoolController.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QDesktopServices>
#include <memory>
#include "core/media/models/MediaItem.h"

MediaPoolController::MediaPoolController(QObject *parent)
    : QObject(parent), m_mediaModel(new MediaListModel(this))
{
}

QAbstractListModel* MediaPoolController::mediaModel() const
{
    return m_mediaModel;
}

void MediaPoolController::importMediaFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        nullptr,
        tr("Import Media"),
        QString(),
        tr("All Supported Files (*.mp4 *.mov *.avi *.mkv *.webm *.wmv *.flv *.ts *.m4v *.3gp "
           "*.mp3 *.wav *.aac *.flac *.ogg *.wma *.m4a *.opus *.aiff *.alac "
           "*.jpg *.jpeg *.png *.bmp *.gif *.tiff *.tif *.webp *.svg "
           "*.srt);;"
           "Video Files (*.mp4 *.mov *.avi *.mkv *.webm *.wmv *.flv *.ts *.m4v *.3gp);;"
           "Audio Files (*.mp3 *.wav *.aac *.flac *.ogg *.wma *.m4a *.opus *.aiff *.alac);;"
           "Image Files (*.jpg *.jpeg *.png *.bmp *.gif *.tiff *.tif *.webp *.svg);;"
           "Subtitle Files (*.srt);;"
           "All Files (*.*)")
    );

    for (const QString& file : files) {
        addMediaPath(file);
    }
}

void MediaPoolController::importMediaPath(const QString& filePath)
{
    addMediaPath(filePath);
}

void MediaPoolController::importMediaFileUrl(const QUrl& fileUrl)
{
    if (fileUrl.isLocalFile()) {
        addMediaPath(fileUrl.toLocalFile());
        return;
    }

    addMediaPath(fileUrl.toString());
}

bool MediaPoolController::removeMediaAt(int row)
{
    const QModelIndex mediaIndex = m_mediaModel->index(row, 0);
    const QString filePath = m_mediaModel->data(mediaIndex, MediaListModel::FilePathRole).toString();
    const bool removed = m_mediaModel->removeMediaItem(row);
    if (removed) {
        emit mediaRemoved(filePath);
    }
    return removed;
}

void MediaPoolController::openFileLocation(int row)
{
    const QModelIndex mediaIndex = m_mediaModel->index(row, 0);
    const QString filePath = m_mediaModel->data(mediaIndex, MediaListModel::FilePathRole).toString();
    if (!filePath.isEmpty()) {
        #ifdef Q_OS_WIN
        const QString explorer = "explorer.exe";
        QStringList args;
        args << "/select," << QDir::toNativeSeparators(filePath);
        QProcess::startDetached(explorer, args);
        #else
        // Fallback for other platforms (just opens the folder)
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).path()));
        #endif
    }
}

bool MediaPoolController::renameMediaAt(int row, const QString& newName)
{
    return m_mediaModel->renameMediaItem(row, newName);
}

bool MediaPoolController::addMediaPath(const QString& filePath)
{
    const QString trimmedPath = filePath.trimmed();
    if (trimmedPath.isEmpty() || !QFileInfo::exists(trimmedPath)) {
        qDebug() << "Skipped missing media file:" << filePath;
        return false;
    }

    try {
        auto item = std::make_shared<MediaItem>(trimmedPath);
        m_mediaModel->addMediaItem(item);
        qDebug() << "Imported media:" << item->fileName();
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to import media:" << trimmedPath << "Error:" << e.what();
        return false;
    }
}
