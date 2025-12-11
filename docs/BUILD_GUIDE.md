# Build Guide

Complete guide for building ULUI on all supported platforms.

## Prerequisites

### All Platforms
- CMake 3.20 or higher
- C++20 compatible compiler
- Git (for fetching dependencies)

### Platform-Specific Requirements

#### Windows
- **Visual Studio 2019+** or **Visual Studio 2022** (recommended)
- Windows SDK 10.0.19041.0 or later
- Alternative: MinGW-w64 with GCC 10+

#### Linux
- GCC 10+ or Clang 11+
- Development libraries:
  ```bash
  sudo apt-get install build-essential cmake git
  sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev 
  sudo apt-get install libxcursor-dev libxi-dev
  sudo apt-get install libegl1-mesa-dev libgles2-mesa-dev
  ```

#### macOS
- Xcode 12+ with Command Line Tools
- macOS 10.15 (Catalina) or later
- Install Xcode Command Line Tools:
  ```bash
  xcode-select --install
  ```

#### iOS
- Xcode 12+
- iOS SDK 13.0+
- Valid Apple Developer account (for device deployment)

#### Android
- Android Studio 4.0+
- Android NDK r21 or later
- Android SDK API Level 21+ (Android 5.0+)
- Gradle 7.0+

## Building on Desktop Platforms

### Quick Build

```bash
# Clone repository (if not already done)
git clone https://github.com/hyzboy/ULUI.git
cd ULUI

# Use the provided build script (Linux/macOS)
./build.sh

# Or on Windows
build.bat
```

### Manual Build

#### Linux / macOS

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)  # Linux
cmake --build . -j$(sysctl -n hw.ncpu)  # macOS

# Run
./bin/ului_app
```

#### Windows (Visual Studio)

```powershell
# Create build directory
mkdir build
cd build

# Configure (generates Visual Studio solution)
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Run
.\bin\Release\ului_app.exe
```

#### Windows (MinGW)

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j%NUMBER_OF_PROCESSORS%

# Run
.\bin\ului_app.exe
```

## Advanced Build Options

### Build Types

```bash
# Debug build (with debug symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Release with debug info
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Minimum size release
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel
```

### Compiler Selection

```bash
# Use Clang instead of GCC (Linux/macOS)
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Use specific GCC version
cmake .. -DCMAKE_CXX_COMPILER=g++-11 -DCMAKE_C_COMPILER=gcc-11
```

### ANGLE Configuration

```bash
# Use ANGLE (default)
cmake .. -DUSE_ANGLE=ON

# Use native OpenGL/OpenGL ES
cmake .. -DUSE_ANGLE=OFF

# Specify ANGLE installation path
cmake .. -DANGLE_ROOT=/path/to/angle
```

## Building for Mobile Platforms

### iOS

```bash
mkdir build-ios
cd build-ios

# Configure for iOS device
cmake .. \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_OSX_ARCHITECTURES=arm64

# Build
cmake --build . --config Release

# Or open in Xcode
open ULUI.xcodeproj
```

#### iOS Simulator

```bash
cmake .. \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphonesimulator \
    -DCMAKE_OSX_ARCHITECTURES=x86_64
```

### Android

#### Using CMake Directly

```bash
mkdir build-android
cd build-android

cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_ANDROID_NDK=/path/to/ndk \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_ANDROID_API=21

cmake --build .
```

#### Android ABIs

Build for different architectures:

```bash
# ARM 64-bit (most modern devices)
-DCMAKE_ANDROID_ARCH_ABI=arm64-v8a

# ARM 32-bit (older devices)
-DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a

# x86 64-bit (emulator)
-DCMAKE_ANDROID_ARCH_ABI=x86_64

# x86 32-bit (older emulator)
-DCMAKE_ANDROID_ARCH_ABI=x86
```

## Cross-Compilation

### Linux → Windows (MinGW)

```bash
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

### macOS Universal Binary

```bash
cmake .. \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_BUILD_TYPE=Release
```

## Troubleshooting

### Common Issues

#### CMake Version Too Old
```bash
# Install newer CMake
pip install cmake --upgrade
# Or download from https://cmake.org/download/
```

#### Missing Dependencies (Linux)
```bash
# Install all required development packages
sudo apt-get update
sudo apt-get install build-essential cmake git \
    libx11-dev libxrandr-dev libxinerama-dev \
    libxcursor-dev libxi-dev libgl1-mesa-dev \
    libegl1-mesa-dev libgles2-mesa-dev
```

#### GLFW Build Fails
- Ensure X11 development libraries are installed (Linux)
- Check CMake output for missing dependencies

#### Shader Files Not Found
- Shaders are automatically copied to build directory
- Check: `build/bin/shaders/`
- Manual copy: `cp -r shaders build/bin/`

### Clean Build

```bash
# Remove build directory and start fresh
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
```

## IDE Integration

### Visual Studio Code

1. Install C/C++ and CMake Tools extensions
2. Open project folder
3. Select kit (compiler)
4. Configure and build from status bar

### CLion

1. Open project folder (CLion detects CMake)
2. Wait for CMake configuration
3. Build from Build menu or Shift+F10

### Visual Studio

1. Open folder (VS 2019+) or CMake project
2. Select configuration from toolbar
3. Build → Build All

### Xcode

```bash
cd build
cmake .. -G Xcode
open ULUI.xcodeproj
```

## Performance Tips

### Release Builds

Always use Release builds for performance testing:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Link-Time Optimization

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Native Architecture

```bash
# Optimize for current CPU (not portable!)
cmake .. -DCMAKE_CXX_FLAGS="-march=native"
```

## Continuous Integration

### GitHub Actions Example

```yaml
- name: Install Dependencies (Linux)
  run: |
    sudo apt-get update
    sudo apt-get install -y libx11-dev libxrandr-dev \
      libxinerama-dev libxcursor-dev libxi-dev \
      libegl1-mesa-dev libgles2-mesa-dev

- name: Configure CMake
  run: cmake -B build -DCMAKE_BUILD_TYPE=Release

- name: Build
  run: cmake --build build --config Release

- name: Test
  run: cd build && ctest -C Release
```

## Next Steps

After successful build:
1. Run the application: `./build/bin/ului_app`
2. Check [ANGLE_INTEGRATION.md](ANGLE_INTEGRATION.md) for ANGLE setup
3. Read [README.md](../README.md) for usage instructions
4. Modify shaders in `shaders/` directory
5. Extend the triangle example in `src/triangle_app.cpp`
