#ifndef CLIPDRAGCONTROLLER_H
#define CLIPDRAGCONTROLLER_H

#include "TimelineController.h"

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QList>

class ClipDragController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ClipDragController(QObject *parent = nullptr);

    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)
    bool isDragging() const { return m_isDragging; }

    Q_INVOKABLE void beginDrag(TimelineController* timeline);
    Q_INVOKABLE void updateDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack);
    Q_INVOKABLE void endDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack, bool linked);

signals:
    void isDraggingChanged();

private:
    struct DragState {
        int row;
        double originalStartSeconds;
        int originalTrackIndex;
    };
    QList<DragState> m_dragStates;
    bool m_isDragging = false;
};

#endif // CLIPDRAGCONTROLLER_H
