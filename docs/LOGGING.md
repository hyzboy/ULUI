# ULUI Logging System

A modern C++20 logging system for ULUI, inspired by [hyzboy/CMCore](https://github.com/hyzboy/CMCore/tree/master/inc/hgl/log).

## Features

- **C++20 Modern Design**: Uses `std::format`, `std::source_location`, and STL containers
- **No Namespaces**: Simple global scope API
- **Type-Safe Formatting**: Compile-time format string checking with `std::format_string`
- **Zero-Overhead Debug Logs**: Debug/Verbose logs completely eliminated in Release builds
- **Object-Oriented**: Each class can have its own logger with automatic type information
- **Source Location Tracking**: Automatic file name and line number capture

## Quick Start

### Global Logging

```cpp
#include "log/ObjectLogger.h"

int main() {
    GLOBAL_LOG_INFO("Application started");
    GLOBAL_LOG_WARNING("Memory usage: {:.2f}MB", 128.5);
    GLOBAL_LOG_ERROR("Failed to load file: {}", filename);
    return 0;
}
```

### Object Logging

```cpp
#include "log/ObjectLogger.h"

class MyComponent {
    OBJECT_LOGGER  // Automatically creates this->Log member

public:
    MyComponent() {
        Log.SetLoggerInstanceName("Component-001");
    }

    void DoWork() {
        LogInfo("Starting work...");
        LogDebug("Processing item {}/{}", 1, 10);
        LogWarning("Low memory: {:.2f}MB remaining", 64.5);
    }
};
```

## Log Levels

The system supports 7 log levels (from lowest to highest priority):

1. **Verbose** - Detailed diagnostic information
2. **Debug** - Debugging information
3. **Info** - General informational messages
4. **Notice** - Normal but significant events
5. **Warning** - Warning messages
6. **Error** - Error messages
7. **Fatal** - Critical errors

### Conditional Compilation

- `LogVerbose()` and `LogDebug()` are **only active in Debug builds** (when `_DEBUG` is defined)
- In Release builds, these calls are completely eliminated (`((void)0)`)
- Other log levels (Info, Notice, Warning, Error, Fatal) are always active

## API Reference

### Global Logging Macros

```cpp
GLOBAL_LOG_VERBOSE(format, args...)
GLOBAL_LOG_DEBUG(format, args...)
GLOBAL_LOG_INFO(format, args...)
GLOBAL_LOG_NOTICE(format, args...)
GLOBAL_LOG_WARNING(format, args...)
GLOBAL_LOG_ERROR(format, args...)
GLOBAL_LOG_FATAL(format, args...)
```

### Object Logging Macros

```cpp
LogVerbose(format, args...)  // Debug builds only
LogDebug(format, args...)     // Debug builds only
LogInfo(format, args...)
LogNotice(format, args...)
LogWarning(format, args...)
LogError(format, args...)
LogFatal(format, args...)
```

### ObjectLogger Class

```cpp
class ObjectLogger {
public:
    // Constructors
    ObjectLogger();
    ObjectLogger(const std::type_info* type_info);
    ObjectLogger(const char* name);

    // Instance naming
    void SetLoggerInstanceName(const std::string& name);
    const std::string& GetLoggerInstanceName() const;

    // Type information
    const std::string& GetLoggerTypeName() const;
    size_t GetLoggerTypeHash() const;

    // Core logging methods
    void LogString(const std::source_location& loc, LogLevel level, 
                   const char* str, size_t len);
    
    template<typename... Args>
    void LogFormat(const std::source_location& loc, LogLevel level,
                   std::format_string<Args...> fmt, Args&&... args);

    // Level-specific methods (Verbose, Debug, Info, Notice, Warning, Error, Fatal)
    template<typename... Args>
    void Info(const std::source_location& loc, 
              std::format_string<Args...> fmt, Args&&... args);
    // ... and similar for other levels
};
```

## Format String Syntax

The logging system uses C++20 `std::format`, which is similar to Python's format strings:

```cpp
LogInfo("Simple message");                          // No formatting
LogInfo("Integer: {}", 42);                         // Integer
LogInfo("Float: {:.2f}", 3.14159);                 // Float with 2 decimals
LogInfo("String: {}", "Hello");                     // String
LogInfo("Multiple: {} {} {}", 1, 2.5, "three");    // Multiple arguments
LogInfo("Named: {0} {1} {0}", "A", "B");           // Positional
```

For more format specifications, see [std::format documentation](https://en.cppreference.com/w/cpp/utility/format/formatter).

## Examples

See the `examples/` directory for complete examples:

- `log_example.cpp` - Basic usage example
- `comprehensive_log_test.cpp` - Comprehensive test of all features

### Building Examples

```bash
cd examples
mkdir build && cd build
cmake ..
make

# Run examples
./log_example
./comprehensive_log_test
```

## Architecture

### Class Hierarchy

```
Logger (base class)
  ↑
  └─ Responsibility chain pattern support
     (future implementations can chain loggers)

ObjectLogger
  └─ Manages per-object logging with type information
  └─ Buffers log output
  └─ Formats messages with std::format
```

### Data Structures

- **LogLevel**: Enum class defining log severity levels
- **LogMessage**: Struct containing all log message metadata
  - Type information (`std::type_info*`)
  - Instance name
  - Source location (file, line, function)
  - Log level
  - Message content

## Design Notes

This logging system is inspired by [hyzboy/CMCore](https://github.com/hyzboy/CMCore) but modernized:

- Uses C++20 `std::format` instead of custom formatters
- Uses `std::source_location` instead of `__FILE__`/`__LINE__` macros
- Uses `std::vector` and `std::string` instead of custom containers
- No namespaces for simpler API
- Supports both format-style and direct string logging

## Requirements

- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20+

## License

Same as the parent ULUI project.
