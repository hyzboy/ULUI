# Android 集成指南

本目录包含将 ULUI 原生代码集成到 Android 应用程序的示例和说明。

ULUI 支持 Android 的**两种部署模式**：
1. **可执行程序** - 可在 Android 上直接运行的独立二进制文件
2. **共享库** - 基于 NativeActivity 的 APK，用于应用商店分发

**注意：** 仅支持 arm64-v8a（64位ARM）。不支持 armv7（32位ARM）。

## 快速构建

### 前置要求

- Android NDK r21 或更高版本
- CMake 3.20 或更高版本
- Android SDK（用于构建 APK）

### 使用构建脚本

```bash
# 设置 Android NDK 路径
export ANDROID_NDK=/path/to/android-ndk

# 同时构建可执行程序和共享库（默认）
./build-android.sh

# 仅构建可执行程序
./build-android.sh Release executable

# 仅构建共享库
./build-android.sh Release shared

# 同时构建两者（明确指定）
./build-android.sh Release both
```

## 构建模式 1：Android 可执行程序

可执行程序可以推送到 Android 设备并直接运行，无需打包成 APK。

### 手动构建

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

### 部署

```bash
# 推送可执行程序到设备
adb push android-executable/arm64-v8a/ului_app /data/local/tmp/

# 推送着色器文件
adb push android-executable/shaders /data/local/tmp/

# 在设备上运行（某些设备可能需要 root 权限）
adb shell /data/local/tmp/ului_app
```

## 构建模式 2：共享库（NativeActivity APK）

共享库被打包成 APK 并通过应用商店分发。

### 手动构建

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

### 复制到 Android 项目

```bash
# 复制共享库
cp build-android-so-arm64/lib/libului_app.so android/app/src/main/jniLibs/arm64-v8a/

# 复制着色器文件
cp -r shaders android/app/src/main/assets/

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
