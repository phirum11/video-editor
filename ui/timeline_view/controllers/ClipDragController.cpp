#include "ClipDragController.h"

ClipDragController::ClipDragController(QObject *parent)
    : QObject(parent)
{
}

void ClipDragController::beginDrag(TimelineController* timeline)
{
    m_dragStates.clear();
    if (!timeline || !timeline->clipModel()) return;

    auto* model = static_cast<TimelineClipModel*>(timeline->clipModel());
    QVariantList selectedIndices = timeline->selectedClipIndices();
    
    for (const QVariant& v : selectedIndices) {
        int row = v.toInt();
        if (row >= 0 && row < model->rowCount()) {
            TimelineClip clip = model->clipAt(row);
            DragState state;
            state.row = row;
            state.originalStartSeconds = clip.startSeconds;
            state.originalTrackIndex = clip.trackIndex;
            m_dragStates.append(state);
        }
    }

    if (!m_dragStates.isEmpty()) {
        m_isDragging = true;
        emit isDraggingChanged();
    }
}

void ClipDragController::updateDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack)
{
    if (!timeline || !timeline->clipModel() || m_dragStates.isEmpty()) return;

    auto* model = static_cast<TimelineClipModel*>(timeline->clipModel());
    
    for (const DragState& state : m_dragStates) {
        if (state.row >= 0 && state.row < model->rowCount()) {
            TimelineClip clip = model->clipAt(state.row);
            clip.startSeconds = std::max(0.0, state.originalStartSeconds + deltaSeconds);
            clip.trackIndex = std::max(0, state.originalTrackIndex + deltaTrack);
            model->updateClip(state.row, clip);
        }
    }
}

void ClipDragController::endDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack, bool linked)
{
    if (!timeline || !timeline->clipModel() || m_dragStates.isEmpty()) return;

    auto* model = static_cast<TimelineClipModel*>(timeline->clipModel());
    
    // 1. Temporarily revert to original state
    for (const DragState& state : m_dragStates) {
        if (state.row >= 0 && state.row < model->rowCount()) {
            TimelineClip clip = model->clipAt(state.row);
            clip.startSeconds = state.originalStartSeconds;
            clip.trackIndex = state.originalTrackIndex;
            model->updateClip(state.row, clip);
        }
    }
    
    // 2. Clear state so we don't hold dangling indices
    m_dragStates.clear();

    // 3. Commit actual move with undo macro!
    timeline->moveSelectedClips(deltaSeconds, deltaTrack, linked);

    m_isDragging = false;
    emit isDraggingChanged();
}
