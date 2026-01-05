# Java风格I/O流文档

## 概述

ULUI提供了完整的Java风格I/O流层次结构，用于字节级和类型化数据I/O操作。这个熟悉的API使Java开发者能够轻松在C++中使用文件和数据流。

## 流层次结构

```
InputStream（抽象）
├── FileInputStream（从资产或外部文件读取）
└── [包装] DataInputStream（类型化数据读取）

OutputStream（抽象）
├── FileOutputStream（写入外部文件）
└── [包装] DataOutputStream（类型化数据写入）
```

## 基础流类

### InputStream

用于读取字节的抽象基类。

```cpp
class InputStream {
    virtual int Read() = 0;  // 读取单个字节
    virtual int Read(uint8_t* buffer, size_t offset, size_t length) = 0;
    virtual int64_t Skip(int64_t n);
    virtual int Available();
    virtual void Close() = 0;
    virtual bool MarkSupported() const;
    virtual void Mark(int readlimit);
    virtual void Reset();
};
```

### OutputStream

用于写入字节的抽象基类。

```cpp
class OutputStream {
    virtual void Write(int b) = 0;  // 写入单个字节
    virtual void Write(const uint8_t* buffer, size_t offset, size_t length) = 0;
    virtual void Flush() = 0;
    virtual void Close() = 0;
};
```

## 文件流类

### FileInputStream

从文件读取字节（支持资产和外部文件）。

```cpp
#include "FileInputStream.h"

// 从资产文件读取
FileInputStream fis(Path("shaders/vertex.glsl"), true);
if (fis.IsOpen()) {
    int b = fis.Read();  // 读取单个字节
    
    uint8_t buffer[1024];
    int bytesRead = fis.Read(buffer, sizeof(buffer));
    
    int available = fis.Available();
    fis.Close();
}
```

**构造函数：**
- `FileInputStream(const Path& path, bool preferAsset = true)`
  - `preferAsset = true`：优先尝试资产，回退到外部文件
  - `preferAsset = false`：优先尝试外部文件，回退到资产

### FileOutputStream

向外部文件写入字节。

```cpp
#include "FileOutputStream.h"

// 创建新文件（如存在则截断）
FileOutputStream fos(Path("output.dat"), false);
if (fos.IsOpen()) {
    fos.Write('A');  // 写入单个字节
    
    uint8_t data[] = {1, 2, 3, 4, 5};
    fos.Write(data, sizeof(data));
    
    fos.Flush();
    fos.Close();
}

// 追加到现有文件
FileOutputStream fos2(Path("log.txt"), true);
```

**构造函数：**
- `FileOutputStream(const Path& path, bool append = false)`
  - `append = false`：创建新文件（如存在则截断）
  - `append = true`：追加到现有文件

## 数据流类

### DataInputStream

以本机字节序读取基本数据类型。

```cpp
#include "DataInputStream.h"
#include "FileInputStream.h"

auto fis = std::make_unique<FileInputStream>(Path("data.bin"), false);
DataInputStream dis(std::move(fis));

// 读取基本类型
bool boolVal = dis.ReadBoolean();
int8_t byteVal = dis.ReadByte();
uint8_t ubyteVal = dis.ReadUnsignedByte();
int16_t shortVal = dis.ReadShort();
uint16_t ushortVal = dis.ReadUnsignedShort();
int32_t intVal = dis.ReadInt();
int64_t longVal = dis.ReadLong();
float floatVal = dis.ReadFloat();
double doubleVal = dis.ReadDouble();
std::string strVal = dis.ReadUTF();

dis.Close();
```

**方法：**
- `ReadBoolean()` - 1字节
- `ReadByte()` - 有符号8位
- `ReadUnsignedByte()` - 无符号8位
- `ReadShort()` - 有符号16位（本机字节序）
- `ReadUnsignedShort()` - 无符号16位（本机字节序）
- `ReadInt()` - 有符号32位（本机字节序）
- `ReadLong()` - 有符号64位（本机字节序）
- `ReadFloat()` - 32位IEEE 754（本机字节序）
- `ReadDouble()` - 64位IEEE 754（本机字节序）
- `ReadUTF()` - UTF-8字符串（2字节长度 + UTF-8字节）
- `ReadFully(buffer, length)` - 读取确切数量的字节

### DataOutputStream

以本机字节序写入基本数据类型。

```cpp
#include "DataOutputStream.h"
#include "FileOutputStream.h"

auto fos = std::make_unique<FileOutputStream>(Path("data.bin"), false);
DataOutputStream dos(std::move(fos));

// 写入基本类型
dos.WriteBoolean(true);
dos.WriteByte(127);
dos.WriteShort(32000);
dos.WriteInt(1234567890);
dos.WriteLong(9876543210LL);
dos.WriteFloat(3.14159f);
dos.WriteDouble(2.718281828459);
dos.WriteUTF("你好，世界！");

size_t bytesWritten = dos.GetBytesWritten();
dos.Flush();
dos.Close();
```

**方法：**
- `WriteBoolean(bool)` - 1字节
- `WriteByte(int8_t)` - 1字节
- `WriteShort(int16_t)` - 2字节（本机字节序）
- `WriteInt(int32_t)` - 4字节（本机字节序）
- `WriteLong(int64_t)` - 8字节（本机字节序）
- `WriteFloat(float)` - 4字节（IEEE 754，本机字节序）
- `WriteDouble(double)` - 8字节（IEEE 754，本机字节序）
- `WriteUTF(const std::string&)` - UTF-8字符串（2字节长度 + UTF-8字节，最大65535字节）
- `GetBytesWritten()` - 已写入的总字节数

## 流组合

用数据流包装文件流以实现类型化I/O：

```cpp
// 写入结构化数据
{
    auto fos = std::make_unique<FileOutputStream>(Path("player.dat"), false);
    DataOutputStream dos(std::move(fos));
    
    dos.WriteUTF("玩家一");
    dos.WriteInt(25);         // 等级
    dos.WriteInt(9850);       // 经验
    dos.WriteFloat(87.5f);    // 生命值
    dos.WriteBoolean(true);   // 有护盾
    
    dos.Close();
}

// 读取结构化数据
{
    auto fis = std::make_unique<FileInputStream>(Path("player.dat"), false);
    DataInputStream dis(std::move(fis));
    
    std::string name = dis.ReadUTF();
    int level = dis.ReadInt();
    int experience = dis.ReadInt();
    float health = dis.ReadFloat();
    bool hasShield = dis.ReadBoolean();
    
    dis.Close();
}
```

## 高级示例

### 缓冲读取

```cpp
FileInputStream fis(Path("large_file.dat"), false);
uint8_t buffer[4096];

while (true) {
    int bytesRead = fis.Read(buffer, sizeof(buffer));
    if (bytesRead <= 0) break;
    
    // 处理缓冲区...
}

fis.Close();
```

### 二进制协议

```cpp
// 写入二进制协议
auto fos = std::make_unique<FileOutputStream>(Path("protocol.bin"), false);
DataOutputStream dos(std::move(fos));

dos.WriteByte(0x01);  // 版本
dos.WriteShort(100);  // 消息ID
dos.WriteInt(1024);   // 负载长度
// 写入负载...

dos.Close();

// 读取二进制协议
auto fis = std::make_unique<FileInputStream>(Path("protocol.bin"), false);
DataInputStream dis(std::move(fis));

uint8_t version = dis.ReadUnsignedByte();
uint16_t messageId = dis.ReadUnsignedShort();
uint32_t payloadLength = dis.ReadInt();
// 读取负载...

dis.Close();
```

## 错误处理

数据流在错误时抛出异常：

```cpp
try {
    auto fis = std::make_unique<FileInputStream>(Path("data.bin"), false);
    DataInputStream dis(std::move(fis));
    
    int value = dis.ReadInt();
    
} catch (const std::runtime_error& e) {
    std::cerr << "流错误：" << e.what() << std::endl;
} catch (const std::invalid_argument& e) {
    std::cerr << "无效参数：" << e.what() << std::endl;
}
```

## 字节序

所有多字节数据类型使用**本机**字节序（平台特定的字节序）：
- x86/x64架构通常是小端字节序
- 使用平台的本机字节序以提高效率
- 数据文件是平台特定的（不可跨平台移植）

## 与File类的比较

| 特性 | File类 | Stream类 |
|------|--------|----------|
| API风格 | C风格（fopen/fread） | Java风格OOP |
| 类型安全 | 原始字节 | 类型化数据 |
| 组合 | 否 | 是（包装流） |
| 继承 | 否 | 是（扩展基类） |
| 错误处理 | 返回码 | 异常 |
| 使用场景 | 简单文件I/O | 复杂协议 |

## 最佳实践

1. **使用智能指针**进行流组合：
   ```cpp
   auto fis = std::make_unique<FileInputStream>(path, false);
   DataInputStream dis(std::move(fis));
   ```

2. **始终关闭流**（或使用RAII）：
   ```cpp
   {
       FileInputStream fis(path);
       // 使用流...
   } // 自动关闭
   ```

3. **处理异常**（数据流）：
   ```cpp
   try {
       value = dis.ReadInt();
   } catch (const std::exception& e) {
       // 处理错误
   }
   ```

4. **检查IsOpen()**（文件流）：
   ```cpp
   FileInputStream fis(path);
   if (!fis.IsOpen()) {
       // 处理错误
       return;
   }
   ```

5. **关键写入后刷新**：
   ```cpp
   dos.WriteInt(importantData);
   dos.Flush();  // 确保写入磁盘
   ```

## 平台支持

- **Windows**：完全支持，使用_fseeki64/_ftelli64处理大文件
- **Linux/macOS**：完全支持，使用fseeko/ftello处理大文件
- **Android**：通过AAssetManager加载资产 + 外部文件支持
- **iOS**：Bundle资源加载 + 外部文件支持

## 另见

- [File类文档](FILE_CLASS_CN.md)
- [文件系统文档](FILE_SYSTEM_CN.md)
- [Path类文档](PATH_CN.md)
- [流示例](../examples/streams_example.cpp)
