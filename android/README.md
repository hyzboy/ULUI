# Android Integration Guide

This directory contains examples and instructions for integrating the ULUI native code into an Android application.

ULUI supports **two deployment modes** for Android:
1. **Executable** - Standalone binary that can be run directly on Android
2. **Shared Library** - NativeActivity-based APK for app store distribution

**Note:** Only arm64-v8a (64-bit ARM) is supported. No armv7 (32-bit ARM) support.

## Quick Build

### Prerequisites

- Android NDK r21 or later
- CMake 3.20 or later
- Android SDK (for APK builds)

### Using the Build Script

```bash
# Set Android NDK path
export ANDROID_NDK=/path/to/android-ndk

# Build both executable and shared library (default)
./build-android.sh

# Build only executable
./build-android.sh Release executable

# Build only shared library
./build-android.sh Release shared

# Build both (explicit)
./build-android.sh Release both
```

## Build Mode 1: Android Executable

The executable can be pushed to an Android device and run directly without packaging into an APK.

### Manual Build

```bash
mkdir -p build-android-exe-arm64
cd build-android-exe-arm64
cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_ANDROID_API=21 \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_BUILD_SHARED=OFF
cmake --build . -j$(nproc)
```

### Deployment

```bash
# Push executable to device
adb push android-executable/arm64-v8a/ului_app /data/local/tmp/

# Push shaders
adb push android-executable/shaders /data/local/tmp/

# Run on device (may require root on some devices)
adb shell /data/local/tmp/ului_app
```

## Build Mode 2: Shared Library (NativeActivity APK)

The shared library is packaged into an APK and distributed via app stores.

### Manual Build

```bash
mkdir -p build-android-so-arm64
cd build-android-so-arm64
cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_ANDROID_API=21 \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_BUILD_SHARED=ON
cmake --build . -j$(nproc)
```

### Copy to Android Project

```bash
# Copy shared library
cp build-android-so-arm64/lib/libului_app.so android/app/src/main/jniLibs/arm64-v8a/

# Copy shaders
cp -r shaders android/app/src/main/assets/
```

## Integration in Android Studio (Shared Library Mode Only)

### 1. Project Structure

```
YourAndroidApp/
├── app/
│   ├── src/
│   │   ├── main/
│   │   │   ├── java/com/example/yourapp/
│   │   │   │   └── MainActivity.java (optional, not needed for NativeActivity)
│   │   │   ├── jniLibs/
│   │   │   │   └── arm64-v8a/
│   │   │   │       └── libului_app.so
│   │   │   ├── assets/
│   │   │   │   └── shaders/
│   │   │   │       ├── triangle.vert
│   │   │   │       └── triangle.frag
│   │   │   └── AndroidManifest.xml
│   │   └── build.gradle
│   └── build.gradle
└── settings.gradle
```

### 2. AndroidManifest.xml

See `app/src/main/AndroidManifest.xml` in this directory for a complete example.

Key requirements:
- Declare native activity
- Request OpenGL ES 3.0
- Set screen orientation (optional)

### 3. Loading the Library

The library uses `android_native_app_glue` and exports an `android_main` function. Android automatically loads and runs it when configured as a NativeActivity.

No Java/Kotlin code is needed - the entire application runs in native C++ code.

### 4. Gradle Configuration

See `app/build.gradle` for the complete configuration.

Important settings:
```gradle
android {
    defaultConfig {
        minSdkVersion 21  // Android 5.0 (for OpenGL ES 3.0)
        ndk {
            abiFilters 'arm64-v8a'  // Only arm64-v8a, no armv7
        }
    }
}
```

## Alternative: Using CMake in Android Studio

Instead of pre-building the library, you can configure Android Studio to build it directly:

### 1. Add to app/build.gradle:

```gradle
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                cppFlags "-std=c++20"
                arguments "-DCMAKE_BUILD_TYPE=Release"
            }
        }
    }
    
    externalNativeBuild {
        cmake {
            path "../../CMakeLists.txt"  // Path to ULUI CMakeLists.txt
            version "3.20.0"
        }
    }
}
```

### 2. Android Studio will:
- Automatically build the native library
- Package it with your APK
- Handle multi-architecture builds

## Running the Application

1. Connect an Android device or start an emulator
2. Install the APK:
```bash
./gradlew installDebug
```

3. Or run directly from Android Studio (Run → Run 'app')

## Debugging

### Viewing Logs

```bash
adb logcat | grep ULUI
```

### Debugging Native Code

1. In Android Studio: Run → Debug 'app'
2. Set breakpoints in C++ code
3. Use LLDB debugger

### Common Issues

**Library not found:**
- Verify .so files are in correct jniLibs directories
- Check ABI matches device architecture
- Ensure minSdkVersion is 21 or higher

**Shader loading fails:**
- Verify shaders are in assets/shaders/
- Check asset loading code in triangle_app.cpp

**EGL initialization fails:**
- Verify device supports OpenGL ES 3.0
- Check AndroidManifest.xml has correct OpenGL ES declaration

## Performance Tips

1. **Build in Release mode** for better performance
2. **Use arm64-v8a** for 64-bit devices (faster)
3. **Enable hardware acceleration** in AndroidManifest.xml (already included in example)

## Example Projects

See the `app/` directory for a complete example Android project that loads and uses the ULUI native library.

## Additional Resources

- [Android NDK Documentation](https://developer.android.com/ndk)
- [NativeActivity Guide](https://developer.android.com/ndk/guides/concepts#naa)
- [OpenGL ES on Android](https://developer.android.com/guide/topics/graphics/opengl)
