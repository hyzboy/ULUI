# ULUI - Cross-Platform OpenGL ES 3.0 Project

A cross-platform OpenGL ES 3.0 project using Google's ANGLE implementation for rendering. Features a simple colored triangle as a demonstration.

## Features

- **Cross-platform support**: Windows, Linux, macOS, iOS, Android
- **Modern CMake** (3.20+) build system
- **C++20** standard
- **OpenGL ES 3.0** via Google ANGLE Project
- **GLFW** for desktop window management
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

The project builds as a shared library (.so) for Android, which can be loaded by an Android application.

#### Quick Build with Script

```bash
# Set Android NDK path
export ANDROID_NDK=/path/to/android-ndk

# Build for all architectures
./build-android.sh

# Libraries will be in android/app/src/main/jniLibs/
```

#### Manual Build

```bash
mkdir build-android-arm64
cd build-android-arm64

cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_NDK=/path/to/ndk \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_ANDROID_API=21

cmake --build .
```

#### Integration in Android Project

See the `android/` directory for a complete example Android project. The library is loaded via NativeActivity.

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
│   └── triangle_app.h     # Triangle application header
├── src/                   # Source files
│   ├── CMakeLists.txt     # Source CMake configuration
│   ├── main.cpp           # Main entry point (platform-specific)
│   └── triangle_app.cpp   # Triangle rendering implementation
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
