# Android 集成示例

本目录包含 6 个不同的示例项目，演示了将 ULUI 共享库 (libului_app.so) 集成到 Android 应用程序的各种方法。

**所有示例使用相同的共享库** - 仅集成方式不同。

## 支持的架构

- **仅支持 arm64-v8a**（64位ARM）
- 不支持 armv7

## 示例项目

### 1. NativeActivity - OpenGL 由 .so 初始化
**目录**: `example-nativeactivity-so-gl/`

- 使用 Android NativeActivity
- OpenGL ES 上下文由原生 .so 库创建和管理
- 不需要 Java/Kotlin 代码
- 最适合纯原生应用

### 2. NativeActivity - OpenGL 由应用初始化
**目录**: `example-nativeactivity-app-gl/`

- 使用 Android NativeActivity
- OpenGL ES 上下文由 Android 应用框架创建
- .so 库使用现有上下文
- 适用于需要应用级 GL 管理的场景

### 3. GameActivity - OpenGL 由 .so 初始化
**目录**: `example-gameactivity-so-gl/`

- 使用 Android GameActivity（NativeActivity 的现代替代品）
- OpenGL ES 上下文由原生 .so 库创建和管理
- 比 NativeActivity 有更好的输入处理
- 推荐用于新游戏项目

### 4. GameActivity - OpenGL 由应用初始化
**目录**: `example-gameactivity-app-gl/`

- 使用 Android GameActivity
- OpenGL ES 上下文由 Android 应用框架创建
- .so 库使用现有上下文
- 对应用生命周期有最佳控制

### 5. Java/Kotlin 应用 - OpenGL 由 .so 初始化
**目录**: `example-java-so-gl/`

- 使用 GLSurfaceView 和 Java/Kotlin Activity
- OpenGL ES 上下文由原生 .so 库创建
- Java 层提供 UI 和应用逻辑
- 原生层处理所有渲染

### 6. Java/Kotlin 应用 - OpenGL 由应用初始化
**目录**: `example-java-app-gl/`

- 使用 GLSurfaceView 和 Java/Kotlin Activity
- OpenGL ES 上下文由 GLSurfaceView 创建
- 原生库仅用于渲染调用
- 混合应用最常见的集成模式

## 快速开始

### 1. 构建共享库

```bash
# 设置 Android NDK 路径
export ANDROID_NDK=/path/to/android-ndk

# 构建 libului_app.so
./build-android.sh

# 脚本会自动将 .so 复制到所有 6 个示例项目
```

### 2. 构建并运行示例

```bash
# 选择一个示例项目
cd android/example-nativeactivity-so-gl

# 构建并安装
./gradlew installDebug

# 或仅构建 APK
./gradlew assembleDebug
```

## 项目结构

每个示例项目都有以下结构：

```
example-xxx/
├── app/
│   ├── build.gradle
│   └── src/
│       └── main/
│           ├── AndroidManifest.xml
│           ├── java/          (仅 Java/Kotlin 示例)
│           ├── jniLibs/
│           │   └── arm64-v8a/
│           │       └── libului_app.so
│           └── assets/
│               └── shaders/
├── build.gradle
└── settings.gradle
```

## 对比矩阵

| 示例 | Activity 类型 | GL 初始化 | Java 代码 | 使用场景 |
|------|--------------|----------|-----------|---------|
| 1 | NativeActivity | .so | 无 | 纯原生应用 |
| 2 | NativeActivity | App | 最少 | 原生应用带 GL 控制 |
| 3 | GameActivity | .so | 无 | 现代原生游戏 |
| 4 | GameActivity | App | 最少 | 现代应用带 GL 控制 |
| 5 | GLSurfaceView | .so | 中等 | 混合应用，原生渲染 |
| 6 | GLSurfaceView | App | 中等 | 标准 Android GL 应用 |

## 技术细节

### 共享库接口

`libului_app.so` 库导出：

- `android_main(android_app*)` - 用于 NativeActivity/GameActivity（.so GL 初始化）
- `ANativeActivity_onCreate()` - 标准原生 activity 入口点
- JNI 函数用于 Java 集成（示例 5 和 6）

### OpenGL ES 版本

所有示例都使用通过 ANGLE 或系统 OpenGL ES 的 **OpenGL ES 3.0**。

### 最低要求

- Android API Level 21 (Android 5.0)
- OpenGL ES 3.0 支持
- 64位 ARM 设备 (arm64-v8a)

## 选择合适的示例

**对于新游戏项目：**
- 从 `example-gameactivity-so-gl`（示例 3）开始

**对于最大控制：**
- 使用 `example-java-app-gl`（示例 6）配合 GLSurfaceView

**对于纯原生 C++ 应用：**
- 使用 `example-nativeactivity-so-gl`（示例 1）

**对于现有 Android 应用集成：**
- 使用 `example-java-so-gl`（示例 5）或 `example-java-app-gl`（示例 6）

## 发布构建

```bash
# 构建 release APK
cd android/example-xxx
./gradlew assembleRelease

# APK 位于 app/build/outputs/apk/release/
```

## 故障排除

### 找不到库
- 确保 `./build-android.sh` 成功完成
- 检查 `app/src/main/jniLibs/arm64-v8a/libului_app.so` 是否存在

### OpenGL 错误
- 验证设备支持 OpenGL ES 3.0
- 检查 logcat 获取详细错误消息：`adb logcat | grep ULUI`

### 着色器加载失败
- 验证着色器在 `app/src/main/assets/shaders/` 中
- 检查代码中的资源路径

## 其他资源

- [Android NDK 文档](https://developer.android.com/ndk)
- [NativeActivity 指南](https://developer.android.com/ndk/guides/concepts#naa)
- [GameActivity 文档](https://developer.android.com/games/agdk/game-activity)
- [Android 上的 OpenGL ES](https://developer.android.com/guide/topics/graphics/opengl)
