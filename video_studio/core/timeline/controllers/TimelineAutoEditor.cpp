#include "TimelineAutoEditor.h"
#include "core/timeline/models/TimelineClipModel.h"
#include "core/timeline/commands/TimelineCommands.h"
#include <QUndoStack>
#include <QVector>
#include <QDebug>
#include <algorithm>

struct ClipInfo {
    int row;
    double startSeconds;
    double durationSeconds;
};

void TimelineAutoEditor::syncVisualsToAudio(TimelineClipModel* model, QUndoStack* undoStack)
{
    if (!model || !undoStack) return;

    QVector<ClipInfo> audioClips;
    QVector<ClipInfo> visualClips;

    // 1. Gather clips
    for (int i = 0; i < model->rowCount(); ++i) {
        TimelineClip clip = model->clipAt(i);
        
        // Find voice clips (usually "Generated Voice" or hasAudio but no video)
        if (clip.clipName == "Generated Voice" || (clip.hasAudio && !clip.hasVideo)) {
            audioClips.append({i, clip.startSeconds, clip.durationSeconds});
        }
        // Find visual clips (has video, usually on track 1 or 0)
        else if (clip.hasVideo) {
            visualClips.append({i, clip.startSeconds, clip.durationSeconds});
        }
    }

    if (audioClips.isEmpty() || visualClips.isEmpty()) {
        qDebug() << "TimelineAutoEditor: Not enough clips to synchronize.";
        return;
    }

    // Sort by start time
    std::sort(audioClips.begin(), audioClips.end(), [](const ClipInfo& a, const ClipInfo& b) {
        return a.startSeconds < b.startSeconds;
    });
    std::sort(visualClips.begin(), visualClips.end(), [](const ClipInfo& a, const ClipInfo& b) {
        return a.startSeconds < b.startSeconds;
    });

    undoStack->beginMacro("Auto Edit (Sync Visuals)");

    int visualIndex = 0;
    
    // 2. Adjust visual clips to match audio clips
    for (int i = 0; i < audioClips.size(); ++i) {
        const ClipInfo& audioClip = audioClips[i];
        
        // Loop visual clips if there are fewer visuals than audios
        ClipInfo& visualClip = visualClips[visualIndex % visualClips.size()];
        
        double targetStart = audioClip.startSeconds;
        double targetDuration = audioClip.durationSeconds;
        
        // If there's a gap between this audio clip and the next one, stretch the visual clip to cover it
        if (i < audioClips.size() - 1) {
            double nextAudioStart = audioClips[i + 1].startSeconds;
            if (nextAudioStart > (targetStart + targetDuration)) {
                targetDuration = nextAudioStart - targetStart;
            }
        }

        // If we need to loop/duplicate a visual clip, we must actually add a new clip to the model.
        // Wait, for simplicity, let's just move/trim the existing ones, and if we run out, 
        // we can copy the clip using AddClipCommand.
        
        if (visualIndex < visualClips.size()) {
            // Modify existing clip
            // Trim clip updates start, duration, and inPoint.
            // Move clip updates start and track.
            // We can just use TrimClipCommand
            double currentInPoint = model->clipAt(visualClip.row).sourceInPoint;
            undoStack->push(new TrimClipCommand(model, visualClip.row, targetStart, targetDuration, currentInPoint, false));
        } else {
            // We need to duplicate the clip
            TimelineClip newClip = model->clipAt(visualClip.row);
            newClip.startSeconds = targetStart;
            newClip.durationSeconds = targetDuration;
            undoStack->push(new AddClipCommand(model, newClip));
        }

        visualIndex++;
    }

    // 3. Remove any leftover visual clips if there were more visuals than audios
    // We must remove them in reverse order so row indices don't shift improperly
    QVector<int> rowsToRemove;
    for (int i = visualIndex; i < visualClips.size(); ++i) {
        rowsToRemove.append(visualClips[i].row);
    }
    
    // Sort in descending order to avoid index shifting issues when removing
    std::sort(rowsToRemove.begin(), rowsToRemove.end(), std::greater<int>());
    
    for (int row : rowsToRemove) {
        undoStack->push(new RemoveClipCommand(model, row, RemoveClipCommand::SingleRow));
    }

    undoStack->endMacro();
}
