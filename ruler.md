# Video Studio Development Rules for AI Assistants

This file contains critical guidelines that all future AI assistants MUST read and follow before modifying the codebase.

## 1. Respect Core Systems
- **DO NOT randomly change existing core files.** The user has spent days perfecting the core architecture, especially the **video timeline** and **video processing/playback engine**.
- Avoid touching deep mechanics in `PlaybackEngine`, `TimelineTracks`, or `TimelineClipModel` unless absolutely necessary and explicitly instructed.

## 2. Review and Understand Before Coding
- Take the time to thoroughly review and deeply understand the existing codebase architecture before proposing any solutions.
- Understand how the UI (QML) interacts with the backend (C++) before making assumptions about where a bug lives.

## 3. Extend, Don't Modify (Inheritance & Proxies)
- When adding new functionality (e.g., subtitles, new clip types, new effects), **create new files** instead of bloating existing ones.
- Use **inheritance** or create **proxy controllers** to interact with the core systems. 
- *Example*: When adding SRT parsing, we created a separate `SubtitleController` rather than hacking subtitle parsing directly into `PlaybackEngine` or `TimelineController`.

## 4. Prioritize Non-Destructive Changes
- Keep changes clean, modular, and reversible.
- If you must touch a core file, make the absolute minimum change required (e.g., adding a single API endpoint or a boolean check). 

## 5. UI State vs Export Pipeline Synchronization
- **Sync UI interactions to the Export Engine**: If you add interactive features in the QML UI (e.g., dragging subtitles, changing orientations), ensure these user preferences are explicitly passed to the headless export engines (like `VideoExporter`).
- **Use Controllers as State Managers**: Do not leave important state (like X/Y positions) isolated inside QML components. Store them in C++ controller objects (e.g., `SubtitleController`) so that both the live preview and the export rendering logic can access the exact same parameters.
- **Respect Layout Orientations**: Always account for both horizontal (16:9) and vertical (9:16) aspect ratios when calculating positions, margins, and export resolutions.

## 6. Subtitle Implementation is Complex
- **Be Extremely Careful with Subtitles**: Subtitle logic touches multiple complex systems (FFmpeg text rendering, Qt Quick text overlays, SRT parsing, dynamic UI positioning, scaling across aspect ratios, etc.).
- **Do Not Break Existing Workarounds**: The current subtitle implementation uses specific workarounds (like generating a concat file of subtitle image sequences) to support complex scripts (e.g., Khmer font). Do NOT rewrite this into a simpler `vf=subtitles=...` filter unless explicitly asked, as FFmpeg has known issues rendering complex text scripts directly.
- **Font & Style Caching**: Ensure any new subtitle features respect the global `SubtitleController` state. Do not create local font properties in QML that get lost during export.

## 7. Timeline Architecture (Unified Audio/Video Clips)
- **Do not split video and audio**: The video timeline uses a unified clip model. A single media file containing both video and audio is imported as ONE `TimelineClip` (with `hasVideo=true` and `hasAudio=true`), which perfectly renders both the filmstrip and the waveform in the UI within a single track.
- **DO NOT revert to separate tracks**: Earlier iterations split video and audio into two independent clips on different tracks. That is incorrect and breaks the "CapCut-like" unified UI experience. This unified implementation is working perfectly and MUST NOT be changed.
