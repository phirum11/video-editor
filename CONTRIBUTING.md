# Contributing to Video Studio

First off, thank you for considering contributing to Video Studio! It's people like you that make open source such a great community to learn, inspire, and create.

## How Can I Contribute?

As a contributor, you are highly encouraged to push code, fix bugs, add new features, and help improve the project overall. All contributions are welcome!

### Reporting Bugs
This section guides you through submitting a bug report for Video Studio.
- Ensure the bug was not already reported by searching on GitHub under Issues.
- If you're unable to find an open issue addressing the problem, open a new one. Be sure to include a title and clear description, as much relevant information as possible, and a code sample or an executable test case demonstrating the expected behavior that is not occurring.

### Suggesting Enhancements
This section guides you through submitting an enhancement suggestion for Video Studio, including completely new features and minor improvements to existing functionality.
- Open a new Issue and provide a clear and detailed explanation of the feature you want and why it's important to add.
- Keep the scope of the feature well-defined.

### Pull Requests
1. Fork the repo and create your branch from `master`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes.
5. Issue that pull request!

## Setting Up Your Development Environment
To build the project locally, you will need to set up your C++ environment, Qt, and FFmpeg.

### 1. Prerequisites
- **CMake (3.16+)**
- **A C++17 compatible compiler** (MinGW or MSVC for Windows, GCC for Linux, Clang for macOS)

### 2. Installing Qt
The project specifically requires **Qt 6.5 or higher** (tested primarily on 6.7.0).
1. Download the [Qt Online Installer](https://www.qt.io/download-qt-installer).
2. During installation, select a Qt 6 version and make sure to include the following modules:
   - `Qt Core` / `Qt Gui` / `Qt Widgets`
   - `Qt QML` / `Qt Quick` / `Qt Quick Controls 2` / `Qt Quick Dialogs 2`
   - `Qt Multimedia`

### 3. Setting up FFmpeg
Because FFmpeg binaries are large, they are not included directly in the git repository. **You must download them manually** before CMake will configure successfully.
1. Download the FFmpeg shared dev libraries for your platform.
   - For Windows: We recommend downloading the `ffmpeg-master-latest-win64-gpl-shared.zip` from [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds/releases).
2. Extract the files and place them inside the `video_studio/third_party/ffmpeg/` directory so your structure looks exactly like this:
   ```
   video_studio/
   └── third_party/
       └── ffmpeg/
           ├── bin/      (contains avcodec-61.dll, etc.)
           ├── include/  (contains libavcodec, libavformat headers)
           └── lib/      (contains avcodec.lib, avformat.lib)
   ```
*(Note: CMake will automatically copy the `.dll` files from `bin/` to your build output directory after compiling).*

## Coding Guidelines
- Please use clear variable names and keep functions small and focused.
- Adhere to the existing C++ styling (we follow standard Qt conventions).
- Document new methods and classes.

Thank you for contributing!
