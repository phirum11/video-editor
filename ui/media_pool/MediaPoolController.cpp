#include "MediaPoolController.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QDesktopServices>
#include <memory>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include "core/media/models/MediaItem.h"

MediaPoolController::MediaPoolController(QObject *parent)
    : QObject(parent), m_mediaModel(new MediaListModel(this)), m_proxyModel(new QSortFilterProxyModel(this))
{
    m_proxyModel->setSourceModel(m_mediaModel);
    m_proxyModel->setFilterRole(MediaListModel::NameRole);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

QAbstractItemModel* MediaPoolController::mediaModel() const
{
    return m_proxyModel;
}

QString MediaPoolController::searchQuery() const
{
    return m_searchQuery;
}

void MediaPoolController::setSearchQuery(const QString& query)
{
    if (m_searchQuery != query) {
        m_searchQuery = query;
        m_proxyModel->setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(query), QRegularExpression::CaseInsensitiveOption));
        emit searchQueryChanged();
    }
}

int MediaPoolController::mediaTypeFilter() const
{
    return m_mediaTypeFilter;
}

void MediaPoolController::setMediaTypeFilter(int filter)
{
    if (m_mediaTypeFilter != filter) {
        m_mediaTypeFilter = filter;
        emit mediaTypeFilterChanged();
    }
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
    const QModelIndex proxyIndex = m_proxyModel->index(row, 0);
    if (!proxyIndex.isValid()) return false;
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    const int sourceRow = sourceIndex.row();

    const QString filePath = m_mediaModel->data(sourceIndex, MediaListModel::FilePathRole).toString();
    const bool removed = m_mediaModel->removeMediaItem(sourceRow);
    if (removed) {
        emit mediaRemoved(filePath);
    }
    return removed;
}

QVariantMap MediaPoolController::getMediaAt(int row)
{
    QVariantMap result;
    const QModelIndex proxyIndex = m_proxyModel->index(row, 0);
    if (proxyIndex.isValid()) {
        const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        result["mediaName"] = m_mediaModel->data(sourceIndex, MediaListModel::NameRole).toString();
        result["mediaFilePath"] = m_mediaModel->data(sourceIndex, MediaListModel::FilePathRole).toString();
        result["mediaDuration"] = m_mediaModel->data(sourceIndex, MediaListModel::DurationRole).toReal();
        result["mediaHasVideo"] = m_mediaModel->data(sourceIndex, MediaListModel::HasVideoRole).toBool();
        result["mediaHasAudio"] = m_mediaModel->data(sourceIndex, MediaListModel::HasAudioRole).toBool();
    }
    return result;
}

void MediaPoolController::openFileLocation(int row)
{
    const QModelIndex proxyIndex = m_proxyModel->index(row, 0);
    if (!proxyIndex.isValid()) return;
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    const QString filePath = m_mediaModel->data(sourceIndex, MediaListModel::FilePathRole).toString();
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
    const QModelIndex proxyIndex = m_proxyModel->index(row, 0);
    if (!proxyIndex.isValid()) return false;
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    return m_mediaModel->renameMediaItem(sourceIndex.row(), newName);
}

bool MediaPoolController::addMediaPath(const QString& filePath)
{
    const QString trimmedPath = filePath.trimmed();
    if (trimmedPath.isEmpty() || !QFileInfo::exists(trimmedPath)) {
        qDebug() << "Skipped missing media file:" << filePath;
        return false;
    }

    try {
        qDebug() << "MediaPoolController::addMediaPath - Creating MediaItem for:" << trimmedPath;
        auto item = std::make_shared<MediaItem>(trimmedPath);
        qDebug() << "MediaPoolController::addMediaPath - MediaItem created. hasVideo=" << item->hasVideo() << "hasAudio=" << item->hasAudio();
        m_mediaModel->addMediaItem(item);
        qDebug() << "MediaPoolController::addMediaPath - Added to MediaListModel.";
        qDebug() << "Imported media:" << item->fileName();
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to import media:" << trimmedPath << "Error:" << e.what();
        return false;
    }
}
