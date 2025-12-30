# Object 基类文档

## 概述

`Object` 类是 ULUI 框架中所有功能类的基类。它提供自动类名检测和基于TAG的日志记录，采用统一的格式。

## 日志格式

所有从 `Object` 派生的类的日志消息都遵循以下格式：

```
[日志级别][类名][TAG]消息内容
```

**示例：**
```
[DEBUG][TriangleApp][Renderer]着色器编译成功
[INFO][TriangleApp][Renderer]OpenGL Version: OpenGL ES 3.0
[ERROR][NetworkManager][Connection]连接服务器失败：超时
```

## 使用方法

### 基本用法

```cpp
#include "object.h"

class MyClass : public ului::Object {
public:
    MyClass() : Object("MyTag") {
        LogI("MyClass 已构造");
    }
    
    void DoSomething() {
        LogD("开始操作");
        
        int result = PerformCalculation();
        if (result < 0) {
            LogE("计算失败，错误代码：%d", result);
            return;
        }
        
        LogI("计算成功完成：结果=%d", result);
    }
    
private:
    int PerformCalculation() {
        // ... 实现 ...
        return 42;
    }
};
```

### 构造函数

`Object` 构造函数需要一个 TAG 参数：

```cpp
explicit Object(const char* tag);
```

- **tag**: 用户定义的标签字符串，用于标识此对象实例
  - 常见模式："Main"、"Renderer"、"Network"、"Audio" 等
  - 应该描述对象的用途
  - 将出现在此对象的所有日志消息中

### 日志方法

所有日志方法都使用 printf 风格的格式化：

```cpp
void LogV(const char* format, ...) const;  // VERBOSE 级别
void LogD(const char* format, ...) const;  // DEBUG 级别
void LogI(const char* format, ...) const;  // INFO 级别
void LogW(const char* format, ...) const;  // WARNING 级别
void LogE(const char* format, ...) const;  // ERROR 级别
void LogF(const char* format, ...) const;  // FATAL 级别
```

### 访问器方法

```cpp
const char* GetTag() const;        // 返回 TAG 字符串
const char* GetClassName() const;  // 返回类类型名称
```

## 示例

### 示例 1：三角形渲染器

```cpp
class TriangleApp : public ului::Object {
public:
    TriangleApp() : Object("Renderer") {
        LogD("TriangleApp 已构造");
    }
    
    bool initialize(int width, int height) {
        LogI("初始化大小 %dx%d", width, height);
        
        if (width <= 0 || height <= 0) {
            LogE("无效尺寸：%dx%d", width, height);
            return false;
        }
        
        LogI("OpenGL 供应商：%s", glGetString(GL_VENDOR));
        LogI("OpenGL 版本：%s", glGetString(GL_VERSION));
        
        LogD("初始化完成");
        return true;
    }
};

// 输出：
// [DEBUG][TriangleApp][Renderer]TriangleApp 已构造
// [INFO][TriangleApp][Renderer]初始化大小 800x600
// [INFO][TriangleApp][Renderer]OpenGL 供应商：Google Inc.
// [INFO][TriangleApp][Renderer]OpenGL 版本：OpenGL ES 3.0
// [DEBUG][TriangleApp][Renderer]初始化完成
```

### 示例 2：网络管理器

```cpp
class NetworkManager : public ului::Object {
public:
    NetworkManager() : Object("Network") {
        LogI("NetworkManager 已初始化");
    }
    
    bool Connect(const char* host, int port) {
        LogD("正在连接到 %s:%d", host, port);
        
        if (!IsValidHost(host)) {
            LogE("无效的主机地址：%s", host);
            return false;
        }
        
        LogV("正在创建套接字...");
        int sock = CreateSocket();
        if (sock < 0) {
            LogE("创建套接字失败：errno=%d", errno);
            return false;
        }
        
        LogI("成功连接到 %s:%d", host, port);
        return true;
    }
};

// 输出：
// [INFO][NetworkManager][Network]NetworkManager 已初始化
// [DEBUG][NetworkManager][Network]正在连接到 example.com:8080
// [VERBOSE][NetworkManager][Network]正在创建套接字...
// [INFO][NetworkManager][Network]成功连接到 example.com:8080
```

### 示例 3：音频系统

```cpp
class AudioSystem : public ului::Object {
public:
    AudioSystem(const char* tag) : Object(tag) {
        LogI("音频系统已创建");
    }
    
    void PlaySound(const char* filename) {
        LogD("播放音效：%s", filename);
        
        if (!FileExists(filename)) {
            LogE("音效文件未找到：%s", filename);
            return;
        }
        
        LogV("正在从 %s 加载音频数据", filename);
        // 加载并播放...
        LogI("正在播放音效：%s", filename);
    }
};

// 使用不同的 TAG：
AudioSystem musicSystem("Music");
AudioSystem sfxSystem("SFX");

musicSystem.PlaySound("background.ogg");
// [DEBUG][AudioSystem][Music]播放音效：background.ogg
// [INFO][AudioSystem][Music]正在播放音效：background.ogg

sfxSystem.PlaySound("explosion.wav");
// [DEBUG][AudioSystem][SFX]播放音效：explosion.wav
// [INFO][AudioSystem][SFX]正在播放音效：explosion.wav
```

## 最佳实践

### 1. 选择描述性的 TAG

使用能清楚标识用途或子系统的 TAG：

```cpp
// 好的
TriangleApp() : Object("Renderer") { }
NetworkManager() : Object("Network") { }
ConfigLoader() : Object("Config") { }

// 不够清晰
MyClass() : Object("Main") { }
SomeClass() : Object("Class1") { }
```

### 2. 使用适当的日志级别

- **VERBOSE**：详细的调试信息，通常仅用于开发
- **DEBUG**：在开发和测试期间有用的调试信息
- **INFO**：关于程序流程的一般信息性消息
- **WARNING**：关于潜在问题的警告消息
- **ERROR**：可恢复错误的错误消息
- **FATAL**：阻止应用程序继续运行的致命错误

```cpp
void ProcessData() {
    LogV("正在处理数据块 %d/%d", current, total);  // 非常详细
    LogD("数据验证通过");                            // 调试信息
    LogI("处理完成");                                // 一般信息
    LogW("内存不足警告");                            // 潜在问题
    LogE("处理块失败：%s", error);                   // 可恢复错误
    LogF("严重错误：系统损坏");                      // 致命错误
}
```

### 3. 包含相关上下文

在日志消息中始终包含相关上下文：

```cpp
// 好的 - 包含上下文
LogE("加载纹理 '%s' 失败：文件未找到", filename);
LogW("网络延迟高：%dms（阈值：%dms）", latency, threshold);
LogI("在 %.2f 秒内加载了 %d 个资源", duration, count);

// 不够有用 - 缺少上下文
LogE("加载失败");
LogW("高延迟");
LogI("完成");
```

### 4. 不要过度记录日志

避免在紧密循环或高频函数中记录日志：

```cpp
// 不好 - 每帧都记录
void Update() {
    LogD("调用了 Update");  // 每秒调用 60+ 次！
    // ...
}

// 好的 - 记录状态变化或重要事件
void Update() {
    if (stateChanged) {
        LogI("状态从 %s 变更为 %s", oldState, newState);
    }
}
```

## 实现细节

### 类名提取

`Object` 类使用 C++ RTTI (`typeid`) 自动提取类名：

- **GCC/Clang**：通过去除前导数字和命名空间来还原名称
- **MSVC**：删除 "class " 前缀和命名空间限定符
- 结果：用于日志记录的清洁类名（例如 "TriangleApp"、"NetworkManager"）

### 线程安全

通过 `Object` 类进行的所有日志记录都是线程安全的，因为它使用了底层的 `Logger` 系统，该系统由互斥锁保护。

### 性能

- 类名提取在构造函数中仅执行一次
- TAG 存储为字符串成员
- 每次日志调用的开销很小（仅字符串格式化）
- 日志过滤在日志记录器级别进行，防止不必要的工作

## 与 Logger 系统集成

`Object` 类与 Logger 系统无缝协作：

```cpp
// 全局配置日志记录器
Logger::Log::Initialize();
Logger::Log::SetMinLogLevel(Logger::LogLevel::DEBUG);

// 添加文件输出
auto fileOut = std::make_shared<Logger::FileOutput>("app.log");
Logger::Log::AddOutput(fileOut);

// 现在所有 Object 派生类都使用此配置
MyClass obj;
obj.DoSomething();  // 日志同时输出到控制台和文件
```

## 相关文档

- [Logger 系统文档](LOGGER_CN.md) - 完整的日志系统参考
- [文件系统文档](FILE_SYSTEM_CN.md) - 跨平台文件 I/O
- [构建指南](BUILD_GUIDE.md) - 构建项目
