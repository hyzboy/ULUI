# 日志系统文档

## 概述

ULUI Logger 提供了一个灵活的跨平台日志系统，灵感来自 Android 的 Log API。它支持多个可配置的输出目标，包括控制台、文件、网络、命名管道以及平台特定的日志系统（Android logcat、iOS/macOS 统一日志）。

## 功能特性

- **Android风格API**: 熟悉的 V/D/I/W/E/F 日志级别
- **多种输出目标**: 控制台、文件、网络(UDP)、命名管道(IPC)、平台特定(Android/iOS)
- **线程安全**: 所有操作都使用互斥锁保护
- **可配置**: 设置最小日志级别、标签过滤、启用/禁用输出
- **格式化输出**: 所有日志消息支持 printf 风格格式化
- **自动初始化**: 首次使用时配置默认输出
- **文件轮转**: 基于大小限制的自动日志文件轮转

## 日志级别

```cpp
enum class LogLevel {
    VERBOSE = 0,  // 详细调试信息
    DEBUG = 1,    // 调试信息
    INFO = 2,     // 一般信息
    WARNING = 3,  // 警告消息
    ERROR = 4,    // 错误消息
    FATAL = 5     // 致命错误
};
```

## 基本用法

### 简单日志记录

```cpp
#include "logger.h"

int main() {
    // 初始化日志器（可选 - 首次日志时自动完成）
    Logger::Log::Initialize();
    
    // 使用不同级别记录日志
    LOG_V("TAG", "详细消息: value=%d", 42);
    LOG_D("TAG", "调试消息");
    LOG_I("TAG", "信息消息");
    LOG_W("TAG", "警告消息");
    LOG_E("TAG", "错误消息");
    LOG_F("TAG", "致命错误消息");
    
    // 关闭（刷新所有输出）
    Logger::Log::Shutdown();
    return 0;
}
```

### 替代语法

```cpp
// 直接使用 Logger::Log 方法
Logger::Log::V("MyApp", "详细: %s", "细节");
Logger::Log::D("MyApp", "调试信息");
Logger::Log::I("MyApp", "信息");
Logger::Log::W("MyApp", "警告！");
Logger::Log::E("MyApp", "发生错误");
Logger::Log::F("MyApp", "致命错误！");

// 通用日志方法
Logger::Log::Log(Logger::LogLevel::INFO, "MyApp", "通用日志");
```

## 输出目标

### 1. 控制台输出

输出到 stdout（INFO 及以下）或 stderr（WARNING 及以上），支持可选的颜色。

```cpp
// 创建带颜色的控制台输出
auto console = std::make_shared<Logger::ConsoleOutput>(true);
Logger::Log::AddOutput(console);

// 禁用颜色
auto plainConsole = std::make_shared<Logger::ConsoleOutput>(false);
```

**输出格式:**
```
2025-12-18 13:00:00.123/I MyApp: 信息消息
```

**颜色**（启用时）:
- VERBOSE: 白色
- DEBUG: 青色
- INFO: 绿色
- WARNING: 黄色
- ERROR: 红色
- FATAL: 洋红色

### 2. 文件输出

将日志写入文件，当超过大小限制时自动轮转。

```cpp
// 创建文件输出
auto fileOutput = std::make_shared<Logger::FileOutput>("app.log", false);

// 配置轮转（最大10MB，保留5个备份）
fileOutput->SetMaxFileSize(10 * 1024 * 1024);  // 10MB
fileOutput->SetMaxBackupFiles(5);

Logger::Log::AddOutput(fileOutput);
```

**文件轮转:**
- 当日志文件达到最大大小时进行轮转
- 当前文件: `app.log`
- 备份: `app.log.1`, `app.log.2`, ..., `app.log.N`
- 达到最大备份数量时删除最旧的备份

### 3. 网络输出 (UDP)

通过 UDP 将日志消息发送到远程服务器。

```cpp
// 将日志发送到远程日志服务器
auto networkOutput = std::make_shared<Logger::NetworkOutput>("192.168.1.100", 5140);
Logger::Log::AddOutput(networkOutput);
```

**使用场景:**
- 远程日志聚合
- 调试无法直接访问的设备
- 分布式系统中的集中日志记录

### 4. 命名管道输出 (IPC)

将日志写入命名管道用于进程间通信。

```cpp
#ifdef _WIN32
    // Windows 命名管道
    auto pipeOutput = std::make_shared<Logger::PipeOutput>("MyAppLog");
#else
    // Unix FIFO
    auto pipeOutput = std::make_shared<Logger::PipeOutput>("/tmp/myapp.log");
#endif

Logger::Log::AddOutput(pipeOutput);
```

**使用场景:**
- 与调试工具通信
- 实时日志监控
- 与系统日志守护进程集成

### 5. 平台特定输出

#### Android Logcat

在 Android 平台上自动添加。日志显示在 `adb logcat` 中。

```cpp
// 在 Android 上自动包含
#ifdef __ANDROID__
auto androidOutput = std::make_shared<Logger::AndroidOutput>();
Logger::Log::AddOutput(androidOutput);
#endif
```

**查看日志:**
```bash
adb logcat | grep MyApp
```

#### iOS/macOS 统一日志

在 Apple 平台上自动添加。日志显示在 Console.app 中或可以使用 `log` 命令查看。

```cpp
// 在 iOS/macOS 上自动包含
#ifdef __APPLE__
auto appleOutput = std::make_shared<Logger::AppleOutput>();
Logger::Log::AddOutput(appleOutput);
#endif
```

**查看日志:**
```bash
# macOS
log stream --predicate 'subsystem == "com.ului.app"'

# iOS (设备)
log collect --device --start 'YYYY-MM-DD HH:MM:SS'
```

## 高级配置

### 最小日志级别

过滤低于特定级别的日志:

```cpp
// 仅记录 WARNING 及以上级别
Logger::Log::SetMinLogLevel(Logger::LogLevel::WARNING);

// 这条不会被记录
LOG_D("MyApp", "调试消息");

// 这条会被记录
LOG_W("MyApp", "警告消息");
```

### 标签过滤

仅记录具有特定标签的消息:

```cpp
// 仅记录 "Network" 标签的消息
Logger::Log::SetTagFilter("Network");

LOG_D("Network", "连接已建立");  // 被记录
LOG_D("UI", "按钮被点击");       // 不被记录

// 清除过滤器
Logger::Log::ClearTagFilter();
```

### 启用/禁用输出

控制单个输出而不移除它们:

```cpp
auto fileOutput = std::make_shared<Logger::FileOutput>("app.log");
Logger::Log::AddOutput(fileOutput);

// 临时禁用文件日志
fileOutput->SetEnabled(false);

// 重新启用
fileOutput->SetEnabled(true);
```

### 管理输出

```cpp
// 添加输出
auto output = std::make_shared<Logger::FileOutput>("app.log");
Logger::Log::AddOutput(output);

// 移除特定输出
Logger::Log::RemoveOutput(output);

// 清除所有输出
Logger::Log::ClearOutputs();
```

## 完整示例

```cpp
#include "logger.h"
#include <memory>

int main() {
    // 初始化日志器
    Logger::Log::Initialize();
    
    // 如需要可清除默认输出
    Logger::Log::ClearOutputs();
    
    // 添加带颜色的控制台输出
    auto console = std::make_shared<Logger::ConsoleOutput>(true);
    Logger::Log::AddOutput(console);
    
    // 添加带轮转的文件输出
    auto fileOutput = std::make_shared<Logger::FileOutput>("app.log");
    fileOutput->SetMaxFileSize(5 * 1024 * 1024);  // 5MB
    fileOutput->SetMaxBackupFiles(3);
    Logger::Log::AddOutput(fileOutput);
    
    // 添加网络输出（可选）
    auto networkOutput = std::make_shared<Logger::NetworkOutput>("10.0.0.1", 5140);
    Logger::Log::AddOutput(networkOutput);
    
    // 设置最小日志级别为 DEBUG
    Logger::Log::SetMinLogLevel(Logger::LogLevel::DEBUG);
    
    // 应用程序日志
    LOG_I("Main", "应用程序已启动");
    LOG_D("Main", "正在初始化子系统");
    
    // 模拟一些工作
    try {
        // 做一些事情
        LOG_I("Worker", "正在处理数据");
    } catch (const std::exception& e) {
        LOG_E("Worker", "捕获异常: %s", e.what());
    }
    
    LOG_I("Main", "应用程序正在关闭");
    
    // 清理
    Logger::Log::Shutdown();
    return 0;
}
```

## 平台特定说明

### Windows

- 控制台颜色需要 Windows 10 或更高版本支持 ANSI 转义码
- 命名管道使用 `\\.\pipe\` 命名空间
- 网络输出需要 ws2_32.lib（自动链接）

### Linux

- 命名管道使用 FIFO 文件（使用 `mkfifo` 创建）
- 日志文件路径遵循 XDG 目录规范
- 大多数终端支持控制台颜色

### macOS

- 统一日志与 Console.app 集成
- Terminal.app 支持控制台颜色
- 命名管道使用 UNIX 域套接字或 FIFO

### Android

- 自动包含 Logcat 输出
- 使用 `adb logcat` 查看日志
- 文件日志通常存放在应用内部存储
- 网络输出对无 USB 调试的设备很有用

### iOS

- 统一日志与 Console.app 和 Instruments 集成
- 文件日志存放在应用的 Documents 或 Library 目录
- iOS 上不常用命名管道

## 最佳实践

1. **使用适当的日志级别**
   - VERBOSE: 极其详细，仅开发使用
   - DEBUG: 调试信息，开发版本
   - INFO: 一般信息，生产环境
   - WARNING: 意外但已处理的情况
   - ERROR: 可能影响功能的错误
   - FATAL: 需要注意的关键错误

2. **使用描述性标签**
   ```cpp
   LOG_D("NetworkManager", "已连接到服务器");
   LOG_E("DatabaseHelper", "打开数据库失败");
   LOG_I("AudioEngine", "播放已开始");
   ```

3. **早期初始化**
   ```cpp
   int main() {
       Logger::Log::Initialize();
       // 应用程序其余部分
   }
   ```

4. **干净地关闭**
   ```cpp
   // 应用程序退出前
   Logger::Log::Shutdown();
   ```

5. **根据构建类型配置**
   ```cpp
   #ifdef DEBUG
       Logger::Log::SetMinLogLevel(Logger::LogLevel::VERBOSE);
   #else
       Logger::Log::SetMinLogLevel(Logger::LogLevel::INFO);
   #endif
   ```

6. **保护敏感数据**
   ```cpp
   // 不要记录密码、令牌或个人数据
   LOG_D("Auth", "用户已登录");  // 好的
   LOG_D("Auth", "密码: %s", pwd);  // 不好！
   ```

7. **使用文件轮转**
   ```cpp
   // 防止磁盘空间问题
   fileOutput->SetMaxFileSize(10 * 1024 * 1024);  // 10MB
   fileOutput->SetMaxBackupFiles(5);
   ```

## 性能考虑

- 日志记录有性能开销，特别是文件和网络输出
- 使用适当的日志级别在生产环境中过滤不必要的日志
- 考虑在发布版本中禁用 VERBOSE 和 DEBUG 日志
- 文件轮转是同步的 - 轮转时会有短暂暂停
- 网络输出是非阻塞的，但网络不可用时可能丢失消息

## 线程安全

所有 Logger 操作都是线程安全的:
- 多个线程可以同时记录日志
- 输出目标使用内部互斥锁
- 日志级别和过滤器更改是原子的

## 与现有代码集成

日志器可以与标准输出共存:

```cpp
// 混合使用 logger 和 cout/cerr
std::cout << "使用 cout" << std::endl;
LOG_I("MyApp", "使用 logger");
std::cerr << "使用 cerr" << std::endl;
```

渐进式迁移可以创建包装宏:

```cpp
#define LEGACY_LOG(msg) LOG_I("Legacy", "%s", msg)
```

## 示例：多输出配置

```cpp
void ConfigureLogger() {
    // 清除默认输出
    Logger::Log::ClearOutputs();
    
    // 开发环境: 控制台 + 文件
    #ifdef DEBUG
        auto console = std::make_shared<Logger::ConsoleOutput>(true);
        Logger::Log::AddOutput(console);
        
        auto fileOut = std::make_shared<Logger::FileOutput>("debug.log");
        Logger::Log::AddOutput(fileOut);
        
        Logger::Log::SetMinLogLevel(Logger::LogLevel::VERBOSE);
    #else
    // 生产环境: 仅文件，较高级别
        auto fileOut = std::make_shared<Logger::FileOutput>("app.log");
        fileOut->SetMaxFileSize(5 * 1024 * 1024);
        fileOut->SetMaxBackupFiles(3);
        Logger::Log::AddOutput(fileOut);
        
        Logger::Log::SetMinLogLevel(Logger::LogLevel::INFO);
    #endif
}
```

## 常见问题

**Q: 如何减少日志文件大小？**  
A: 设置最小日志级别和文件轮转：
```cpp
Logger::Log::SetMinLogLevel(Logger::LogLevel::INFO);
fileOutput->SetMaxFileSize(5 * 1024 * 1024);
fileOutput->SetMaxBackupFiles(3);
```

**Q: 如何在多个模块中使用相同的日志配置？**  
A: 在应用启动时配置一次，所有模块共享相同的日志器实例。

**Q: 网络输出失败会影响应用吗？**  
A: 不会，网络输出失败是静默的，不会阻塞或崩溃应用。

**Q: 可以动态改变日志级别吗？**  
A: 可以，随时调用 `Logger::Log::SetMinLogLevel()`。

**Q: 如何在Android上查看日志？**  
A: 使用 `adb logcat` 或Android Studio的Logcat窗口。
