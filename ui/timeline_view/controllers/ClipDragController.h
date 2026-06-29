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

    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)
    Q_PROPERTY(double ghostX READ ghostX NOTIFY ghostXChanged)
    Q_PROPERTY(double ghostY READ ghostY NOTIFY ghostYChanged)
    Q_PROPERTY(double dragDeltaSeconds READ dragDeltaSeconds NOTIFY dragDeltaSecondsChanged)
    Q_PROPERTY(int dragDeltaTrack READ dragDeltaTrack NOTIFY dragDeltaTrackChanged)

public:
    explicit ClipDragController(QObject *parent = nullptr);

    bool isDragging() const { return m_isDragging; }
    double ghostX() const { return m_ghostX; }
    double ghostY() const { return m_ghostY; }
    double dragDeltaSeconds() const { return m_dragDeltaSeconds; }
    int dragDeltaTrack() const { return m_dragDeltaTrack; }

    Q_INVOKABLE void beginDrag(TimelineController* timeline, double startX = 0.0, double startY = 0.0);
    Q_INVOKABLE void updateDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack, double curX = 0.0, double curY = 0.0);
    Q_INVOKABLE void endDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack, bool linked);

signals:
    void isDraggingChanged();
    void ghostXChanged();
    void ghostYChanged();
    void dragDeltaSecondsChanged();
    void dragDeltaTrackChanged();

private:
    struct DragState {
        int row;
        double originalStartSeconds;
        int originalTrackIndex;
    };
    QList<DragState> m_dragStates;
    bool m_isDragging = false;
    double m_ghostX = 0.0;
    double m_ghostY = 0.0;
    double m_dragDeltaSeconds = 0.0;
    int m_dragDeltaTrack = 0;
};

#endif // CLIPDRAGCONTROLLER_H
