# Android Integration Examples

This directory contains 6 different example projects demonstrating various ways to integrate the ULUI shared library (libului_app.so) into Android applications.

**All examples use the same shared library** - only the integration approach differs.

## Supported Architecture

- **arm64-v8a only** (64-bit ARM)
- No armv7 support

## Example Projects

### 1. NativeActivity - OpenGL Initialized by .so
**Directory**: `example-nativeactivity-so-gl/`

- Uses Android NativeActivity
- OpenGL ES context created and managed by the native .so library
- No Java/Kotlin code needed
- Best for pure native apps

### 2. NativeActivity - OpenGL Initialized by App  
**Directory**: `example-nativeactivity-app-gl/`

- Uses Android NativeActivity
- OpenGL ES context created by the Android app framework
- .so library uses existing context
- Useful when you need app-level GL management

### 3. GameActivity - OpenGL Initialized by .so
**Directory**: `example-gameactivity-so-gl/`

- Uses Android GameActivity (modern replacement for NativeActivity)
- OpenGL ES context created and managed by the native .so library
- Better input handling than NativeActivity
- Recommended for new game projects

### 4. GameActivity - OpenGL Initialized by App
**Directory**: `example-gameactivity-app-gl/`

- Uses Android GameActivity
- OpenGL ES context created by the Android app framework
- .so library uses existing context
- Best control over app lifecycle

### 5. Java/Kotlin App - OpenGL Initialized by .so
**Directory**: `example-java-so-gl/`

- Uses GLSurfaceView with Java/Kotlin activity
- OpenGL ES context created by the native .so library
- Java layer provides UI and app logic
- Native layer handles all rendering

### 6. Java/Kotlin App - OpenGL Initialized by App
**Directory**: `example-java-app-gl/`

- Uses GLSurfaceView with Java/Kotlin activity
- OpenGL ES context created by GLSurfaceView
- Native library called for rendering only
- Most common integration pattern for mixed apps

## Quick Start

### 1. Build the Shared Library

```bash
# Set Android NDK path
export ANDROID_NDK=/path/to/android-ndk

# Build libului_app.so
./build-android.sh

# The script automatically copies the .so to all 6 example projects
```

### 2. Build and Run an Example

```bash
# Choose an example project
cd android/example-nativeactivity-so-gl

# Build and install
./gradlew installDebug

# Or build APK only
./gradlew assembleDebug
```

## Project Structure

Each example project has the following structure:

```
example-xxx/
├── app/
│   ├── build.gradle
│   └── src/
│       └── main/
│           ├── AndroidManifest.xml
│           ├── java/          (Java/Kotlin examples only)
│           ├── jniLibs/
│           │   └── arm64-v8a/
│           │       └── libului_app.so
│           └── assets/
│               └── shaders/
├── build.gradle
└── settings.gradle
```

## Comparison Matrix

| Example | Activity Type | GL Init | Java Code | Use Case |
|---------|--------------|---------|-----------|----------|
| 1 | NativeActivity | .so | None | Pure native app |
| 2 | NativeActivity | App | Minimal | Native with GL control |
| 3 | GameActivity | .so | None | Modern native game |
| 4 | GameActivity | App | Minimal | Modern with GL control |
| 5 | GLSurfaceView | .so | Medium | Hybrid app, native rendering |
| 6 | GLSurfaceView | App | Medium | Standard Android GL app |

## Technical Details

### Shared Library Interface

The `libului_app.so` library exports:

- `android_main(android_app*)` - For NativeActivity/GameActivity with .so GL init
- `ANativeActivity_onCreate()` - Standard native activity entry point
- JNI functions for Java integration (examples 5 & 6)

### OpenGL ES Version

All examples use **OpenGL ES 3.0** via ANGLE or system OpenGL ES.

### Minimum Requirements

- Android API Level 21 (Android 5.0)
- OpenGL ES 3.0 support
- 64-bit ARM device (arm64-v8a)

## Choosing the Right Example

**For a new game project:**
- Start with `example-gameactivity-so-gl` (Example 3)

**For maximum control:**
- Use `example-java-app-gl` (Example 6) with GLSurfaceView

**For pure native C++ app:**
- Use `example-nativeactivity-so-gl` (Example 1)

**For existing Android app integration:**
- Use `example-java-so-gl` (Example 5) or `example-java-app-gl` (Example 6)

## Building for Release

```bash
# Build release APK
cd android/example-xxx
./gradlew assembleRelease

# APK will be in app/build/outputs/apk/release/
```

## Troubleshooting

### Library not found
- Ensure `./build-android.sh` completed successfully
- Check `app/src/main/jniLibs/arm64-v8a/libului_app.so` exists

### OpenGL errors
- Verify device supports OpenGL ES 3.0
- Check logcat for detailed error messages: `adb logcat | grep ULUI`

### Shader loading fails
- Verify shaders are in `app/src/main/assets/shaders/`
- Check asset paths in code

## Additional Resources

- [Android NDK Documentation](https://developer.android.com/ndk)
- [NativeActivity Guide](https://developer.android.com/ndk/guides/concepts#naa)
- [GameActivity Documentation](https://developer.android.com/games/agdk/game-activity)
- [OpenGL ES on Android](https://developer.android.com/guide/topics/graphics/opengl)
