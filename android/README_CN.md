# Android 集成指南

本目录包含将 ULUI 原生库集成到 Android 应用程序的示例和说明。

## 构建原生库

### 前置要求

- Android NDK r21 或更高版本
- CMake 3.20 或更高版本
- Android SDK

### 构建步骤

1. 设置 Android NDK 路径：
```bash
export ANDROID_NDK=/path/to/android-ndk
```

2. 为不同架构构建：

```bash
# ARM 64位（大多数现代设备）
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

# ARM 32位（旧设备）
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

3. 构建的库将位于：
   - `build-android-arm64/lib/libului_app.so`
   - `build-android-arm/lib/libului_app.so`

### 复制库到 Android 项目

```bash
# 复制到您的 Android 项目的 jniLibs 目录
cp build-android-arm64/lib/libului_app.so /path/to/your/android/app/src/main/jniLibs/arm64-v8a/
cp build-android-arm/lib/libului_app.so /path/to/your/android/app/src/main/jniLibs/armeabi-v7a/

# 同时复制着色器文件
cp -r shaders /path/to/your/android/app/src/main/assets/
```

## 在 Android Studio 中集成

### 1. 项目结构

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

请参阅本目录中的 `app/src/main/AndroidManifest.xml` 获取完整示例。

关键要求：
- 声明 native activity
- 请求 OpenGL ES 3.0
- 设置屏幕方向（可选）

### 3. 加载库

该库使用 `android_native_app_glue` 并导出 `android_main` 函数。配置为 NativeActivity 时，Android 会自动加载并运行它。

不需要 Java/Kotlin 代码 - 整个应用程序在原生 C++ 代码中运行。

### 4. Gradle 配置

请参阅 `app/build.gradle` 获取完整配置。

重要设置：
```gradle
android {
    defaultConfig {
        minSdkVersion 21  // Android 5.0（用于 OpenGL ES 3.0）
        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a'
        }
    }
}
```

## 替代方案：在 Android Studio 中使用 CMake

与预先构建库不同，您可以配置 Android Studio 直接构建它：

### 1. 添加到 app/build.gradle：

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
            path "../../CMakeLists.txt"  // ULUI CMakeLists.txt 的路径
            version "3.20.0"
        }
    }
}
```

### 2. Android Studio 将：
- 自动构建原生库
- 将其打包到 APK 中
- 处理多架构构建

## 运行应用程序

1. 连接 Android 设备或启动模拟器
2. 安装 APK：
```bash
./gradlew installDebug
```

3. 或直接从 Android Studio 运行（Run → Run 'app'）

## 调试

### 查看日志

```bash
adb logcat | grep ULUI
```

### 调试原生代码

1. 在 Android Studio 中：Run → Debug 'app'
2. 在 C++ 代码中设置断点
3. 使用 LLDB 调试器

### 常见问题

**找不到库：**
- 验证 .so 文件在正确的 jniLibs 目录中
- 检查 ABI 是否与设备架构匹配
- 确保 minSdkVersion 为 21 或更高

**着色器加载失败：**
- 验证着色器在 assets/shaders/ 中
- 检查 triangle_app.cpp 中的资源加载代码

**EGL 初始化失败：**
- 验证设备支持 OpenGL ES 3.0
- 检查 AndroidManifest.xml 是否有正确的 OpenGL ES 声明

## 性能提示

1. **使用 Release 模式构建**以获得更好的性能
2. **对 64 位设备使用 arm64-v8a**（更快）
3. **在 AndroidManifest.xml 中启用硬件加速**（示例中已包含）

## 示例项目

请参阅 `app/` 目录获取加载和使用 ULUI 原生库的完整示例 Android 项目。

## 其他资源

- [Android NDK 文档](https://developer.android.com/ndk)
- [NativeActivity 指南](https://developer.android.com/ndk/guides/concepts#naa)
- [Android 上的 OpenGL ES](https://developer.android.com/guide/topics/graphics/opengl)
