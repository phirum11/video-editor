#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include "ui/timeline_view/controllers/TimelineController.h"
#include "ui/media_pool/MediaPoolController.h"
#include "core/subtitle/SubtitleController.h"

class ProjectManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ProjectManager(QObject* parent = nullptr);

    Q_INVOKABLE bool saveProject(const QString& filePath, 
                                 MediaPoolController* mediaPool, 
                                 TimelineController* timeline, 
                                 SubtitleController* subtitles);

    Q_INVOKABLE bool loadProject(const QString& filePath, 
                                 MediaPoolController* mediaPool, 
                                 TimelineController* timeline, 
                                 SubtitleController* subtitles);

private:
    QString normalizePath(const QString& path) const;
};
