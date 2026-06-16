#ifndef TIMELINECOMMANDS_H
#define TIMELINECOMMANDS_H

#include <QUndoCommand>
#include <QVector>
#include "core/timeline/models/TimelineClipModel.h"

// AddClipCommand
class AddClipCommand : public QUndoCommand {
public:
    AddClipCommand(TimelineClipModel* model, const TimelineClip& clip, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    TimelineClipModel* m_model;
    TimelineClip m_clip;
    int m_row;
    bool m_firstTime;
};

// RemoveClipCommand
class RemoveClipCommand : public QUndoCommand {
public:
    enum Type { SingleRow, LinkedGroup, FilePath, All };
    RemoveClipCommand(TimelineClipModel* model, int row, Type type, const QString& param = QString(), QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    TimelineClipModel* m_model;
    int m_row;
    Type m_type;
    QString m_param;
    
    struct SavedClip {
        int originalRow;
        TimelineClip clip;
    };
    QVector<SavedClip> m_savedClips;
    bool m_firstTime;
};

// MoveClipCommand
class MoveClipCommand : public QUndoCommand {
public:
    MoveClipCommand(TimelineClipModel* model, int row, double newStart, int newTrack, bool linked, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    TimelineClipModel* m_model;
    int m_row;
    double m_newStart;
    int m_newTrack;
    bool m_linked;
    
    struct SavedState {
        int row;
        TimelineClip clip;
    };
    QVector<SavedState> m_savedStates;
    bool m_firstTime;
};

// TrimClipCommand
class TrimClipCommand : public QUndoCommand {
public:
    TrimClipCommand(TimelineClipModel* model, int row, double newStart, double newDuration, double newInPoint, bool linked, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    TimelineClipModel* m_model;
    int m_row;
    double m_newStart;
    double m_newDuration;
    double m_newInPoint;
    bool m_linked;
    
    struct SavedState {
        int row;
        TimelineClip clip;
    };
    QVector<SavedState> m_savedStates;
    bool m_firstTime;
};

// SplitClipCommand
class SplitClipCommand : public QUndoCommand {
public:
    SplitClipCommand(TimelineClipModel* model, int row, double splitSeconds, const QString& rightLinkGroupId = QString(), QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    TimelineClipModel* m_model;
    int m_row;
    double m_splitSeconds;
    QString m_rightLinkGroupId;
    
    TimelineClip m_originalClip;
    int m_newRightRow;
    bool m_firstTime;
};

#endif // TIMELINECOMMANDS_H
