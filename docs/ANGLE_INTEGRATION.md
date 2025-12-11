# ANGLE Integration Guide

## Overview

This project is designed to use Google's ANGLE (Almost Native Graphics Layer Engine) for OpenGL ES 3.0 rendering. ANGLE provides a consistent OpenGL ES implementation across different platforms by translating OpenGL ES API calls to platform-native graphics APIs.

## Architecture

### ANGLE Translation Layers

ANGLE translates OpenGL ES 3.0 calls to:
- **Windows**: Direct3D 11
- **macOS**: Metal  
- **Linux**: Vulkan (with fallback to desktop OpenGL)

### Current Implementation

The project supports two modes:

1. **ANGLE Mode** (Preferred): Uses Google ANGLE libraries
2. **System Mode** (Fallback): Uses system-provided OpenGL ES or OpenGL libraries

## Installation

### Obtaining ANGLE

#### Option 1: Build from Source (Recommended for Production)

```bash
# Clone ANGLE repository
git clone https://chromium.googlesource.com/angle/angle
cd angle

# Install depot_tools
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=$PATH:$(pwd)/depot_tools

# Sync dependencies
python scripts/bootstrap.py
gclient sync

# Build ANGLE
gn gen out/Release --args='is_debug=false'
ninja -C out/Release
```

#### Option 2: Use Prebuilt Binaries

Download prebuilt ANGLE binaries from:
- [ANGLE Releases](https://github.com/google/angle/releases)
- Chrome/Chromium installations (includes ANGLE DLLs)

### Platform-Specific Installation

#### Windows

1. Download or build ANGLE:
   - `libEGL.dll`
   - `libGLESv2.dll`

2. Place DLLs in one of these locations:
   - Same directory as your executable
   - System PATH directory
   - Specify path via CMake: `-DANGLE_ROOT=C:/path/to/angle`

#### Linux

```bash
# Option 1: Use system OpenGL ES (Mesa)
sudo apt-get install libegl1-mesa-dev libgles2-mesa-dev

# Option 2: Build and install ANGLE
# After building ANGLE:
sudo cp out/Release/libEGL.so /usr/local/lib/
sudo cp out/Release/libGLESv2.so /usr/local/lib/
sudo ldconfig
```

#### macOS

```bash
# Build ANGLE from source (see above)
# Copy libraries to system location
sudo cp out/Release/libEGL.dylib /usr/local/lib/
sudo cp out/Release/libGLESv2.dylib /usr/local/lib/
```

## CMake Integration

The project automatically detects available OpenGL ES implementations:

```cmake
# CMake will search for libraries in this order:
1. ANGLE libraries (libEGL, libGLESv2)
2. System OpenGL ES libraries
3. Desktop OpenGL (fallback)
```

### Force ANGLE Usage

```bash
cmake .. -DUSE_ANGLE=ON -DANGLE_ROOT=/path/to/angle
```

### Disable ANGLE

```bash
cmake .. -DUSE_ANGLE=OFF
```

## Headers

The project includes minimal ANGLE headers in `external/angle/include/`:
- `EGL/egl.h` - EGL interface
- `GLES3/gl3.h` - OpenGL ES 3.0 interface

For production use, replace these with full ANGLE headers from the ANGLE repository.

## Verification

To verify ANGLE is being used, check the renderer string at runtime:

```cpp
const char* renderer = (const char*)glGetString(GL_RENDERER);
std::cout << "Renderer: " << renderer << std::endl;
```

Expected output with ANGLE:
- Windows: "ANGLE (Direct3D11 ...)"
- macOS: "ANGLE (Metal ...)"
- Linux: "ANGLE (Vulkan ...)" or "ANGLE (OpenGL ...)"

## Performance Considerations

### ANGLE Advantages
- Consistent behavior across platforms
- Better compatibility with older GPUs
- Security benefits (sandboxing)
- Used by Chrome/Chromium browsers

### ANGLE Limitations
- Small performance overhead from API translation
- Additional runtime dependencies
- Larger binary size

### When to Use Native OpenGL ES
- Maximum performance is critical
- Target platform has native OpenGL ES support
- Embedded systems (Android, iOS)

## Troubleshooting

### ANGLE Libraries Not Found

```bash
# Check library paths
export LD_LIBRARY_PATH=/path/to/angle/lib:$LD_LIBRARY_PATH  # Linux
export DYLD_LIBRARY_PATH=/path/to/angle/lib:$DYLD_LIBRARY_PATH  # macOS
```

### Runtime Errors

1. **"libEGL.so: cannot open shared object file"**
   - Install ANGLE or system EGL libraries
   - Add library directory to LD_LIBRARY_PATH

2. **"Failed to create EGL context"**
   - Check GPU driver is installed
   - Verify EGL configuration matches your system

3. **"Shader compilation failed"**
   - Verify shader syntax is GLSL ES 3.00
   - Check for platform-specific limitations

## Additional Resources

- [ANGLE Project](https://github.com/google/angle)
- [ANGLE Documentation](https://chromium.googlesource.com/angle/angle/+/HEAD/README.md)
- [OpenGL ES 3.0 Specification](https://www.khronos.org/opengles/)
- [EGL Specification](https://www.khronos.org/egl)
