# Java-Style I/O Streams Documentation

## Overview

ULUI provides a complete Java-style I/O stream hierarchy for byte-level and typed data I/O operations. This familiar API makes it easy for Java developers to work with files and data streams in C++.

## Stream Hierarchy

```
InputStream (abstract)
├── FileInputStream (reads from assets or external files)
└── [wrapped by] DataInputStream (typed data reading)

OutputStream (abstract)
├── FileOutputStream (writes to external files)
└── [wrapped by] DataOutputStream (typed data writing)
```

## Base Stream Classes

### InputStream

Abstract base class for reading bytes.

```cpp
class InputStream {
    virtual int Read() = 0;  // Read single byte
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

Abstract base class for writing bytes.

```cpp
class OutputStream {
    virtual void Write(int b) = 0;  // Write single byte
    virtual void Write(const uint8_t* buffer, size_t offset, size_t length) = 0;
    virtual void Flush() = 0;
    virtual void Close() = 0;
};
```

## File Stream Classes

### FileInputStream

Reads bytes from files (supports both assets and external files).

```cpp
#include "FileInputStream.h"

// Read from asset file
FileInputStream fis(Path("shaders/vertex.glsl"), true);
if (fis.IsOpen()) {
    int b = fis.Read();  // Read single byte
    
    uint8_t buffer[1024];
    int bytesRead = fis.Read(buffer, sizeof(buffer));
    
    int available = fis.Available();
    fis.Close();
}
```

**Constructor:**
- `FileInputStream(const Path& path, bool preferAsset = true)`
  - `preferAsset = true`: Try asset first, fallback to external
  - `preferAsset = false`: Try external first, fallback to asset

### FileOutputStream

Writes bytes to external files.

```cpp
#include "FileOutputStream.h"

// Create new file (truncate if exists)
FileOutputStream fos(Path("output.dat"), false);
if (fos.IsOpen()) {
    fos.Write('A');  // Write single byte
    
    uint8_t data[] = {1, 2, 3, 4, 5};
    fos.Write(data, sizeof(data));
    
    fos.Flush();
    fos.Close();
}

// Append to existing file
FileOutputStream fos2(Path("log.txt"), true);
```

**Constructor:**
- `FileOutputStream(const Path& path, bool append = false)`
  - `append = false`: Create new file (truncate if exists)
  - `append = true`: Append to existing file

## Data Stream Classes

### DataInputStream

Reads primitive data types in big-endian byte order.

```cpp
#include "DataInputStream.h"
#include "FileInputStream.h"

auto fis = std::make_unique<FileInputStream>(Path("data.bin"), false);
DataInputStream dis(std::move(fis));

// Read primitive types
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

**Methods:**
- `ReadBoolean()` - 1 byte
- `ReadByte()` - Signed 8-bit
- `ReadUnsignedByte()` - Unsigned 8-bit
- `ReadShort()` - Signed 16-bit (big-endian)
- `ReadUnsignedShort()` - Unsigned 16-bit (big-endian)
- `ReadInt()` - Signed 32-bit (big-endian)
- `ReadLong()` - Signed 64-bit (big-endian)
- `ReadFloat()` - 32-bit IEEE 754 (big-endian)
- `ReadDouble()` - 64-bit IEEE 754 (big-endian)
- `ReadUTF()` - UTF-8 string (2-byte length + UTF-8 bytes)
- `ReadFully(buffer, length)` - Read exact number of bytes

### DataOutputStream

Writes primitive data types in big-endian byte order.

```cpp
#include "DataOutputStream.h"
#include "FileOutputStream.h"

auto fos = std::make_unique<FileOutputStream>(Path("data.bin"), false);
DataOutputStream dos(std::move(fos));

// Write primitive types
dos.WriteBoolean(true);
dos.WriteByte(127);
dos.WriteShort(32000);
dos.WriteInt(1234567890);
dos.WriteLong(9876543210LL);
dos.WriteFloat(3.14159f);
dos.WriteDouble(2.718281828459);
dos.WriteUTF("Hello, World!");

size_t bytesWritten = dos.GetBytesWritten();
dos.Flush();
dos.Close();
```

**Methods:**
- `WriteBoolean(bool)` - 1 byte
- `WriteByte(int8_t)` - 1 byte
- `WriteShort(int16_t)` - 2 bytes (big-endian)
- `WriteInt(int32_t)` - 4 bytes (big-endian)
- `WriteLong(int64_t)` - 8 bytes (big-endian)
- `WriteFloat(float)` - 4 bytes (IEEE 754, big-endian)
- `WriteDouble(double)` - 8 bytes (IEEE 754, big-endian)
- `WriteUTF(const std::string&)` - UTF-8 string (2-byte length + UTF-8 bytes, max 65535 bytes)
- `GetBytesWritten()` - Total bytes written

## Stream Composition

Wrap file streams with data streams for typed I/O:

```cpp
// Write structured data
{
    auto fos = std::make_unique<FileOutputStream>(Path("player.dat"), false);
    DataOutputStream dos(std::move(fos));
    
    dos.WriteUTF("PlayerOne");
    dos.WriteInt(25);         // level
    dos.WriteInt(9850);       // experience
    dos.WriteFloat(87.5f);    // health
    dos.WriteBoolean(true);   // hasShield
    
    dos.Close();
}

// Read structured data
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

## Advanced Examples

### Buffered Reading

```cpp
FileInputStream fis(Path("large_file.dat"), false);
uint8_t buffer[4096];

while (true) {
    int bytesRead = fis.Read(buffer, sizeof(buffer));
    if (bytesRead <= 0) break;
    
    // Process buffer...
}

fis.Close();
```

### Binary Protocol

```cpp
// Write binary protocol
auto fos = std::make_unique<FileOutputStream>(Path("protocol.bin"), false);
DataOutputStream dos(std::move(fos));

dos.WriteByte(0x01);  // Version
dos.WriteShort(100);  // Message ID
dos.WriteInt(1024);   // Payload length
// Write payload...

dos.Close();

// Read binary protocol
auto fis = std::make_unique<FileInputStream>(Path("protocol.bin"), false);
DataInputStream dis(std::move(fis));

uint8_t version = dis.ReadUnsignedByte();
uint16_t messageId = dis.ReadUnsignedShort();
uint32_t payloadLength = dis.ReadInt();
// Read payload...

dis.Close();
```

### Custom Stream

You can implement custom streams by inheriting from InputStream/OutputStream:

```cpp
class MemoryInputStream : public InputStream {
private:
    std::vector<uint8_t> m_data;
    size_t m_position;

public:
    MemoryInputStream(std::vector<uint8_t> data)
        : m_data(std::move(data)), m_position(0) {}
    
    int Read() override {
        if (m_position >= m_data.size()) return -1;
        return m_data[m_position++];
    }
    
    int Read(uint8_t* buffer, size_t offset, size_t length) override {
        size_t available = m_data.size() - m_position;
        size_t toRead = std::min(length, available);
        if (toRead == 0) return -1;
        
        std::copy(m_data.begin() + m_position,
                  m_data.begin() + m_position + toRead,
                  buffer + offset);
        m_position += toRead;
        return static_cast<int>(toRead);
    }
    
    void Close() override {
        m_data.clear();
        m_position = 0;
    }
};
```

## Error Handling

Data streams throw exceptions on errors:

```cpp
try {
    auto fis = std::make_unique<FileInputStream>(Path("data.bin"), false);
    DataInputStream dis(std::move(fis));
    
    int value = dis.ReadInt();
    
} catch (const std::runtime_error& e) {
    std::cerr << "Stream error: " << e.what() << std::endl;
} catch (const std::invalid_argument& e) {
    std::cerr << "Invalid argument: " << e.what() << std::endl;
}
```

## Byte Order

All multi-byte data types use **big-endian** byte order for cross-platform compatibility:
- Matches Java's DataInputStream/DataOutputStream
- Network byte order (most significant byte first)
- Portable across different architectures

## Comparison with File Class

| Feature | File Class | Stream Classes |
|---------|------------|----------------|
| API Style | C-style (fopen/fread) | Java-style OOP |
| Type Safety | Raw bytes | Typed data |
| Composition | No | Yes (wrap streams) |
| Inheritance | No | Yes (extend base) |
| Error Handling | Return codes | Exceptions |
| Use Case | Simple file I/O | Complex protocols |

## Best Practices

1. **Use smart pointers** for stream composition:
   ```cpp
   auto fis = std::make_unique<FileInputStream>(path, false);
   DataInputStream dis(std::move(fis));
   ```

2. **Always close streams** (or use RAII):
   ```cpp
   {
       FileInputStream fis(path);
       // Use stream...
   } // Automatically closed
   ```

3. **Handle exceptions** for data streams:
   ```cpp
   try {
       value = dis.ReadInt();
   } catch (const std::exception& e) {
       // Handle error
   }
   ```

4. **Check IsOpen()** for file streams:
   ```cpp
   FileInputStream fis(path);
   if (!fis.IsOpen()) {
       // Handle error
       return;
   }
   ```

5. **Flush after critical writes**:
   ```cpp
   dos.WriteInt(importantData);
   dos.Flush();  // Ensure written to disk
   ```

## Platform Support

- **Windows**: Full support with _fseeki64/_ftelli64 for large files
- **Linux/macOS**: Full support with fseeko/ftello for large files
- **Android**: Asset loading via AAssetManager + external file support
- **iOS**: Bundle resource loading + external file support

## See Also

- [File Class Documentation](FILE_CLASS.md)
- [File System Documentation](FILE_SYSTEM.md)
- [Path Class Documentation](PATH.md)
- [Stream Example](../examples/streams_example.cpp)
