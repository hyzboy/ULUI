# ULUI - 跨平台 OpenGL ES 3.0 项目

一个使用 Google ANGLE 实现的跨平台 OpenGL ES 3.0 项目，展示了一个简单的彩色三角形渲染示例。

## 特性

- **跨平台支持**: Windows, Linux, macOS, iOS, Android
- **现代 CMake** (3.20+) 构建系统
- **C++20** 标准
- **OpenGL ES 3.0** 通过 Google ANGLE 项目实现
- **GLFW** 用于桌面平台的窗口管理
- **ECS (Entity Component System)** 专为 2D 应用设计的架构
- **面向对象基类**，具有自动类名检测和基于TAG的日志记录
- **跨平台文件系统**抽象，支持资产和外部文件
- **完善的日志系统**，支持多种输出目标（控制台、文件、网络、管道、平台特定）
- 简单的彩色三角形渲染示例

## 项目需求

### 所有平台
- CMake 3.20 或更高版本
- 支持 C++20 的编译器
- Google ANGLE 库 (libEGL, libGLESv2)

### 桌面平台 (Windows/Linux/macOS)
- GLFW (通过 CMake FetchContent 自动下载)

### Windows
- Visual Studio 2019 或更高版本 / MinGW-w64
- Windows SDK

### Linux
- GCC 10+ 或 Clang 11+
- X11 开发库
- Mesa 或 ANGLE 库

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

## 构建方法

### 桌面平台 (Windows/Linux/macOS)

```bash
# 创建构建目录
mkdir build
cd build

# 配置
cmake ..

# 构建
cmake --build .

# 运行
./bin/ului_app  # Linux/macOS
# 或
.\bin\ului_app.exe  # Windows
```

### 高级构建选项

```bash
# Release 构建
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

# 指定编译器
cmake -DCMAKE_CXX_COMPILER=clang++ ..

# 禁用 ANGLE (使用原生 OpenGL ES)
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

# 在 Xcode 中打开
open ULUI.xcodeproj
```

### Android

项目编译为 Android **共享库 (.so)**，提供 **6个集成示例**展示不同的使用模式。

**注意：** 仅支持 arm64-v8a（不支持 armv7）。

#### 构建共享库

```bash
# 设置 Android NDK 路径
export ANDROID_NDK=/path/to/android-ndk

# 构建 libului_app.so
./build-android.sh

# 库会自动复制到所有 6 个示例项目
```

#### 6个集成示例

`android/` 目录包含 6 个完整的示例项目：

1. **NativeActivity + SO-GL** - 纯原生，.so 管理 OpenGL
2. **NativeActivity + App-GL** - 纯原生，应用管理 OpenGL
3. **GameActivity + SO-GL** - 现代原生游戏，.so 管理 OpenGL
4. **GameActivity + App-GL** - 现代原生游戏，应用管理 OpenGL
5. **Java/GLSurfaceView + SO-GL** - Java 应用，.so 管理 OpenGL
6. **Java/GLSurfaceView + App-GL** - Java 应用，应用管理 OpenGL

**所有示例使用相同的 .so 库** - 仅集成方式不同。

#### 运行示例

```bash
cd android/example-nativeactivity-so-gl
./gradlew installDebug
```

详细文档请参阅 [android/README.md](android/README.md)。

详细说明请参阅 [android/README.md](android/README.md)。

## 项目结构

```
ULUI/
├── CMakeLists.txt          # 根 CMake 配置
├── README.md               # 英文说明文档
├── README_CN.md            # 中文说明文档（本文件）
├── LICENSE                 # 许可证文件
├── .gitignore             # Git 忽略规则
├── build.sh               # Linux/macOS 构建脚本
├── external/              # 外部依赖
│   ├── CMakeLists.txt     # 外部依赖 CMake 配置
│   └── angle/             # ANGLE 头文件
│       └── include/
│           ├── EGL/       # EGL 头文件
│           └── GLES3/     # OpenGL ES 3.0 头文件
├── include/               # 公共头文件
│   ├── triangle_app.h     # 三角形应用程序头文件
│   └── file_system.h      # 跨平台文件 I/O 抽象
├── src/                   # 源文件
│   ├── CMakeLists.txt     # 源文件 CMake 配置
│   ├── main.cpp           # 主入口点（平台相关）
│   ├── triangle_app.cpp   # 三角形渲染实现
│   └── file_system.cpp    # 文件系统实现
├── shaders/               # GLSL 着色器
│   ├── triangle.vert      # 顶点着色器
│   └── triangle.frag      # 片段着色器
└── docs/                  # 文档
    ├── ANGLE_INTEGRATION.md  # ANGLE 集成指南
    ├── BUILD_GUIDE.md        # 详细构建指南
    ├── FILE_SYSTEM.md        # 文件系统抽象指南
    └── FILE_SYSTEM_CN.md     # 文件系统指南（中文）
```

## ANGLE 集成

本项目使用 Google 的 ANGLE (Almost Native Graphics Layer Engine) 在所有平台上提供 OpenGL ES 3.0 支持。ANGLE 将 OpenGL ES API 调用转换为：
- **Windows**: Direct3D 11
- **macOS**: Metal
- **Linux**: Vulkan 或原生 OpenGL

### 安装 ANGLE

#### Windows
从 ANGLE 发布页面下载预构建的 ANGLE 二进制文件，或从源代码构建。

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get install libegl1-mesa-dev libgles2-mesa-dev

# 或从源代码构建 ANGLE
```

#### macOS
```bash
# 使用 Homebrew（如果可用）
# 或从源代码构建 ANGLE
```

详细的 ANGLE 集成说明请参阅 [docs/ANGLE_INTEGRATION.md](docs/ANGLE_INTEGRATION.md)。

## 文件系统抽象

ULUI 提供跨平台文件系统抽象，处理：

- **内部资产（只读）**：
  - Android: APK 资产
  - iOS: 应用包资源
  - 桌面: 可执行文件旁的 `assets/` 目录

- **外部文件（读写）**：
  - 所有平台的标准文件系统访问
  - 平台特定的用户数据目录

### 快速示例

```cpp
#include "file_system.h"
using namespace ului;

// 初始化
FileSystem::Initialize();

// 从内部资产读取着色器
std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");

// 写入存档文件到外部存储
std::string savePath = FileSystem::GetExternalDataPath() + "save.dat";
FileSystem::WriteExternalBinary(savePath.c_str(), saveData);
```

完整文档请参阅 [docs/FILE_SYSTEM_CN.md](docs/FILE_SYSTEM_CN.md)。

## Object 基类

ULUI 为所有功能类提供一个带有自动日志支持的基类：

```cpp
#include "object.h"

class MyRenderer : public ului::Object {
public:
    MyRenderer() : Object("Renderer") {
        LogI("MyRenderer 已初始化");
    }
    
    void Draw() {
        LogD("正在绘制帧");
        // ... 绘制代码 ...
    }
};

// 日志输出格式：[日志级别][类名][TAG]消息
// [INFO][MyRenderer][Renderer]MyRenderer 已初始化
// [DEBUG][MyRenderer][Renderer]正在绘制帧
```

完整文档请参阅 [docs/OBJECT_CN.md](docs/OBJECT_CN.md)。

## ECS (Entity Component System) 架构

ULUI 提供专为 2D 应用设计的 ECS 架构，帮助组织游戏逻辑：

```cpp
#include "ecs/ECS.h"
using namespace ului::ecs;

// 创建场景
Scene scene;

// 创建实体和组件
Entity player = CreateSpriteEntity(scene, "player.png", 100, 100, 64, 64);

// 访问组件
Transform2D* transform = scene.GetComponent<Transform2D>(player);
transform->Translate(10.0f, 0.0f);

// 添加系统
scene.AddSystem(std::make_unique<RenderSystem>());

// 更新
scene.Update(deltaTime);
```

**核心概念**:
- **Entity (实体)**: 轻量级唯一标识符
- **Component (组件)**: 纯数据容器 (Transform2D, Sprite2D, Renderable2D)
- **System (系统)**: 处理实体的逻辑
- **Scene (场景)**: 管理所有实体、组件和系统

完整文档请参阅:
- [docs/ECS.md](docs/ECS.md) - ECS 架构详细说明
- [docs/ECS_MIGRATION.md](docs/ECS_MIGRATION.md) - 迁移指南
- [examples/ecs_example.cpp](examples/ecs_example.cpp) - 完整示例

## 使用方法

应用程序创建一个窗口并渲染一个彩色三角形：
- **红色**顶点在顶部
- **绿色**顶点在左下角
- **蓝色**顶点在右下角

按 **ESC** 键关闭应用程序（桌面平台）。

## 自定义

### 修改三角形

编辑 `src/triangle_app.cpp` 来更改：
- 顶点位置
- 颜色
- 三角形形状

### 着色器

修改 `shaders/` 目录中的 GLSL 着色器：
- `triangle.vert` - 顶点着色器
- `triangle.frag` - 片段着色器

两个着色器都使用 GLSL ES 3.0 语法 (`#version 300 es`)。

## 技术细节

- **C++ 标准**: C++20
- **CMake 版本**: 3.20+
- **OpenGL ES 版本**: 3.0
- **GLSL 版本**: 3.00 ES
- **窗口系统**: GLFW 3.4 (桌面), 原生 (移动)

## 许可证

详见 LICENSE 文件。

## 贡献

欢迎贡献！请确保：
- 代码遵循 C++20 最佳实践
- CMake 配置保持跨平台兼容性
- 在多个平台上测试更改

## 故障排除

### 找不到 GLFW
GLFW 通过 CMake FetchContent 自动下载。确保在配置期间有互联网连接。

### 找不到 ANGLE 库
确保系统上已安装 ANGLE，或将库放在 CMake 可以找到的位置。

### 着色器编译错误
检查着色器文件是否已复制到构建目录。CMake 应自动处理此操作。

### 启动时黑屏
验证：
1. ANGLE/OpenGL ES 驱动程序已正确安装
2. 您的 GPU 支持 OpenGL ES 3.0 或等效版本
3. 着色器文件可在 `shaders/` 目录中访问

## 详细文档

- [构建指南](docs/BUILD_GUIDE.md) - 所有平台的详细构建说明
- [ANGLE 集成](docs/ANGLE_INTEGRATION.md) - ANGLE 安装和配置指南

## 参考资源

- [ANGLE 项目](https://github.com/google/angle)
- [ANGLE 文档](https://chromium.googlesource.com/angle/angle/+/HEAD/README.md)
- [OpenGL ES 3.0 规范](https://www.khronos.org/opengles/)
- [GLFW 文档](https://www.glfw.org/documentation.html)
- [CMake 文档](https://cmake.org/documentation/)

## 快速开始

```bash
# 克隆仓库
git clone https://github.com/hyzboy/ULUI.git
cd ULUI

# 构建项目
./build.sh  # Linux/macOS

# 运行应用程序
cd build/bin
./ului_app
```

项目已在 Linux 上成功构建和测试。Windows 和 macOS 平台的构建流程相同。
