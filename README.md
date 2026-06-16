
<h1 align="center">Video Studio</h1>

<p align="center">
  <a href="https://github.com/phirum11/video-editor/actions"><img src="https://github.com/phirum11/video-editor/workflows/Build/badge.svg" alt="Build Status"></a>
  <a href="https://www.qt.io/"><img src="https://img.shields.io/badge/Qt-6.5%2B-41CD52?logo=qt&logoColor=white" alt="Qt Version"></a>
  <a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white" alt="C++ Standard"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License"></a>
</p>
<p align="center">
  A modern, high-performance video editor built with C++, Qt, and QML. Video Studio Pro features a sleek, dark-themed user interface inspired by industry-leading editing software.
</p>

## ✨ Features

- **Modern UI**: A clean, fully-customized dark mode interface built with Qt Quick / QML.
- **Media Pool**: Import, preview, and organize your media assets. Includes real-time search filtering.
- **Magnetic Timeline**: Multi-track timeline supporting seamless drag-and-drop of video and audio clips.
- **Properties & Effects Hub**: Fine-tune your clips with precise controls for Scale, Position, Rotation, Anchor Points, and Opacity.
- **Hardware Accelerated Preview**: Real-time playback engine powered by FFmpeg and Qt Multimedia.

## 🚀 Getting Started

### Prerequisites
- CMake (3.16+)
- Qt 6.5 or higher (with QtQuick, QtMultimedia modules)
- FFmpeg libraries
- MinGW / GCC (for Windows builds)

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/phirum11/video-editor.git
   cd video-editor
   ```

2. Configure and build using CMake:
   ```bash
   mkdir build_mingw
   cd build_mingw
   cmake ..
   cmake --build .
   ```

3. Run the application:
   ```bash
   .\VideoStudio.exe
   ```

## 🏗️ Architecture Overview

The project is structured into clear, decoupled domains to ensure performance and maintainability:
- **`core/`**: The high-performance C++ backend. Handles the Timeline engine, Media Models, Playback, and Export rendering.
- **`ui/`**: The frontend layer built entirely with modern Qt Quick (QML) for a fluid, hardware-accelerated user experience.
- **`third_party/`**: External dependencies, primarily pre-compiled FFmpeg libraries used for media processing.
- **`tests/`**: C++ test probes for ensuring core engine stability.

## 🗺️ Roadmap

We have ambitious goals for Video Studio Pro! Here is what's coming next:
- [ ] Implement audio waveform rendering for timeline clips.
- [ ] Add Advanced keyframe animation for Transform and Color effects.
- [ ] Expand export formats (e.g., specific presets for YouTube, TikTok).
- [ ] Create a plugin system for custom third-party effects.

## 💬 Community

Want to discuss a feature or need help getting the project to build?
- Join the discussion on our [GitHub Discussions](https://github.com/phirum11/video-editor/discussions) board.

## 🤝 Contributing
Contributions are always welcome! Please read the `CONTRIBUTING.md` file for details on our guidelines and the process for submitting pull requests.

## 📝 License
This project is licensed under the MIT License - see the `LICENSE` file for details.
