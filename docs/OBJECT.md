# Object Base Class Documentation

## Overview

The `Object` class is the base class for all functionality classes in the ULUI framework. It provides automatic class name detection and TAG-based logging with a unified format.

## Log Format

All log messages from classes derived from `Object` follow this format:

```
[LogLevel][ClassName][TAG]message
```

**Example:**
```
[DEBUG][TriangleApp][Renderer]Shader compiled successfully
[INFO][TriangleApp][Renderer]OpenGL Version: OpenGL ES 3.0
[ERROR][NetworkManager][Connection]Failed to connect to server: timeout
```

## Usage

### Basic Usage

```cpp
#include "object.h"

class MyClass : public ului::Object {
public:
    MyClass() : Object("MyTag") {
        LogI("MyClass constructed");
    }
    
    void DoSomething() {
        LogD("Starting operation");
        
        int result = PerformCalculation();
        if (result < 0) {
            LogE("Calculation failed with error code: %d", result);
            return;
        }
        
        LogI("Calculation completed successfully: result=%d", result);
    }
    
private:
    int PerformCalculation() {
        // ... implementation ...
        return 42;
    }
};
```

### Constructor

The `Object` constructor requires a TAG parameter:

```cpp
explicit Object(const char* tag);
```

- **tag**: A user-defined tag string that identifies this object instance
  - Common patterns: "Main", "Renderer", "Network", "Audio", etc.
  - Should be descriptive of the object's purpose
  - Will appear in all log messages from this object

### Logging Methods

All logging methods use printf-style formatting:

```cpp
void LogV(const char* format, ...) const;  // VERBOSE level
void LogD(const char* format, ...) const;  // DEBUG level
void LogI(const char* format, ...) const;  // INFO level
void LogW(const char* format, ...) const;  // WARNING level
void LogE(const char* format, ...) const;  // ERROR level
void LogF(const char* format, ...) const;  // FATAL level
```

### Accessor Methods

```cpp
const char* GetTag() const;        // Returns the TAG string
const char* GetClassName() const;  // Returns the class type name
```

## Examples

### Example 1: Triangle Renderer

```cpp
class TriangleApp : public ului::Object {
public:
    TriangleApp() : Object("Renderer") {
        LogD("TriangleApp constructed");
    }
    
    bool initialize(int width, int height) {
        LogI("Initializing with size %dx%d", width, height);
        
        if (width <= 0 || height <= 0) {
            LogE("Invalid dimensions: %dx%d", width, height);
            return false;
        }
        
        LogI("OpenGL Vendor: %s", glGetString(GL_VENDOR));
        LogI("OpenGL Version: %s", glGetString(GL_VERSION));
        
        LogD("Initialization complete");
        return true;
    }
};

// Output:
// [DEBUG][TriangleApp][Renderer]TriangleApp constructed
// [INFO][TriangleApp][Renderer]Initializing with size 800x600
// [INFO][TriangleApp][Renderer]OpenGL Vendor: Google Inc.
// [INFO][TriangleApp][Renderer]OpenGL Version: OpenGL ES 3.0
// [DEBUG][TriangleApp][Renderer]Initialization complete
```

### Example 2: Network Manager

```cpp
class NetworkManager : public ului::Object {
public:
    NetworkManager() : Object("Network") {
        LogI("NetworkManager initialized");
    }
    
    bool Connect(const char* host, int port) {
        LogD("Connecting to %s:%d", host, port);
        
        if (!IsValidHost(host)) {
            LogE("Invalid host address: %s", host);
            return false;
        }
        
        LogV("Creating socket...");
        int sock = CreateSocket();
        if (sock < 0) {
            LogE("Failed to create socket: errno=%d", errno);
            return false;
        }
        
        LogI("Connected successfully to %s:%d", host, port);
        return true;
    }
};

// Output:
// [INFO][NetworkManager][Network]NetworkManager initialized
// [DEBUG][NetworkManager][Network]Connecting to example.com:8080
// [VERBOSE][NetworkManager][Network]Creating socket...
// [INFO][NetworkManager][Network]Connected successfully to example.com:8080
```

### Example 3: Audio System

```cpp
class AudioSystem : public ului::Object {
public:
    AudioSystem(const char* tag) : Object(tag) {
        LogI("Audio system created");
    }
    
    void PlaySound(const char* filename) {
        LogD("Playing sound: %s", filename);
        
        if (!FileExists(filename)) {
            LogE("Sound file not found: %s", filename);
            return;
        }
        
        LogV("Loading audio data from %s", filename);
        // Load and play...
        LogI("Sound playing: %s", filename);
    }
};

// Usage with different tags:
AudioSystem musicSystem("Music");
AudioSystem sfxSystem("SFX");

musicSystem.PlaySound("background.ogg");
// [DEBUG][AudioSystem][Music]Playing sound: background.ogg
// [INFO][AudioSystem][Music]Sound playing: background.ogg

sfxSystem.PlaySound("explosion.wav");
// [DEBUG][AudioSystem][SFX]Playing sound: explosion.wav
// [INFO][AudioSystem][SFX]Sound playing: explosion.wav
```

## Best Practices

### 1. Choose Descriptive TAGs

Use TAGs that clearly identify the purpose or subsystem:

```cpp
// Good
TriangleApp() : Object("Renderer") { }
NetworkManager() : Object("Network") { }
ConfigLoader() : Object("Config") { }

// Less clear
MyClass() : Object("Main") { }
SomeClass() : Object("Class1") { }
```

### 2. Use Appropriate Log Levels

- **VERBOSE**: Detailed debug information, typically only for development
- **DEBUG**: Debug information useful during development and testing
- **INFO**: General informational messages about program flow
- **WARNING**: Warning messages about potential issues
- **ERROR**: Error messages for recoverable errors
- **FATAL**: Fatal errors that prevent the application from continuing

```cpp
void ProcessData() {
    LogV("Processing data chunk %d/%d", current, total);  // Very detailed
    LogD("Data validation passed");                        // Debug info
    LogI("Processing completed");                          // General info
    LogW("Low memory warning");                            // Potential issue
    LogE("Failed to process chunk: %s", error);           // Recoverable error
    LogF("Critical error: system corrupted");             // Fatal error
}
```

### 3. Include Relevant Context

Always include relevant context in log messages:

```cpp
// Good - includes context
LogE("Failed to load texture '%s': file not found", filename);
LogW("Network latency high: %dms (threshold: %dms)", latency, threshold);
LogI("Loaded %d resources in %.2f seconds", count, duration);

// Less helpful - lacks context
LogE("Load failed");
LogW("High latency");
LogI("Done");
```

### 4. Don't Over-Log

Avoid logging in tight loops or high-frequency functions:

```cpp
// Bad - logs every frame
void Update() {
    LogD("Update called");  // Called 60+ times per second!
    // ...
}

// Good - log state changes or important events
void Update() {
    if (stateChanged) {
        LogI("State changed from %s to %s", oldState, newState);
    }
}
```

## Implementation Details

### Class Name Extraction

The `Object` class automatically extracts the class name using C++ RTTI (`typeid`):

- **GCC/Clang**: Demangles names by stripping leading digits and namespaces
- **MSVC**: Removes "class " prefix and namespace qualifiers
- Result: Clean class name for logging (e.g., "TriangleApp", "NetworkManager")

### Thread Safety

All logging through the `Object` class is thread-safe because it uses the underlying `Logger` system, which is protected by mutexes.

### Performance

- Class name extraction happens once in the constructor
- TAG is stored as a string member
- Minimal overhead per log call (just string formatting)
- Log filtering at the logger level prevents unnecessary work

## Integration with Logger System

The `Object` class works seamlessly with the Logger system:

```cpp
// Configure logger globally
Logger::Log::Initialize();
Logger::Log::SetMinLogLevel(Logger::LogLevel::DEBUG);

// Add file output
auto fileOut = std::make_shared<Logger::FileOutput>("app.log");
Logger::Log::AddOutput(fileOut);

// Now all Object-derived classes use this configuration
MyClass obj;
obj.DoSomething();  // Logs go to console and file
```

## See Also

- [Logger System Documentation](LOGGER.md) - Complete logging system reference
- [File System Documentation](FILE_SYSTEM.md) - Cross-platform file I/O
- [Build Guide](BUILD_GUIDE.md) - Building the project
