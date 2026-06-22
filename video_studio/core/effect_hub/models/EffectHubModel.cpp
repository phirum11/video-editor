#include "EffectHubModel.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

EffectHubModel::EffectHubModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Try absolute path first since the scraper downloads there
    QString targetPath = "C:/we_hunting/video_studio/assets/effect";
    QDir dir(targetPath);
    if (!dir.exists()) {
        // Fallback to relative to app dir
        targetPath = QCoreApplication::applicationDirPath() + "/assets/effect";
    }
    
    scanDirectory(targetPath);
}

void EffectHubModel::scanDirectory(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Effect directory does not exist:" << path;
        return;
    }

    dir.setNameFilters(QStringList() << "*.gif" << "*.webp" << "*.png" << "*.jpg");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        EffectItem item;
        item.title = fileInfo.completeBaseName();
        item.fileUrl = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
        item.category = "All"; // All effects are categorized as "All" for now
        m_allEffects.append(item);
    }
    
    // Load default
    loadEffects("All");
}

void EffectHubModel::search(const QString& query)
{
    m_searchQuery = query;
    loadEffects(m_currentCategory);
}

void EffectHubModel::loadEffects(const QString& category)
{
    beginResetModel();
    m_effects.clear();
    
    m_currentCategory = category;
    
    // First, filter by category
    QList<EffectItem> categoryEffects;
    if (category == "All") {
        categoryEffects = m_allEffects;
    } else {
        for (const auto& item : m_allEffects) {
            if (item.category == category) {
                categoryEffects.append(item);
            }
        }
        if (categoryEffects.isEmpty()) {
            categoryEffects = m_allEffects; // Fallback
        }
    }
    
    // Then, filter by search query
    if (m_searchQuery.isEmpty()) {
        m_effects = categoryEffects;
    } else {
        for (const auto& item : categoryEffects) {
            if (item.title.contains(m_searchQuery, Qt::CaseInsensitive)) {
                m_effects.append(item);
            }
        }
    }
    
    endResetModel();
}

int EffectHubModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_effects.count();
}

QVariant EffectHubModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_effects.count())
        return QVariant();

    const EffectItem &item = m_effects[index.row()];
    
    switch (role) {
    case TitleRole:
        return item.title;
    case FileUrlRole:
        return item.fileUrl;
    case CategoryRole:
        return item.category;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> EffectHubModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[FileUrlRole] = "fileURL";
    roles[CategoryRole] = "category";
    return roles;
}
