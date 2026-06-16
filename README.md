<p align="center">
  <img src="https://github.com/phirum11/video-editor/raw/master/screenshot.png" alt="Video Studio Pro UI">
</p>

<h1 align="center">Video Studio Pro 🎬</h1>

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

## 🤝 Contributing
Contributions are always welcome! Please read the `CONTRIBUTING.md` file for details on our guidelines and the process for submitting pull requests.

## 📝 License
This project is licensed under the MIT License - see the `LICENSE` file for details.
