#pragma once

#include <QObject>
#include <QString>
#include "../models/EditorImageModel.h"

class XcfProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit XcfProjectManager(QObject *parent = nullptr);
    ~XcfProjectManager() override;

    Q_INVOKABLE bool loadProject(const QString& filePath, EditorImageModel* imageModel);
    Q_INVOKABLE bool saveProject(const QString& filePath, EditorImageModel* imageModel);

signals:
    void projectLoaded(bool success);
    void projectSaved(bool success);
};
