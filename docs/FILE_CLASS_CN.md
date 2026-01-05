# File 类文档

## 概述

`File` 类为 ULUI 提供统一的文件访问接口，支持：
- **内部资产**（只读）：应用程序打包的资源
- **外部文件**（读写）：文件系统上的文件

它提供标准的文件 I/O 操作（read、write、seek、tell）和现代 C++ 接口。

## 特性

- **统一 API**：资产和外部文件访问的单一接口
- **基于路径**：使用 `Path` 类进行跨平台路径处理
- **标准操作**：read、write、seek、tell、flush
- **元数据查询**：canRead、canWrite、GetLength、IsEof
- **移动语义**：高效的文件句柄管理
- **二进制和文本支持**：读写二进制数据和文本

## 基本用法

### 包含头文件

```cpp
#include "File.h"
using namespace ului;
```

### 从资产读取

```cpp
// 初始化文件系统（应用程序启动时一次）
FileSystem::Initialize();

// 从资产读取着色器（自动优先尝试资产）
File shaderFile(Path("shaders/vertex.glsl"));
if (shaderFile.IsOpen()) {
    std::string shaderCode = shaderFile.ReadAllText();
    // 使用着色器代码...
}
```

### 写入外部文件

```cpp
// 打开文件进行写入
File outputFile(Path("save.dat"), File::OpenMode::Write, false);
if (outputFile.IsOpen()) {
    std::string data = "玩家进度：第 5 关";
    outputFile.Write(data);
}
```

### 从外部文件读取

```cpp
// 打开外部文件进行读取
File inputFile(Path("config.ini"), File::OpenMode::Read, false);
if (inputFile.IsOpen()) {
    std::string config = inputFile.ReadAllText();
    // 解析配置...
}
```

## 文件打开模式

`File::OpenMode` 枚举指定文件的打开方式：

| 模式 | 描述 | 资产 | 外部 |
|------|------|------|------|
| `Read` | 仅读取 | ✓ | ✓ |
| `Write` | 写入（如果存在则截断） | ✗ | ✓ |
| `ReadWrite` | 读写 | ✗ | ✓ |
| `Append` | 写入，追加到末尾 | ✗ | ✓ |

## 核心操作

### 打开文件

```cpp
// 方法 1：构造函数自动打开
File file(Path("data.txt"), File::OpenMode::Read);

// 方法 2：默认构造函数 + 显式 Open()
File file;
file.Open(Path("data.txt"), File::OpenMode::Read);

// 方法 3：优先外部文件而非资产
File file(Path("config.ini"), File::OpenMode::Read, false);
```

第三个参数 `preferAsset`（默认：`true`）控制首先尝试哪个源：
- `true`：首先尝试资产，回退到外部文件
- `false`：首先尝试外部文件，回退到资产

### 读取数据

```cpp
// 读取所有内容为文本
std::string text = file.ReadAllText();

// 读取所有内容为二进制
std::vector<uint8_t> data = file.ReadAll();

// 读取特定字节数
std::vector<uint8_t> chunk = file.Read(1024);

// 读取到预分配的缓冲区
char buffer[256];
size_t bytesRead = file.Read(buffer, sizeof(buffer));
```

### 写入数据

```cpp
// 写入字符串
size_t written = file.Write("你好，世界！");

// 写入向量
std::vector<uint8_t> data = {0x01, 0x02, 0x03};
file.Write(data);

// 写入原始缓冲区
uint8_t buffer[100];
file.Write(buffer, sizeof(buffer));

// 确保写入刷新到磁盘
file.Flush();
```

### 定位和位置

```cpp
// 定位到开始
file.Seek(0, File::SeekOrigin::Begin);

// 从当前位置向前定位
file.Seek(100, File::SeekOrigin::Current);

// 从末尾向后定位
file.Seek(-50, File::SeekOrigin::End);

// 获取当前位置
int64_t pos = file.Tell();

// 获取文件长度
int64_t length = file.GetLength();

// 检查是否在文件末尾
if (file.IsEof()) {
    // 到达文件末尾
}
```

### 关闭文件

```cpp
// 显式关闭
file.Close();

// File 对象销毁时自动关闭
{
    File file("temp.txt", File::OpenMode::Write);
    file.Write("data");
} // 文件在此处自动关闭
```

## 文件属性和查询

```cpp
// 检查文件是否打开
if (file.IsOpen()) {
    // 文件操作...
}

// 检查功能
if (file.CanRead()) {
    // 可以从文件读取
}
if (file.CanWrite()) {
    // 可以写入文件
}

// 检查文件类型
if (file.IsAsset()) {
    // 文件来自资产包（只读）
}
if (file.IsExternal()) {
    // 文件来自外部文件系统
}

// 获取文件路径
const Path& path = file.GetPath();
std::cout << "文件：" << path.ToString() << std::endl;

// 获取文件大小
int64_t size = file.GetLength();
std::cout << "大小：" << size << " 字节" << std::endl;
```

## 高级示例

### 读取配置文件

```cpp
File configFile(Path("settings.json"), File::OpenMode::Read, false);
if (configFile.IsOpen()) {
    std::string json = configFile.ReadAllText();
    // 使用您喜欢的库解析 JSON
    // auto config = parseJson(json);
}
```

### 二进制文件处理

```cpp
// 分块读取二进制文件
File binFile(Path("data.bin"), File::OpenMode::Read, false);
if (binFile.IsOpen()) {
    const size_t CHUNK_SIZE = 4096;
    std::vector<uint8_t> chunk;
    
    while (!binFile.IsEof()) {
        chunk = binFile.Read(CHUNK_SIZE);
        if (!chunk.empty()) {
            // 处理块...
        }
    }
}
```

### 追加到日志文件

```cpp
File logFile(Path("app.log"), File::OpenMode::Append, false);
if (logFile.IsOpen()) {
    std::string logEntry = "[2024-01-01 12:00:00] 应用程序已启动\n";
    logFile.Write(logEntry);
    logFile.Flush(); // 确保立即写入
}
```

### 随机访问文件

```cpp
File dataFile(Path("records.dat"), File::OpenMode::ReadWrite, false);
if (dataFile.IsOpen()) {
    // 读取特定位置的记录
    const size_t RECORD_SIZE = 128;
    int recordIndex = 10;
    
    dataFile.Seek(recordIndex * RECORD_SIZE, File::SeekOrigin::Begin);
    auto record = dataFile.Read(RECORD_SIZE);
    
    // 修改记录
    // ... 修改数据 ...
    
    // 写回
    dataFile.Seek(recordIndex * RECORD_SIZE, File::SeekOrigin::Begin);
    dataFile.Write(record);
}
```

### 从资产加载着色器

```cpp
// 自动从资产目录加载
File vertShader(Path("shaders/triangle.vert"));
File fragShader(Path("shaders/triangle.frag"));

if (vertShader.IsOpen() && fragShader.IsOpen()) {
    std::string vertCode = vertShader.ReadAllText();
    std::string fragCode = fragShader.ReadAllText();
    
    // 编译着色器...
    // compileShader(vertCode, fragCode);
}
```

## 移动语义

`File` 类支持移动语义以实现高效的资源管理：

```cpp
// 移动构造函数
File createFile() {
    File file("data.txt", File::OpenMode::Write);
    file.Write("data");
    return file; // 文件被移动，而不是复制
}

File myFile = createFile(); // 移动，没有文件句柄重复

// 移动赋值
File file1("a.txt", File::OpenMode::Read);
File file2("b.txt", File::OpenMode::Read);
file2 = std::move(file1); // file1 现在无效，file2 持有 file1 的句柄
```

## 错误处理

```cpp
File file(Path("nonexistent.txt"), File::OpenMode::Read);
if (!file.IsOpen()) {
    // 处理错误：文件不存在或无法打开
    std::cerr << "无法打开文件" << std::endl;
    return;
}

// 继续文件操作...
size_t bytesRead = file.Read(buffer, size);
if (bytesRead == 0) {
    // 文件末尾或读取错误
    if (file.IsEof()) {
        std::cout << "到达文件末尾" << std::endl;
    } else {
        std::cerr << "读取错误" << std::endl;
    }
}
```

## 最佳实践

1. **始终检查文件是否打开**：在执行操作前使用 `IsOpen()`
2. **使用 RAII**：让析构函数处理关闭以实现异常安全
3. **刷新重要写入**：在关键写入后调用 `Flush()`
4. **优先使用 Path 对象**：使用 `Path` 类而不是原始字符串
5. **选择适当的模式**：为您的用例选择正确的 `OpenMode`
6. **处理读取失败**：检查读取操作的返回值
7. **使用移动语义**：利用移动操作提高效率

## 平台特定行为

### 资产文件
- **Android**：通过 `AAssetManager` 从 APK 资产加载
- **iOS**：从应用程序包资源加载
- **桌面**：从可执行文件相对的 `assets/` 目录加载

### 外部文件
- 所有平台使用标准 C `FILE*` 操作
- 使用平台的文件系统解析路径
- 使用 `FileSystem::GetExternalDataPath()` 获取平台适当的数据目录

## 相关类

- **Path**：跨平台路径处理
- **FileSystem**：文件操作的静态实用函数
  - `ReadAssetText()` / `ReadAssetBinary()`
  - `ReadExternalText()` / `ReadExternalBinary()`
  - `WriteExternalText()` / `WriteExternalBinary()`
  - 各种目录路径获取器

## 从 FileSystem 迁移

如果您正在使用静态 `FileSystem` 方法，可以迁移到 `File` 类：

```cpp
// 旧方法
std::string shader = FileSystem::ReadAssetText("shader.vert");
std::string config = FileSystem::ReadExternalText("config.ini");

// 使用 File 类的新方法
File shaderFile("shader.vert");
std::string shader = shaderFile.ReadAllText();

File configFile("config.ini", File::OpenMode::Read, false);
std::string config = configFile.ReadAllText();
```

`File` 类提供更多控制和更好的资源管理，特别适用于：
- 顺序读取/写入
- 使用 seek 操作进行随机访问
- 分块流式传输大文件
- 增量文件处理

## 另见

- [文件系统文档](FILE_SYSTEM_CN.md)
- [Path 类文档](PATH_CN.md)
- [文件示例](../examples/file_example.cpp)
