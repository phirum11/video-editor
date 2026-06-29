#include "ClipDragController.h"

ClipDragController::ClipDragController(QObject *parent)
    : QObject(parent)
{
}

void ClipDragController::beginDrag(TimelineController* timeline, double startX, double startY)
{
    m_dragStates.clear();
    m_ghostX = startX;
    m_ghostY = startY;
    m_dragDeltaSeconds = 0.0;
    m_dragDeltaTrack = 0;
    emit ghostXChanged();
    emit ghostYChanged();
    emit dragDeltaSecondsChanged();
    emit dragDeltaTrackChanged();

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

void ClipDragController::updateDrag(TimelineController* timeline, double deltaSeconds, int deltaTrack, double curX, double curY)
{
    if (m_ghostX != curX) {
        m_ghostX = curX;
        emit ghostXChanged();
    }
    if (m_ghostY != curY) {
        m_ghostY = curY;
        emit ghostYChanged();
    }
    if (m_dragDeltaSeconds != deltaSeconds) {
        m_dragDeltaSeconds = deltaSeconds;
        emit dragDeltaSecondsChanged();
    }
    if (m_dragDeltaTrack != deltaTrack) {
        m_dragDeltaTrack = deltaTrack;
        emit dragDeltaTrackChanged();
    }

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
    
    // 1. Temporarily revert to original state so moveSelectedClips computes the correct delta
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
    m_dragDeltaSeconds = 0.0;
    m_dragDeltaTrack = 0;
    emit isDraggingChanged();
    emit dragDeltaSecondsChanged();
    emit dragDeltaTrackChanged();
}
