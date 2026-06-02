#ifndef CLIPTRIMCONTROLLER_H
#define CLIPTRIMCONTROLLER_H

#include "TimelineController.h"

#include <QObject>
#include <QtQml/qqmlregistration.h>

class ClipTrimController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ClipTrimController(QObject *parent = nullptr);

    Q_INVOKABLE bool trimClipLeft(TimelineController* timeline, int row, double deltaSeconds);
    Q_INVOKABLE bool trimClipRight(TimelineController* timeline, int row, double deltaSeconds);

private:
    bool isValid(TimelineController* timeline, int row) const;
};

#endif // CLIPTRIMCONTROLLER_H
