# ULUI - Cross-Platform OpenGL ES 3.0 Project

A cross-platform OpenGL ES 3.0 project using Google's ANGLE implementation for rendering. Features a simple colored triangle as a demonstration.

## Features

- **Cross-platform support**: Windows, Linux, macOS, iOS, Android
- **Modern CMake** (3.20+) build system
- **C++20** standard
- **OpenGL ES 3.0** via Google ANGLE Project
- **GLFW** for desktop window management
- **Cross-platform file system** abstraction for assets and external files
- Simple colored triangle rendering example

## Requirements

### All Platforms
- CMake 3.20 or higher
- C++20 compatible compiler
- Google ANGLE libraries (libEGL, libGLESv2)

### Desktop Platforms (Windows/Linux/macOS)
- GLFW (automatically downloaded via CMake FetchContent)

### Windows
- Visual Studio 2019 or later / MinGW-w64
- Windows SDK

### Linux
- GCC 10+ or Clang 11+
- X11 development libraries
- Mesa or ANGLE libraries

### macOS
- Xcode 12+
- macOS 10.15+

### iOS
- Xcode 12+
- iOS SDK 13.0+

### Android
- Android NDK r21+
- Android SDK
- Gradle

## Building

### Desktop Platforms (Windows/Linux/macOS)

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./bin/ului_app  # Linux/macOS
# or
.\bin\ului_app.exe  # Windows
```

### Advanced Build Options

```bash
# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

# Specify compiler
cmake -DCMAKE_CXX_COMPILER=clang++ ..

# Disable ANGLE (use native OpenGL ES)
cmake -DUSE_ANGLE=OFF ..
```

### iOS

```bash
mkdir build-ios
cd build-ios

cmake .. \
    -GXcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0

# Open in Xcode
open ULUI.xcodeproj
```

### Android

The project compiles as a **shared library (.so)** for Android with **6 integration examples** showing different usage patterns.

**Note:** Only arm64-v8a is supported (no armv7).

#### Build Shared Library

```bash
# Set Android NDK path
export ANDROID_NDK=/path/to/android-ndk

# Build libului_app.so
./build-android.sh

# Library automatically copied to all 6 example projects
```

#### 6 Integration Examples

The `android/` directory contains 6 complete example projects:

1. **NativeActivity + SO-GL** - Pure native, .so manages OpenGL
2. **NativeActivity + App-GL** - Pure native, app manages OpenGL
3. **GameActivity + SO-GL** - Modern native game, .so manages OpenGL
4. **GameActivity + App-GL** - Modern native game, app manages OpenGL
5. **Java/GLSurfaceView + SO-GL** - Java app, .so manages OpenGL
6. **Java/GLSurfaceView + App-GL** - Java app, app manages OpenGL

**All examples use the same .so library** - only integration differs.

#### Run an Example

```bash
cd android/example-nativeactivity-so-gl
./gradlew installDebug
```

See [android/README.md](android/README.md) for detailed documentation.

For detailed instructions, see [android/README.md](android/README.md).

## Project Structure

```
ULUI/
├── CMakeLists.txt          # Root CMake configuration
├── README.md               # This file
├── README_CN.md            # Chinese documentation
├── LICENSE                 # License file
├── .gitignore             # Git ignore rules
├── build.sh               # Linux/macOS build script
├── build-android.sh       # Android build script
├── external/              # External dependencies
│   ├── CMakeLists.txt     # External dependencies CMake
│   └── angle/             # ANGLE headers
│       └── include/
│           ├── EGL/       # EGL headers
│           └── GLES3/     # OpenGL ES 3.0 headers
├── include/               # Public headers
│   ├── triangle_app.h     # Triangle application header
│   └── file_system.h      # Cross-platform file I/O abstraction
├── src/                   # Source files
│   ├── CMakeLists.txt     # Source CMake configuration
│   ├── main.cpp           # Main entry point (platform-specific)
│   ├── triangle_app.cpp   # Triangle rendering implementation
│   └── file_system.cpp    # File system implementation
├── shaders/               # GLSL shaders
│   ├── triangle.vert      # Vertex shader
│   └── triangle.frag      # Fragment shader
├── android/               # Android integration
│   ├── README.md          # Android build & integration guide
│   ├── build.gradle       # Gradle build file
│   ├── settings.gradle    # Gradle settings
│   └── app/               # Example Android app
│       ├── build.gradle   # App build configuration
│       └── src/main/
│           ├── AndroidManifest.xml
│           └── res/       # Android resources
└── docs/                  # Documentation
    ├── BUILD_GUIDE.md     # Detailed build guide
    ├── ANGLE_INTEGRATION.md
    ├── FILE_SYSTEM.md     # File system abstraction guide
    ├── FILE_SYSTEM_CN.md  # File system guide (Chinese)
    └── PROJECT_SUMMARY.md
```

## ANGLE Integration

This project uses Google's ANGLE (Almost Native Graphics Layer Engine) to provide OpenGL ES 3.0 support across all platforms. ANGLE translates OpenGL ES API calls to:
- **Windows**: Direct3D 11
- **macOS**: Metal
- **Linux**: Vulkan or native OpenGL

### Installing ANGLE

#### Windows
Download prebuilt ANGLE binaries from the ANGLE releases page or build from source.

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get install libegl1-mesa-dev libgles2-mesa-dev

# Or use ANGLE from source
```

#### macOS
```bash
# Using Homebrew (if available)
# Or build ANGLE from source
```

## File System Abstraction

ULUI provides a cross-platform file system abstraction that handles:

- **Internal Assets (Read-Only)**:
  - Android: APK assets
  - iOS: App bundle resources
  - Desktop: `assets/` directory next to executable

- **External Files (Read-Write)**:
  - Standard file system access on all platforms
  - Platform-specific user data directories

### Quick Example

```cpp
#include "file_system.h"
using namespace ului;

// Initialize
FileSystem::Initialize();

// Read shader from internal assets
std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");

// Write save file to external storage
std::string savePath = FileSystem::GetExternalDataPath() + "save.dat";
FileSystem::WriteExternalBinary(savePath.c_str(), saveData);
```

See [docs/FILE_SYSTEM.md](docs/FILE_SYSTEM.md) for complete documentation.

## Usage

The application creates a window and renders a colored triangle:
- **Red** vertex at the top
- **Green** vertex at bottom-left
- **Blue** vertex at bottom-right

Press **ESC** to close the application (desktop platforms).

## Customization

### Modifying the Triangle

Edit `src/triangle_app.cpp` to change:
- Vertex positions
- Colors
- Triangle shape

### Shaders

Modify GLSL shaders in the `shaders/` directory:
- `triangle.vert` - Vertex shader
- `triangle.frag` - Fragment shader

Both use GLSL ES 3.0 syntax (`#version 300 es`).

## Technical Details

- **C++ Standard**: C++20
- **CMake Version**: 3.20+
- **OpenGL ES Version**: 3.0
- **GLSL Version**: 3.00 ES
- **Window System**: GLFW 3.4 (desktop), Native (mobile)

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please ensure:
- Code follows C++20 best practices
- CMake configuration remains cross-platform
- Changes are tested on multiple platforms

## Troubleshooting

### GLFW not found
GLFW is automatically downloaded via CMake FetchContent. Ensure you have internet connectivity during configuration.

### ANGLE libraries not found
Make sure ANGLE is installed on your system or place the libraries in a location CMake can find them.

### Shader compilation errors
Check that shader files are copied to the build directory. CMake should handle this automatically.

### Black screen on startup
Verify that:
1. ANGLE/OpenGL ES drivers are properly installed
2. Your GPU supports OpenGL ES 3.0 or equivalent
3. Shader files are accessible in the `shaders/` directory

## References

- [ANGLE Project](https://github.com/google/angle)
- [OpenGL ES 3.0 Specification](https://www.khronos.org/opengles/)
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- [CMake Documentation](https://cmake.org/documentation/)
