# Logger System Documentation

## Overview

The ULUI Logger provides a flexible, cross-platform logging system inspired by Android's Log API. It supports multiple configurable output targets including console, files, network, named pipes, and platform-specific logging systems (Android logcat, iOS/macOS unified logging).

## Features

- **Android-Style API**: Familiar V/D/I/W/E/F logging levels
- **Multiple Output Targets**: Console, File, Network (UDP), Named Pipes (IPC), Platform-specific (Android/iOS)
- **Thread-Safe**: All operations are protected with mutexes
- **Configurable**: Set minimum log level, tag filters, enable/disable outputs
- **Formatted Output**: Printf-style formatting for all log messages
- **Automatic Initialization**: Default outputs configured on first use
- **File Rotation**: Automatic log file rotation based on size limits

## Log Levels

```cpp
enum class LogLevel {
    VERBOSE = 0,  // Detailed information for debugging
    DEBUG = 1,    // Debug information
    INFO = 2,     // General information
    WARNING = 3,  // Warning messages
    ERROR = 4,    // Error messages
    FATAL = 5     // Fatal errors
};
```

## Basic Usage

### Simple Logging

```cpp
#include "logger.h"

int main() {
    // Initialize logger (optional - done automatically on first log)
    Logger::Log::Initialize();
    
    // Log messages with different levels
    LOG_V("TAG", "Verbose message: value=%d", 42);
    LOG_D("TAG", "Debug message");
    LOG_I("TAG", "Info message");
    LOG_W("TAG", "Warning message");
    LOG_E("TAG", "Error message");
    LOG_F("TAG", "Fatal error message");
    
    // Shutdown (flushes all outputs)
    Logger::Log::Shutdown();
    return 0;
}
```

### Alternative Syntax

```cpp
// Using Logger::Log methods directly
Logger::Log::V("MyApp", "Verbose: %s", "details");
Logger::Log::D("MyApp", "Debug info");
Logger::Log::I("MyApp", "Information");
Logger::Log::W("MyApp", "Warning!");
Logger::Log::E("MyApp", "Error occurred");
Logger::Log::F("MyApp", "Fatal error!");

// Generic log method
Logger::Log::Log(Logger::LogLevel::INFO, "MyApp", "Generic log");
```

## Output Targets

### 1. Console Output

Outputs to stdout (INFO and below) or stderr (WARNING and above), with optional color support.

```cpp
// Create console output with colors
auto console = std::make_shared<Logger::ConsoleOutput>(true);
Logger::Log::AddOutput(console);

// Disable colors
auto plainConsole = std::make_shared<Logger::ConsoleOutput>(false);
```

**Output Format:**
```
2025-12-18 13:00:00.123/I MyApp: Information message
```

**Colors** (when enabled):
- VERBOSE: White
- DEBUG: Cyan
- INFO: Green
- WARNING: Yellow
- ERROR: Red
- FATAL: Magenta

### 2. File Output

Writes logs to a file with automatic rotation when size limits are exceeded.

```cpp
// Create file output
auto fileOutput = std::make_shared<Logger::FileOutput>("app.log", false);

// Configure rotation (10MB max, keep 5 backups)
fileOutput->SetMaxFileSize(10 * 1024 * 1024);  // 10MB
fileOutput->SetMaxBackupFiles(5);

Logger::Log::AddOutput(fileOutput);
```

**File Rotation:**
- When the log file reaches the maximum size, it's rotated
- Current file: `app.log`
- Backups: `app.log.1`, `app.log.2`, ..., `app.log.N`
- Oldest backups are deleted when max backup count is reached

### 3. Network Output (UDP)

Sends log messages to a remote server via UDP.

```cpp
// Send logs to a remote log server
auto networkOutput = std::make_shared<Logger::NetworkOutput>("192.168.1.100", 5140);
Logger::Log::AddOutput(networkOutput);
```

**Use Cases:**
- Remote log aggregation
- Debugging devices without direct access
- Centralized logging in distributed systems

### 4. Named Pipe Output (IPC)

Writes logs to a named pipe for inter-process communication.

```cpp
#ifdef _WIN32
    // Windows named pipe
    auto pipeOutput = std::make_shared<Logger::PipeOutput>("MyAppLog");
#else
    // Unix FIFO
    auto pipeOutput = std::make_shared<Logger::PipeOutput>("/tmp/myapp.log");
#endif

Logger::Log::AddOutput(pipeOutput);
```

**Use Cases:**
- Communication with debugging tools
- Real-time log monitoring
- Integration with system logging daemons

### 5. Platform-Specific Outputs

#### Android Logcat

Automatically added on Android platforms. Logs appear in `adb logcat`.

```cpp
// Automatically included on Android
#ifdef __ANDROID__
auto androidOutput = std::make_shared<Logger::AndroidOutput>();
Logger::Log::AddOutput(androidOutput);
#endif
```

**View logs:**
```bash
adb logcat | grep MyApp
```

#### iOS/macOS Unified Logging

Automatically added on Apple platforms. Logs appear in Console.app or can be viewed with `log` command.

```cpp
// Automatically included on iOS/macOS
#ifdef __APPLE__
auto appleOutput = std::make_shared<Logger::AppleOutput>();
Logger::Log::AddOutput(appleOutput);
#endif
```

**View logs:**
```bash
# macOS
log stream --predicate 'subsystem == "com.ului.app"'

# iOS (device)
log collect --device --start 'YYYY-MM-DD HH:MM:SS'
```

## Advanced Configuration

### Minimum Log Level

Filter out logs below a certain level:

```cpp
// Only log WARNING and above
Logger::Log::SetMinLogLevel(Logger::LogLevel::WARNING);

// This won't be logged
LOG_D("MyApp", "Debug message");

// This will be logged
LOG_W("MyApp", "Warning message");
```

### Tag Filtering

Only log messages with a specific tag:

```cpp
// Only log messages with "Network" tag
Logger::Log::SetTagFilter("Network");

LOG_D("Network", "Connection established");  // Logged
LOG_D("UI", "Button clicked");                // Not logged

// Clear filter
Logger::Log::ClearTagFilter();
```

### Enable/Disable Outputs

Control individual outputs without removing them:

```cpp
auto fileOutput = std::make_shared<Logger::FileOutput>("app.log");
Logger::Log::AddOutput(fileOutput);

// Temporarily disable file logging
fileOutput->SetEnabled(false);

// Re-enable
fileOutput->SetEnabled(true);
```

### Managing Outputs

```cpp
// Add output
auto output = std::make_shared<Logger::FileOutput>("app.log");
Logger::Log::AddOutput(output);

// Remove specific output
Logger::Log::RemoveOutput(output);

// Clear all outputs
Logger::Log::ClearOutputs();
```

## Complete Example

```cpp
#include "logger.h"
#include <memory>

int main() {
    // Initialize logger
    Logger::Log::Initialize();
    
    // Clear default outputs if needed
    Logger::Log::ClearOutputs();
    
    // Add console output with colors
    auto console = std::make_shared<Logger::ConsoleOutput>(true);
    Logger::Log::AddOutput(console);
    
    // Add file output with rotation
    auto fileOutput = std::make_shared<Logger::FileOutput>("app.log");
    fileOutput->SetMaxFileSize(5 * 1024 * 1024);  // 5MB
    fileOutput->SetMaxBackupFiles(3);
    Logger::Log::AddOutput(fileOutput);
    
    // Add network output (optional)
    auto networkOutput = std::make_shared<Logger::NetworkOutput>("10.0.0.1", 5140);
    Logger::Log::AddOutput(networkOutput);
    
    // Set minimum log level to DEBUG
    Logger::Log::SetMinLogLevel(Logger::LogLevel::DEBUG);
    
    // Application logging
    LOG_I("Main", "Application started");
    LOG_D("Main", "Initializing subsystems");
    
    // Simulate some work
    try {
        // Do something
        LOG_I("Worker", "Processing data");
    } catch (const std::exception& e) {
        LOG_E("Worker", "Exception caught: %s", e.what());
    }
    
    LOG_I("Main", "Application shutting down");
    
    // Cleanup
    Logger::Log::Shutdown();
    return 0;
}
```

## Platform-Specific Notes

### Windows

- Console colors require Windows 10 or later for ANSI escape codes
- Named pipes use the `\\.\pipe\` namespace
- Network output requires ws2_32.lib (automatically linked)

### Linux

- Named pipes use FIFO files (create with `mkfifo`)
- XDG directories respected for log file paths
- Console colors supported in most terminals

### macOS

- Unified logging integrates with Console.app
- Console colors supported in Terminal.app
- Named pipes use UNIX domain sockets or FIFOs

### Android

- Logcat output automatically included
- Use `adb logcat` to view logs
- File logs typically go to app's internal storage
- Network output useful for devices without USB debug

### iOS

- Unified logging integrates with Console.app and Instruments
- File logs in app's Documents or Library directories
- Named pipes not commonly used on iOS

## Best Practices

1. **Use Appropriate Log Levels**
   - VERBOSE: Extremely detailed, development only
   - DEBUG: Debugging information, development builds
   - INFO: General information, production
   - WARNING: Unexpected but handled situations
   - ERROR: Errors that may affect functionality
   - FATAL: Critical errors requiring attention

2. **Use Descriptive Tags**
   ```cpp
   LOG_D("NetworkManager", "Connected to server");
   LOG_E("DatabaseHelper", "Failed to open database");
   LOG_I("AudioEngine", "Playback started");
   ```

3. **Initialize Early**
   ```cpp
   int main() {
       Logger::Log::Initialize();
       // Rest of application
   }
   ```

4. **Shutdown Cleanly**
   ```cpp
   // Before application exit
   Logger::Log::Shutdown();
   ```

5. **Configure for Build Type**
   ```cpp
   #ifdef DEBUG
       Logger::Log::SetMinLogLevel(Logger::LogLevel::VERBOSE);
   #else
       Logger::Log::SetMinLogLevel(Logger::LogLevel::INFO);
   #endif
   ```

6. **Protect Sensitive Data**
   ```cpp
   // Don't log passwords, tokens, or personal data
   LOG_D("Auth", "User logged in");  // Good
   LOG_D("Auth", "Password: %s", pwd);  // BAD!
   ```

7. **Use File Rotation**
   ```cpp
   // Prevent disk space issues
   fileOutput->SetMaxFileSize(10 * 1024 * 1024);  // 10MB
   fileOutput->SetMaxBackupFiles(5);
   ```

## Performance Considerations

- Logging has a performance cost, especially for file and network outputs
- Use appropriate log levels to filter unnecessary logs in production
- Consider disabling VERBOSE and DEBUG logs in release builds
- File rotation happens synchronously - plan for brief pauses when rotating
- Network output is non-blocking but may lose messages if network is unavailable

## Thread Safety

All Logger operations are thread-safe:
- Multiple threads can log simultaneously
- Output targets use internal mutexes
- Log level and filter changes are atomic

## Integration with Existing Code

The logger can coexist with standard output:

```cpp
// Mix logger with cout/cerr
std::cout << "Using cout" << std::endl;
LOG_I("MyApp", "Using logger");
std::cerr << "Using cerr" << std::endl;
```

For gradual migration, you can create wrapper macros:

```cpp
#define LEGACY_LOG(msg) LOG_I("Legacy", "%s", msg)
```
