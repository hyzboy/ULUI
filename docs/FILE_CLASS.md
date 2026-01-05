# File Class Documentation

## Overview

The `File` class provides a unified interface for file access in ULUI, supporting both:
- **Internal assets** (read-only): Resources bundled with the application
- **External files** (read-write): Files on the file system

It offers standard file I/O operations (read, write, seek, tell) with a modern C++ interface.

## Features

- **Unified API**: Single interface for both asset and external file access
- **Path-based**: Uses the `Path` class for cross-platform path handling
- **Standard operations**: read, write, seek, tell, flush
- **Metadata queries**: canRead, canWrite, GetLength, IsEof
- **Move semantics**: Efficient file handle management
- **Binary and text support**: Read/write both binary data and text

## Basic Usage

### Include the Header

```cpp
#include "File.h"
using namespace ului;
```

### Reading from Assets

```cpp
// Initialize file system (once at application start)
FileSystem::Initialize();

// Read shader from assets (automatically tries asset first)
File shaderFile(Path("shaders/vertex.glsl"));
if (shaderFile.IsOpen()) {
    std::string shaderCode = shaderFile.ReadAllText();
    // Use shader code...
}
```

### Writing to External Files

```cpp
// Open file for writing
File outputFile(Path("save.dat"), File::OpenMode::Write, false);
if (outputFile.IsOpen()) {
    std::string data = "Player progress: Level 5";
    outputFile.Write(data);
}
```

### Reading from External Files

```cpp
// Open external file for reading
File inputFile(Path("config.ini"), File::OpenMode::Read, false);
if (inputFile.IsOpen()) {
    std::string config = inputFile.ReadAllText();
    // Parse config...
}
```

## File Open Modes

The `File::OpenMode` enum specifies how a file should be opened:

| Mode | Description | Assets | External |
|------|-------------|--------|----------|
| `Read` | Open for reading only | ✓ | ✓ |
| `Write` | Open for writing (truncate if exists) | ✗ | ✓ |
| `ReadWrite` | Open for reading and writing | ✗ | ✓ |
| `Append` | Open for writing, append to end | ✗ | ✓ |

## Core Operations

### Opening Files

```cpp
// Method 1: Constructor with automatic open
File file(Path("data.txt"), File::OpenMode::Read);

// Method 2: Default constructor + explicit Open()
File file;
file.Open(Path("data.txt"), File::OpenMode::Read);

// Method 3: Prefer external file over asset
File file(Path("config.ini"), File::OpenMode::Read, false);
```

The third parameter `preferAsset` (default: `true`) controls which source to try first:
- `true`: Try asset first, fallback to external file
- `false`: Try external file first, fallback to asset

### Reading Data

```cpp
// Read all content as text
std::string text = file.ReadAllText();

// Read all content as binary
std::vector<uint8_t> data = file.ReadAll();

// Read specific number of bytes
std::vector<uint8_t> chunk = file.Read(1024);

// Read into pre-allocated buffer
char buffer[256];
size_t bytesRead = file.Read(buffer, sizeof(buffer));
```

### Writing Data

```cpp
// Write string
size_t written = file.Write("Hello, World!");

// Write vector
std::vector<uint8_t> data = {0x01, 0x02, 0x03};
file.Write(data);

// Write raw buffer
uint8_t buffer[100];
file.Write(buffer, sizeof(buffer));

// Ensure writes are flushed to disk
file.Flush();
```

### Seeking and Positioning

```cpp
// Seek to beginning
file.Seek(0, File::SeekOrigin::Begin);

// Seek forward from current position
file.Seek(100, File::SeekOrigin::Current);

// Seek backward from end
file.Seek(-50, File::SeekOrigin::End);

// Get current position
int64_t pos = file.Tell();

// Get file length
int64_t length = file.GetLength();

// Check if at end of file
if (file.IsEof()) {
    // End of file reached
}
```

### Closing Files

```cpp
// Explicit close
file.Close();

// Files are automatically closed when File object is destroyed
{
    File file("temp.txt", File::OpenMode::Write);
    file.Write("data");
} // file is automatically closed here
```

## File Properties and Queries

```cpp
// Check if file is open
if (file.IsOpen()) {
    // File operations...
}

// Check capabilities
if (file.CanRead()) {
    // Can read from file
}
if (file.CanWrite()) {
    // Can write to file
}

// Check file type
if (file.IsAsset()) {
    // File is from asset package (read-only)
}
if (file.IsExternal()) {
    // File is from external file system
}

// Get file path
const Path& path = file.GetPath();
std::cout << "File: " << path.ToString() << std::endl;

// Get file size
int64_t size = file.GetLength();
std::cout << "Size: " << size << " bytes" << std::endl;
```

## Advanced Examples

### Reading Configuration File

```cpp
File configFile(Path("settings.json"), File::OpenMode::Read, false);
if (configFile.IsOpen()) {
    std::string json = configFile.ReadAllText();
    // Parse JSON with your favorite library
    // auto config = parseJson(json);
}
```

### Binary File Processing

```cpp
// Read binary file in chunks
File binFile(Path("data.bin"), File::OpenMode::Read, false);
if (binFile.IsOpen()) {
    const size_t CHUNK_SIZE = 4096;
    std::vector<uint8_t> chunk;
    
    while (!binFile.IsEof()) {
        chunk = binFile.Read(CHUNK_SIZE);
        if (!chunk.empty()) {
            // Process chunk...
        }
    }
}
```

### Appending to Log File

```cpp
File logFile(Path("app.log"), File::OpenMode::Append, false);
if (logFile.IsOpen()) {
    std::string logEntry = "[2024-01-01 12:00:00] Application started\n";
    logFile.Write(logEntry);
    logFile.Flush(); // Ensure immediate write
}
```

### Random Access File

```cpp
File dataFile(Path("records.dat"), File::OpenMode::ReadWrite, false);
if (dataFile.IsOpen()) {
    // Read record at specific position
    const size_t RECORD_SIZE = 128;
    int recordIndex = 10;
    
    dataFile.Seek(recordIndex * RECORD_SIZE, File::SeekOrigin::Begin);
    auto record = dataFile.Read(RECORD_SIZE);
    
    // Modify record
    // ... modify data ...
    
    // Write back
    dataFile.Seek(recordIndex * RECORD_SIZE, File::SeekOrigin::Begin);
    dataFile.Write(record);
}
```

### Loading Shader from Assets

```cpp
// Automatically loads from assets directory
File vertShader(Path("shaders/triangle.vert"));
File fragShader(Path("shaders/triangle.frag"));

if (vertShader.IsOpen() && fragShader.IsOpen()) {
    std::string vertCode = vertShader.ReadAllText();
    std::string fragCode = fragShader.ReadAllText();
    
    // Compile shaders...
    // compileShader(vertCode, fragCode);
}
```

## Move Semantics

The `File` class supports move semantics for efficient resource management:

```cpp
// Move constructor
File createFile() {
    File file("data.txt", File::OpenMode::Write);
    file.Write("data");
    return file; // File is moved, not copied
}

File myFile = createFile(); // Move, no file handle duplication

// Move assignment
File file1("a.txt", File::OpenMode::Read);
File file2("b.txt", File::OpenMode::Read);
file2 = std::move(file1); // file1 is now invalid, file2 holds file1's handle
```

## Error Handling

```cpp
File file(Path("nonexistent.txt"), File::OpenMode::Read);
if (!file.IsOpen()) {
    // Handle error: file doesn't exist or can't be opened
    std::cerr << "Failed to open file" << std::endl;
    return;
}

// Proceed with file operations...
size_t bytesRead = file.Read(buffer, size);
if (bytesRead == 0) {
    // Either end of file or read error
    if (file.IsEof()) {
        std::cout << "End of file reached" << std::endl;
    } else {
        std::cerr << "Read error" << std::endl;
    }
}
```

## Best Practices

1. **Always check if file is open**: Use `IsOpen()` before performing operations
2. **Use RAII**: Let the destructor handle closing for exception safety
3. **Flush important writes**: Call `Flush()` after critical writes
4. **Prefer Path objects**: Use `Path` class instead of raw strings
5. **Choose appropriate mode**: Select the correct `OpenMode` for your use case
6. **Handle read failures**: Check return values from read operations
7. **Use move semantics**: Take advantage of move operations for efficiency

## Platform-Specific Behavior

### Asset Files
- **Android**: Loaded from APK assets via `AAssetManager`
- **iOS**: Loaded from app bundle resources
- **Desktop**: Loaded from `assets/` directory relative to executable

### External Files
- All platforms use standard C `FILE*` operations
- Paths are resolved using the platform's file system
- Use `FileSystem::GetExternalDataPath()` for platform-appropriate data directories

## Related Classes

- **Path**: Cross-platform path handling
- **FileSystem**: Static utility functions for file operations
  - `ReadAssetText()` / `ReadAssetBinary()`
  - `ReadExternalText()` / `ReadExternalBinary()`
  - `WriteExternalText()` / `WriteExternalBinary()`
  - Various directory path getters

## Migration from FileSystem

If you're using the static `FileSystem` methods, you can migrate to the `File` class:

```cpp
// Old approach
std::string shader = FileSystem::ReadAssetText("shader.vert");
std::string config = FileSystem::ReadExternalText("config.ini");

// New approach with File class
File shaderFile("shader.vert");
std::string shader = shaderFile.ReadAllText();

File configFile("config.ini", File::OpenMode::Read, false);
std::string config = configFile.ReadAllText();
```

The `File` class provides more control and better resource management, especially for:
- Sequential reading/writing
- Random access with seek operations
- Streaming large files in chunks
- Incremental file processing

## See Also

- [File System Documentation](FILE_SYSTEM.md)
- [Path Class Documentation](PATH.md)
- [File Example](../examples/file_example.cpp)
