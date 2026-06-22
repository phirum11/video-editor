import os
import re

replacements = {
    "core/actions/managers/ActionManager": "core/actions/managers/ActionManager",
    "core/actions/managers/MenuManager": "core/actions/managers/MenuManager",
    "core/actions/handlers/FileActions": "core/actions/handlers/FileActions",
    "core/actions/handlers/EditActions": "core/actions/handlers/EditActions",
    "core/actions/handlers/ClipActions": "core/actions/handlers/ClipActions",
    "core/actions/handlers/SequenceActions": "core/actions/handlers/SequenceActions",
    "core/actions/handlers/ViewActions": "core/actions/handlers/ViewActions",
    "core/actions/handlers/ToolActions": "core/actions/handlers/ToolActions",
    "core/actions/handlers/HelpActions": "core/actions/handlers/HelpActions",
    
    "core/effects/processors/EffectProcessor": "core/effects/processors/EffectProcessor",
    "core/effects/controllers/EffectController": "core/effects/controllers/EffectController",
    "core/effects/models/EffectData.h": "core/effects/models/EffectData.h",
    
    "core/export/encoders/HardwareEncoder.h": "core/export/encoders/HardwareEncoder.h",
    "core/export/encoders/AmdEncoder": "core/export/encoders/AmdEncoder",
    "core/export/encoders/IntelEncoder": "core/export/encoders/IntelEncoder",
    "core/export/encoders/NvidiaEncoder": "core/export/encoders/NvidiaEncoder",
    "core/export/encoders/HardwareDetector": "core/export/encoders/HardwareDetector",
    "core/export/renderers/AdvancedSubtitleRenderer": "core/export/renderers/AdvancedSubtitleRenderer",
    "core/export/exporters/VideoExporter": "core/export/exporters/VideoExporter",
    
    "core/media/models/MediaItem": "core/media/models/MediaItem",
    "core/media/models/MediaListModel": "core/media/models/MediaListModel",
    "core/media/providers/AudioWaveformProvider": "core/media/providers/AudioWaveformProvider",
    "core/media/managers/MediaManager": "core/media/managers/MediaManager",
    
    "core/playback/engines/AudioEngine": "core/playback/engines/AudioEngine",
    "core/playback/engines/PlaybackEngine": "core/playback/engines/PlaybackEngine",
    
    "core/timeline/models/Clip": "core/timeline/models/Clip",
    "core/timeline/models/Track": "core/timeline/models/Track",
    "core/timeline/models/TimelineClipModel": "core/timeline/models/TimelineClipModel",
    "core/timeline/commands/TimelineCommands": "core/timeline/commands/TimelineCommands",
    "core/timeline/managers/TimelineManager": "core/timeline/managers/TimelineManager"
}

# Create a mapping for local includes (e.g., #include "core/actions/managers/ActionManager.h" -> #include "core/actions/managers/ActionManager.h")
local_includes = {}
for old_path, new_path in replacements.items():
    if old_path.endswith(".h"):
        basename = old_path.split("/")[-1]
        local_includes[f'#include "{basename}"'] = f'#include "{new_path}"'
    else:
        basename = old_path.split("/")[-1]
        local_includes[f'#include "{basename}.h"'] = f'#include "{new_path}.h"'

directory = r"c:\we_hunting\video_studio"

for root, _, files in os.walk(directory):
    if "build" in root or ".git" in root:
        continue
    for file in files:
        if file.endswith((".cpp", ".h", ".qml", ".txt", ".py")):
            filepath = os.path.join(root, file)
            if filepath == os.path.abspath(__file__):
                continue
            
            try:
                with open(filepath, "r", encoding="utf-8") as f:
                    content = f.read()
            except Exception:
                continue
                
            original_content = content
            
            # Replace full paths (e.g. core/actions/managers/ActionManager.cpp)
            for old_path, new_path in replacements.items():
                content = content.replace(f"{old_path}.cpp", f"{new_path}.cpp")
                content = content.replace(f"{old_path}.h", f"{new_path}.h")
                # Also without extension for some cases
                content = content.replace(old_path, new_path)
                
            # Replace local includes (e.g. #include "core/actions/managers/ActionManager.h")
            if file.endswith((".cpp", ".h", ".py")):
                for old_inc, new_inc in local_includes.items():
                    content = content.replace(old_inc, new_inc)
            
            if content != original_content:
                with open(filepath, "w", encoding="utf-8", newline="") as f:
                    f.write(content)
                print(f"Updated {filepath}")
