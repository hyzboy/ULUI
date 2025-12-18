# 文件系统抽象层

## 概述

ULUI 文件系统提供跨平台的文件 I/O 操作抽象，支持两种文件访问类型：

1. **内部资产（只读）**：平台特定的资产包
2. **外部文件（读写）**：标准文件系统访问

## 架构

### 内部资产

内部资产是与应用程序打包在一起的只读文件：

| 平台 | 位置 | 访问方法 |
|------|------|----------|
| **Android** | APK `assets/` 目录 | AAssetManager API |
| **iOS** | 应用包资源 | NSBundle API |
| **Windows/Linux/macOS** | 可执行文件旁的 `assets/` 子目录 | 标准文件 I/O |

### 外部文件

外部文件使用标准文件系统访问，具有读写功能：

| 平台 | 默认位置 | 访问方法 |
|------|----------|----------|
| **Android** | 外部存储 (`/sdcard/`) | 标准文件 I/O |
| **iOS** | Documents 目录 | 标准文件 I/O |
| **Windows** | 我的文档 | 标准文件 I/O |
| **Linux/macOS** | 主目录 | 标准文件 I/O |

## 使用方法

### 初始化

```cpp
#include "file_system.h"
using namespace ului;

// 桌面平台（自动路径检测）
FileSystem::Initialize();

// 或指定自定义资产路径
FileSystem::Initialize("my_assets/");

// Android（需要 AAssetManager）
#ifdef __ANDROID__
FileSystem::Initialize(nullptr);
FileSystem::SetAndroidAssetManager(state->activity->assetManager);
#endif

// iOS（自动）
FileSystem::Initialize();
```

### 读取内部资产

```cpp
// 从资产读取着色器
std::string vertexShader = FileSystem::ReadAssetText("shaders/triangle.vert");

// 读取二进制资产（例如纹理）
std::vector<uint8_t> textureData = FileSystem::ReadAssetBinary("textures/logo.png");

// 检查资产是否存在
if (FileSystem::AssetExists("config.json")) {
    std::string config = FileSystem::ReadAssetText("config.json");
}
```

### 读取外部文件

```cpp
// 读取配置文件
std::string config = FileSystem::ReadExternalText("/path/to/config.txt");

// 读取二进制数据
std::vector<uint8_t> data = FileSystem::ReadExternalBinary("/path/to/data.bin");

// 检查文件是否存在
if (FileSystem::ExternalFileExists("/path/to/save.dat")) {
    auto saveData = FileSystem::ReadExternalBinary("/path/to/save.dat");
}
```

### 写入外部文件

```cpp
// 写入文本文件
std::string gameState = "level=5\nscore=1000";
FileSystem::WriteExternalText("/path/to/save.txt", gameState);

// 写入二进制文件
std::vector<uint8_t> saveData = {0x01, 0x02, 0x03};
FileSystem::WriteExternalBinary("/path/to/save.dat", saveData);

// 删除文件
FileSystem::DeleteExternalFile("/path/to/old_save.dat");
```

### 平台特定路径

```cpp
// 获取资产目录路径
std::string assetPath = FileSystem::GetAssetPath();
std::cout << "资产位置: " << assetPath << std::endl;

// 获取可写数据目录
std::string dataPath = FileSystem::GetExternalDataPath();
std::cout << "可写位置: " << dataPath << std::endl;

// 用于存档文件
std::string savePath = dataPath + "savegame.dat";
FileSystem::WriteExternalBinary(savePath.c_str(), saveData);
```

### 清理

```cpp
// 应用程序退出时关闭
FileSystem::Shutdown();
```

## 平台特定详情

### Android

**资产访问：**
```cpp
// 在 android_main 中
void android_main(android_app* state) {
    FileSystem::Initialize(nullptr);
    FileSystem::SetAndroidAssetManager(state->activity->assetManager);
    
    // 现在可以访问 APK 资产
    std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");
}
```

**外部存储：**
- 需要在 AndroidManifest.xml 中申请 `WRITE_EXTERNAL_STORAGE` 权限
- 使用 `GetExternalDataPath()` 获取应用专用存储
- 外部存储中的文件在应用卸载后保留（用户数据）

**资产打包：**
- 将文件放在 `android/app/src/main/assets/`
- 构建脚本自动复制到所有示例项目
- 资产在 APK 中被压缩

### iOS

**资产访问：**
```cpp
// 自动 - 使用 NSBundle
FileSystem::Initialize();

// 访问包资源
std::string config = FileSystem::ReadAssetText("config.plist");
```

**外部存储：**
- 使用 Documents 目录（iTunes/iCloud 备份）
- Documents 目录无需特殊权限
- 使用 `GetExternalDataPath()` 保存文件

**资产打包：**
- 将文件添加到 Xcode 项目
- 标记为"复制包资源"
- 资产在应用包中

### 桌面（Windows/Linux/macOS）

**资产访问：**
```cpp
// 使用可执行文件的相对路径
FileSystem::Initialize("assets/");  // 默认

// 资产在：executable_dir/assets/
std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");
```

**外部存储：**
- 完整文件系统访问
- 无需特殊权限
- 使用绝对或相对路径

**资产组织：**
```
my_app/
├── my_app.exe          (或 Linux/macOS 上的 my_app)
└── assets/
    ├── shaders/
    │   ├── triangle.vert
    │   └── triangle.frag
    ├── textures/
    └── config.json
```

## 错误处理

所有读写操作在失败时返回空结果，并将错误消息打印到 `std::cerr`：

```cpp
std::string data = FileSystem::ReadAssetText("missing.txt");
if (data.empty()) {
    std::cerr << "加载资产失败" << std::endl;
    // 处理错误
}

bool success = FileSystem::WriteExternalText("save.txt", data);
if (!success) {
    std::cerr << "写入文件失败" << std::endl;
    // 处理错误
}
```

## 最佳实践

### 1. 使用内部资产存放只读数据

✅ **正确：**
```cpp
// 从资产读取着色器、纹理、配置
std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");
```

❌ **避免：**
```cpp
// 不要尝试写入资产
FileSystem::WriteExternalText("assets/shader.vert", modifiedShader); // 错误！
```

### 2. 使用外部文件存放用户数据

✅ **正确：**
```cpp
// 保存游戏状态到可写位置
std::string savePath = FileSystem::GetExternalDataPath() + "save.dat";
FileSystem::WriteExternalBinary(savePath.c_str(), saveData);
```

### 3. 读取前检查文件是否存在

```cpp
if (FileSystem::ExternalFileExists(savePath.c_str())) {
    auto data = FileSystem::ReadExternalBinary(savePath.c_str());
} else {
    // 创建默认存档
}
```

### 4. 尽早初始化

```cpp
int main() {
    FileSystem::Initialize();  // 在访问任何文件之前
    
    // ... 其余初始化
}
```

## 示例：加载着色器

```cpp
#include "file_system.h"

bool loadShaders() {
    // 从内部资产读取（跨平台）
    std::string vertexSource = FileSystem::ReadAssetText("shaders/triangle.vert");
    std::string fragmentSource = FileSystem::ReadAssetText("shaders/triangle.frag");
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "加载着色器失败" << std::endl;
        return false;
    }
    
    // 编译着色器...
    return true;
}
```

## 示例：保存/加载游戏状态

```cpp
#include "file_system.h"
#include <nlohmann/json.hpp>  // JSON 库示例

void saveGameState(const GameState& state) {
    // 转换状态为 JSON
    nlohmann::json j;
    j["level"] = state.level;
    j["score"] = state.score;
    
    std::string json = j.dump();
    
    // 写入外部存储
    std::string savePath = FileSystem::GetExternalDataPath() + "save.json";
    if (!FileSystem::WriteExternalText(savePath.c_str(), json)) {
        std::cerr << "保存游戏状态失败" << std::endl;
    }
}

GameState loadGameState() {
    std::string savePath = FileSystem::GetExternalDataPath() + "save.json";
    
    if (!FileSystem::ExternalFileExists(savePath.c_str())) {
        return GameState{};  // 返回默认状态
    }
    
    std::string json = FileSystem::ReadExternalText(savePath.c_str());
    if (json.empty()) {
        return GameState{};
    }
    
    // 解析 JSON
    auto j = nlohmann::json::parse(json);
    GameState state;
    state.level = j["level"];
    state.score = j["score"];
    return state;
}
```

## API 参考

完整 API 文档请参阅 [`include/file_system.h`](../include/file_system.h)。

### 主要函数

- `Initialize(assetPath)` - 初始化文件系统
- `Shutdown()` - 清理资源
- `ReadAssetText(filename)` - 从内部资产读取文本
- `ReadAssetBinary(filename)` - 从内部资产读取二进制
- `ReadExternalText(filepath)` - 从外部文件读取文本
- `ReadExternalBinary(filepath)` - 从外部文件读取二进制
- `WriteExternalText(filepath, content)` - 写入文本到外部文件
- `WriteExternalBinary(filepath, data)` - 写入二进制到外部文件
- `AssetExists(filename)` - 检查内部资产是否存在
- `ExternalFileExists(filepath)` - 检查外部文件是否存在
- `DeleteExternalFile(filepath)` - 删除外部文件
- `GetAssetPath()` - 获取内部资产目录
- `GetExternalDataPath()` - 获取可写数据目录

## 故障排除

### 找不到资产（桌面）

**问题：** `Failed to open asset: shaders/triangle.vert`

**解决方案：**
- 验证可执行文件旁边存在 `assets/` 目录
- 检查 CMake 复制的文件：`build/bin/assets/shaders/`
- 验证初始化：`FileSystem::Initialize("assets/")`

### 找不到资产（Android）

**问题：** `AAssetManager not set`

**解决方案：**
```cpp
FileSystem::SetAndroidAssetManager(state->activity->assetManager);
```

### 无法写入文件

**问题：** 写入操作失败

**解决方案：**
- 使用 `GetExternalDataPath()` 获取可写位置
- 不要尝试写入资产目录
- 检查移动平台上的文件权限

## 线程安全

**非线程安全** - FileSystem 类使用静态成员，应从主线程访问，或在多线程使用时使用互斥锁保护。

```cpp
// 如果从多个线程使用：
static std::mutex g_fileSystemMutex;

void workerThread() {
    std::lock_guard<std::mutex> lock(g_fileSystemMutex);
    auto data = FileSystem::ReadAssetBinary("data.bin");
}
```
