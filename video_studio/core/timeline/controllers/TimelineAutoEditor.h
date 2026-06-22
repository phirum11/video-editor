#ifndef TIMELINEAUTOEDITOR_H
#define TIMELINEAUTOEDITOR_H

class TimelineClipModel;
class QUndoStack;

class TimelineAutoEditor
{
public:
    static void syncVisualsToAudio(TimelineClipModel* model, QUndoStack* undoStack);
};

#endif // TIMELINEAUTOEDITOR_H
