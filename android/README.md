# Android Integration Guide

This directory contains examples and instructions for integrating the ULUI native library into an Android application.

## Building the Native Library

### Prerequisites

- Android NDK r21 or later
- CMake 3.20 or later
- Android SDK

### Build Steps

1. Set the Android NDK path:
```bash
export ANDROID_NDK=/path/to/android-ndk
```

2. Build for different architectures:

```bash
# ARM 64-bit (most modern devices)
mkdir -p build-android-arm64
cd build-android-arm64
cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_ANDROID_API=21 \
    -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# ARM 32-bit (older devices)
mkdir -p build-android-arm
cd build-android-arm
cmake .. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
    -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_ANDROID_API=21 \
    -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

3. The built libraries will be in:
   - `build-android-arm64/lib/libului_app.so`
   - `build-android-arm/lib/libului_app.so`

### Copy Libraries to Android Project

```bash
# Copy to your Android project's jniLibs directory
cp build-android-arm64/lib/libului_app.so /path/to/your/android/app/src/main/jniLibs/arm64-v8a/
cp build-android-arm/lib/libului_app.so /path/to/your/android/app/src/main/jniLibs/armeabi-v7a/

# Also copy shaders
cp -r shaders /path/to/your/android/app/src/main/assets/
```

## Integration in Android Studio

### 1. Project Structure

```
YourAndroidApp/
├── app/
│   ├── src/
│   │   ├── main/
│   │   │   ├── java/com/example/yourapp/
│   │   │   │   └── MainActivity.java
│   │   │   ├── jniLibs/
│   │   │   │   ├── arm64-v8a/
│   │   │   │   │   └── libului_app.so
│   │   │   │   └── armeabi-v7a/
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
            abiFilters 'arm64-v8a', 'armeabi-v7a'
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
