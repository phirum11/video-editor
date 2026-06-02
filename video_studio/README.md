# Video Studio Project

Welcome to the `video_studio` source tree. This is the foundational structure for building a high-performance, professional-grade desktop video editing application.

## Directory Layout Overview

### `core/`
The high-performance heart of the application. Responsible for fast memory access, complex multithreading, and direct communication with hardware.
- **playback/**: Synchronizes audio and video tracks, manages the real-time playback clock, and ensures smooth framing without lag.
- **rendering/**: Interacts with lower-level graphics APIs (Vulkan, Metal, DirectX) to draw the composited video frames using GPU acceleration.
- **timeline/**: Manages the abstract data representation of sequences, clips, cuts, and keyframes.
- **media/**: Interface wrappers for `FFmpeg`. Handles demuxing, decoding, and encoding of varying media formats.

### `ui/`
The dark-mode desktop interface that the user interacts with.
- **timeline_view/**: The complex, horizontally-scrolling component where clips are dragged, dropped, and trimmed.
- **media_pool/**: For importing, organizing, and searching source video/audio assets.
- **monitors/**: The dual-screen setup often seen in professional editors (Source monitor for raw clips, Program monitor for the final timeline).
- **inspector/**: Right-side panel containing granular controls (color wheels, audio sliders, effect parameters).

### `effects/`
Pluggable DSP (Digital Signal Processing) and video filtering algorithms.
- **color/**: Implementation of color correction algorithms like Lift/Gamma/Gain, HSL curves, and LUT parsing.
- **transitions/**: Code generating visual transitions (e.g., cross-dissolve, dip to black).
- **audio/**: Filters for EQ, compression, or spatial mixing.

### `io/`
Handles moving data into and out of the application state.
- **project_manager/**: Serializes the timeline state and asset links to a persistent file (e.g., SQLite or JSON).
- **export/**: Manages the offline rendering queue to compile the final video file.

### `assets/` & `tests/` & `docs/`
- **assets/**: UI themes, fonts, SVG icons.
- **tests/**: Crucial for an application with heavy multithreading—contains unit, integration, and performance benchmarking tests.
- **docs/**: API documentation for module boundaries.

## Next Steps
To begin development:
1. Initialize the build system for the chosen tech stack inside this root directory.
2. Link the core `FFmpeg` libraries in `core/media`.
3. Scaffold the basic main window layout inside `ui/`.
