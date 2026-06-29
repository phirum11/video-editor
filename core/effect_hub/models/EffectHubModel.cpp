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
        
        QString titleLower = item.title.toLower();
        if (titleLower.contains("12 grapes") || titleLower.contains("4 shots") || titleLower.contains("5-pic squad") || titleLower.contains("money rush") || titleLower.contains("popular") || titleLower.contains("wow") || titleLower.contains("yeah") || titleLower.contains("insta") || titleLower.contains("ig") || titleLower.contains("trending") || titleLower.contains("viral") || titleLower.contains("warning") || titleLower.contains("what") || titleLower.contains("oh")) {
            item.category = "Trending";
        } else if (titleLower.contains("glitter") || titleLower.contains("bling") || titleLower.contains("star") || titleLower.contains("spark") || titleLower.contains("shiny") || titleLower.contains("glow") || titleLower.contains("neon") || titleLower.contains("crystal") || titleLower.contains("gem") || titleLower.contains("laser") || titleLower.contains("prism") || titleLower.contains("lightray") || titleLower.contains("sheen") || titleLower.contains("golden") || titleLower.contains("radiant") || titleLower.contains("flare")) {
            item.category = "Bling";
        } else if (titleLower.contains("party") || titleLower.contains("confetti") || titleLower.contains("balloon") || titleLower.contains("disco") || titleLower.contains("firework") || titleLower.contains("edm") || titleLower.contains("rhythm") || titleLower.contains("music") || titleLower.contains("boom") || titleLower.contains("beat") || titleLower.contains("love") || titleLower.contains("xmas") || titleLower.contains("christmas") || titleLower.contains("heart") || titleLower.contains("gift") || titleLower.contains("poker")) {
            item.category = "Party";
        } else if (titleLower.contains("retro") || titleLower.contains("vintage") || titleLower.contains("old") || titleLower.contains("bw") || titleLower.contains("b&w") || titleLower.contains("archive") || titleLower.contains("tape") || titleLower.contains("crt") || titleLower.contains("betamax") || titleLower.contains("vhs") || titleLower.contains("noise") || titleLower.contains("film") || titleLower.contains("cinema") || titleLower.contains("monochrome") || titleLower.contains("antique") || titleLower.contains("scanner") || titleLower.contains("screen") || titleLower.contains("grain") || titleLower.contains("corrupt") || titleLower.contains("glitch") || titleLower.contains("damage")) {
            item.category = "Retro";
        } else if (titleLower.contains("comic") || titleLower.contains("anime") || titleLower.contains("manga") || titleLower.contains("cartoon") || titleLower.contains("doodle") || titleLower.contains("graffiti") || titleLower.contains("halftone") || titleLower.contains("sketch") || titleLower.contains("newspaper") || titleLower.contains("superhero") || titleLower.contains("art") || titleLower.contains("poster") || titleLower.contains("draw") || titleLower.contains("clapping") || titleLower.contains("scare")) {
            item.category = "Comic";
        } else if (titleLower.contains("split") || titleLower.contains("slice") || titleLower.contains("grid") || titleLower.contains("matrix") || titleLower.contains("tile") || titleLower.contains("four") || titleLower.contains("nine") || titleLower.contains("panel") || titleLower.contains("array") || titleLower.contains("stack") || titleLower.contains("section") || titleLower.contains("multi") || titleLower.contains("carousel") || titleLower.contains("cube") || titleLower.contains("box") || titleLower.contains("window")) {
            item.category = "Split";
        } else if (titleLower.contains("nature") || titleLower.contains("earth") || titleLower.contains("snow") || titleLower.contains("rain") || titleLower.contains("water") || titleLower.contains("cloud") || titleLower.contains("beach") || titleLower.contains("flower") || titleLower.contains("rose") || titleLower.contains("maple") || titleLower.contains("ocean") || titleLower.contains("lava") || titleLower.contains("flame") || titleLower.contains("fire") || titleLower.contains("storm") || titleLower.contains("wind") || titleLower.contains("galaxy") || titleLower.contains("nebula") || titleLower.contains("solar") || titleLower.contains("heat") || titleLower.contains("thermal") || titleLower.contains("piranha") || titleLower.contains("smoke") || titleLower.contains("beast") || titleLower.contains("shark")) {
            item.category = "Nature";
        } else if (titleLower.contains("transition") || titleLower.contains("wipe") || titleLower.contains("swipe") || titleLower.contains("open") || titleLower.contains("close") || titleLower.contains("merge") || titleLower.contains("fall") || titleLower.contains("flip") || titleLower.contains("swap") || titleLower.contains("fold") || titleLower.contains("turn") || titleLower.contains("teleport") || titleLower.contains("scroll") || titleLower.contains("slide") || titleLower.contains("-in") || titleLower.contains("-out") || titleLower.contains("passing") || titleLower.contains("pasting") || titleLower.contains("cut") || titleLower.contains("bounce") || titleLower.contains("shift") || titleLower.contains("move") || titleLower.contains("subway")) {
            item.category = "Transitions";
        } else {
            item.category = "Basic";
        }
        
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
