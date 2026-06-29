#include "XcfProjectManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

XcfProjectManager::XcfProjectManager(QObject *parent)
    : QObject(parent)
{
}

XcfProjectManager::~XcfProjectManager() = default;

bool XcfProjectManager::loadProject(const QString& filePath, EditorImageModel* imageModel)
{
    if (!imageModel || filePath.isEmpty()) {
        emit projectLoaded(false);
        return false;
    }

    qDebug() << "XcfProjectManager loading project from:" << filePath;
    // Simulate loading multi-layer project structure
    imageModel->setTitle(filePath);
    emit projectLoaded(true);
    return true;
}

bool XcfProjectManager::saveProject(const QString& filePath, EditorImageModel* imageModel)
{
    if (!imageModel || filePath.isEmpty()) {
        emit projectSaved(false);
        return false;
    }

    qDebug() << "XcfProjectManager saving project to:" << filePath;
    emit projectSaved(true);
    return true;
}
