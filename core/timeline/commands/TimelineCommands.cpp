#include "core/timeline/commands/TimelineCommands.h"

// ---------------------------------------------------------
// AddClipCommand
// ---------------------------------------------------------
AddClipCommand::AddClipCommand(TimelineClipModel* model, const TimelineClip& clip, QUndoCommand* parent)
    : QUndoCommand(parent), m_model(model), m_clip(clip), m_row(-1), m_firstTime(true)
{
    setText("Add Clip");
}

void AddClipCommand::undo() {
    if (m_row >= 0) {
        m_model->removeClip(m_row);
    }
}

void AddClipCommand::redo() {
    if (m_firstTime) {
        m_row = m_model->addClip(m_clip);
        m_firstTime = false;
    } else {
        m_model->insertClipAt(m_row, m_clip);
    }
}

// ---------------------------------------------------------
// RemoveClipCommand
// ---------------------------------------------------------
RemoveClipCommand::RemoveClipCommand(TimelineClipModel* model, int row, Type type, const QString& param, QUndoCommand* parent)
    : QUndoCommand(parent), m_model(model), m_row(row), m_type(type), m_param(param), m_firstTime(true)
{
    setText("Remove Clip");
}

void RemoveClipCommand::undo() {
    // Restore in reverse order to maintain correct indices
    for (int i = m_savedClips.size() - 1; i >= 0; --i) {
        const SavedClip& sc = m_savedClips[i];
        m_model->insertClipAt(sc.originalRow, sc.clip);
    }
}

void RemoveClipCommand::redo() {
    if (m_firstTime) {
        m_firstTime = false;
        
        // Find which clips will be removed and save them
        QVector<int> rowsToRemove;
        if (m_type == SingleRow) {
            if (m_model->clipAt(m_row).clipName != "") {
                rowsToRemove.append(m_row);
            }
        } else if (m_type == LinkedGroup) {
            rowsToRemove = m_model->getLinkedRows(m_param);
        } else if (m_type == FilePath) {
            for (int i = 0; i < m_model->rowCount(); ++i) {
                if (m_model->clipAt(i).filePath == m_param) {
                    rowsToRemove.append(i);
                }
            }
        } else if (m_type == All) {
            for (int i = 0; i < m_model->rowCount(); ++i) {
                rowsToRemove.append(i);
            }
        }
        
        // Sort ascending
        std::sort(rowsToRemove.begin(), rowsToRemove.end());
        
        for (int row : rowsToRemove) {
            m_savedClips.append({row, m_model->clipAt(row)});
        }
        
        // Now remove them in reverse order to not mess up indices
        for (int i = rowsToRemove.size() - 1; i >= 0; --i) {
            m_model->removeClip(rowsToRemove[i]);
        }
    } else {
        // Redo after undo: just remove the saved rows in reverse order
        for (int i = m_savedClips.size() - 1; i >= 0; --i) {
            m_model->removeClip(m_savedClips[i].originalRow);
        }
    }
}

// ---------------------------------------------------------
// MoveClipCommand
// ---------------------------------------------------------
MoveClipCommand::MoveClipCommand(TimelineClipModel* model, int row, double newStart, int newTrack, bool linked, QUndoCommand* parent)
    : QUndoCommand(parent), m_model(model), m_row(row), m_newStart(newStart), m_newTrack(newTrack), m_linked(linked), m_firstTime(true)
{
    setText("Move Clip");
}

void MoveClipCommand::undo() {
    for (const SavedState& state : m_savedStates) {
        m_model->updateClip(state.row, state.clip);
    }
}

void MoveClipCommand::redo() {
    if (m_firstTime) {
        m_firstTime = false;
        
        // Save current state
        if (m_linked) {
            QString linkGroup = m_model->linkGroupAt(m_row);
            QVector<int> rows = m_model->getLinkedRows(linkGroup);
            if (rows.isEmpty()) rows.append(m_row);
            
            for (int r : rows) {
                m_savedStates.append({r, m_model->clipAt(r)});
            }
        } else {
            m_savedStates.append({m_row, m_model->clipAt(m_row)});
        }
        
        // Let the model do the complex move logic once
        m_model->moveClip(m_row, m_newStart, m_newTrack, m_linked);
    } else {
        // We know what the result was, but it's easier to just call moveClip again
        // Wait! We can just call moveClip with the saved logic, but m_model->moveClip is deterministic.
        m_model->moveClip(m_row, m_newStart, m_newTrack, m_linked);
    }
}

// ---------------------------------------------------------
// TrimClipCommand
// ---------------------------------------------------------
TrimClipCommand::TrimClipCommand(TimelineClipModel* model, int row, double newStart, double newDuration, double newInPoint, bool linked, QUndoCommand* parent)
    : QUndoCommand(parent), m_model(model), m_row(row), m_newStart(newStart), m_newDuration(newDuration), m_newInPoint(newInPoint), m_linked(linked), m_firstTime(true)
{
    setText("Trim Clip");
}

void TrimClipCommand::undo() {
    for (const SavedState& state : m_savedStates) {
        m_model->updateClip(state.row, state.clip);
    }
}

void TrimClipCommand::redo() {
    if (m_firstTime) {
        m_firstTime = false;
        
        // Save current state
        if (m_linked) {
            QString linkGroup = m_model->linkGroupAt(m_row);
            QVector<int> rows = m_model->getLinkedRows(linkGroup);
            if (rows.isEmpty()) rows.append(m_row);
            
            for (int r : rows) {
                m_savedStates.append({r, m_model->clipAt(r)});
            }
        } else {
            m_savedStates.append({m_row, m_model->clipAt(m_row)});
        }
        
        // Let model do trim logic
        m_model->trimClip(m_row, m_newStart, m_newDuration, m_newInPoint, m_linked);
    } else {
        m_model->trimClip(m_row, m_newStart, m_newDuration, m_newInPoint, m_linked);
    }
}

// ---------------------------------------------------------
// SplitClipCommand
// ---------------------------------------------------------
SplitClipCommand::SplitClipCommand(TimelineClipModel* model, int row, double splitSeconds, const QString& rightLinkGroupId, QUndoCommand* parent)
    : QUndoCommand(parent), m_model(model), m_row(row), m_splitSeconds(splitSeconds), m_rightLinkGroupId(rightLinkGroupId), m_newRightRow(-1), m_firstTime(true)
{
    setText("Split Clip");
}

void SplitClipCommand::undo() {
    // Delete the new clip
    if (m_newRightRow >= 0) {
        m_model->removeClip(m_newRightRow);
        // Restore the original clip
        m_model->updateClip(m_row, m_originalClip);
    }
}

void SplitClipCommand::redo() {
    if (m_firstTime) {
        m_firstTime = false;
        m_originalClip = m_model->clipAt(m_row);
        m_newRightRow = m_model->splitClip(m_row, m_splitSeconds, m_rightLinkGroupId);
    } else {
        m_newRightRow = m_model->splitClip(m_row, m_splitSeconds, m_rightLinkGroupId);
    }
}




