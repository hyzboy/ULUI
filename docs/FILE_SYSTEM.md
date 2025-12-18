# FileSystem Abstraction Layer

## Overview

The ULUI FileSystem provides a cross-platform abstraction for file I/O operations, supporting two types of file access:

1. **Internal Assets (Read-Only)**: Platform-specific asset bundles
2. **External Files (Read-Write)**: Standard file system access

## Architecture

### Internal Assets

Internal assets are read-only files bundled with the application:

| Platform | Location | Access Method |
|----------|----------|---------------|
| **Android** | APK `assets/` directory | AAssetManager API |
| **iOS** | App bundle resources | NSBundle API |
| **Windows/Linux/macOS** | `assets/` subdirectory next to executable | Standard file I/O |

### External Files

External files use standard file system access with read-write capabilities:

| Platform | Default Location | Access Method |
|----------|-----------------|---------------|
| **Android** | External storage (`/sdcard/`) | Standard file I/O |
| **iOS** | Documents directory | Standard file I/O |
| **Windows** | My Documents | Standard file I/O |
| **Linux/macOS** | Home directory | Standard file I/O |

## Usage

### Initialization

```cpp
#include "file_system.h"
using namespace ului;

// Desktop platforms (automatic path detection)
FileSystem::Initialize();

// Or specify custom asset path
FileSystem::Initialize("my_assets/");

// Android (requires AAssetManager)
#ifdef __ANDROID__
FileSystem::Initialize(nullptr);
FileSystem::SetAndroidAssetManager(state->activity->assetManager);
#endif

// iOS (automatic)
FileSystem::Initialize();
```

### Reading Internal Assets

```cpp
// Read shader from assets
std::string vertexShader = FileSystem::ReadAssetText("shaders/triangle.vert");

// Read binary asset (e.g., texture)
std::vector<uint8_t> textureData = FileSystem::ReadAssetBinary("textures/logo.png");

// Check if asset exists
if (FileSystem::AssetExists("config.json")) {
    std::string config = FileSystem::ReadAssetText("config.json");
}
```

### Reading External Files

```cpp
// Read configuration file
std::string config = FileSystem::ReadExternalText("/path/to/config.txt");

// Read binary data
std::vector<uint8_t> data = FileSystem::ReadExternalBinary("/path/to/data.bin");

// Check if file exists
if (FileSystem::ExternalFileExists("/path/to/save.dat")) {
    auto saveData = FileSystem::ReadExternalBinary("/path/to/save.dat");
}
```

### Writing External Files

```cpp
// Write text file
std::string gameState = "level=5\nscore=1000";
FileSystem::WriteExternalText("/path/to/save.txt", gameState);

// Write binary file
std::vector<uint8_t> saveData = {0x01, 0x02, 0x03};
FileSystem::WriteExternalBinary("/path/to/save.dat", saveData);

// Delete file
FileSystem::DeleteExternalFile("/path/to/old_save.dat");
```

### Platform-Specific Paths

```cpp
// Get asset directory path
std::string assetPath = FileSystem::GetAssetPath();
std::cout << "Assets location: " << assetPath << std::endl;

// Get writable data directory
std::string dataPath = FileSystem::GetExternalDataPath();
std::cout << "Writable location: " << dataPath << std::endl;

// Use for save files
std::string savePath = dataPath + "savegame.dat";
FileSystem::WriteExternalBinary(savePath.c_str(), saveData);
```

### Cleanup

```cpp
// Shutdown when application exits
FileSystem::Shutdown();
```

## Platform-Specific Details

### Android

**Asset Access:**
```cpp
// In android_main
void android_main(android_app* state) {
    FileSystem::Initialize(nullptr);
    FileSystem::SetAndroidAssetManager(state->activity->assetManager);
    
    // Now can access APK assets
    std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");
}
```

**External Storage:**
- Requires `WRITE_EXTERNAL_STORAGE` permission in AndroidManifest.xml
- Use `GetExternalDataPath()` for app-specific storage
- Files in external storage persist after app uninstall (user data)

**Asset Packaging:**
- Place files in `android/app/src/main/assets/`
- Build script automatically copies to all example projects
- Assets are compressed in APK

### iOS

**Asset Access:**
```cpp
// Automatic - uses NSBundle
FileSystem::Initialize();

// Access bundle resources
std::string config = FileSystem::ReadAssetText("config.plist");
```

**External Storage:**
- Uses Documents directory (backed up by iTunes/iCloud)
- No special permissions needed for Documents directory
- Use `GetExternalDataPath()` for save files

**Asset Packaging:**
- Add files to Xcode project
- Mark as "Copy Bundle Resources"
- Assets are in app bundle

### Desktop (Windows/Linux/macOS)

**Asset Access:**
```cpp
// Uses relative path from executable
FileSystem::Initialize("assets/");  // Default

// Assets in: executable_dir/assets/
std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");
```

**External Storage:**
- Full file system access
- No special permissions needed
- Use absolute or relative paths

**Asset Organization:**
```
my_app/
├── my_app.exe          (or my_app on Linux/macOS)
└── assets/
    ├── shaders/
    │   ├── triangle.vert
    │   └── triangle.frag
    ├── textures/
    └── config.json
```

## Error Handling

All read/write operations return empty results on failure with error messages printed to `std::cerr`:

```cpp
std::string data = FileSystem::ReadAssetText("missing.txt");
if (data.empty()) {
    std::cerr << "Failed to load asset" << std::endl;
    // Handle error
}

bool success = FileSystem::WriteExternalText("save.txt", data);
if (!success) {
    std::cerr << "Failed to write file" << std::endl;
    // Handle error
}
```

## Best Practices

### 1. Use Internal Assets for Read-Only Data

✅ **Good:**
```cpp
// Shaders, textures, configs from assets
std::string shader = FileSystem::ReadAssetText("shaders/triangle.vert");
```

❌ **Avoid:**
```cpp
// Don't try to write to assets
FileSystem::WriteExternalText("assets/shader.vert", modifiedShader); // Wrong!
```

### 2. Use External Files for User Data

✅ **Good:**
```cpp
// Save game state to writable location
std::string savePath = FileSystem::GetExternalDataPath() + "save.dat";
FileSystem::WriteExternalBinary(savePath.c_str(), saveData);
```

### 3. Check File Existence Before Reading

```cpp
if (FileSystem::ExternalFileExists(savePath.c_str())) {
    auto data = FileSystem::ReadExternalBinary(savePath.c_str());
} else {
    // Create default save
}
```

### 4. Initialize Early

```cpp
int main() {
    FileSystem::Initialize();  // Before accessing any files
    
    // ... rest of initialization
}
```

### 5. Platform-Specific Paths

```cpp
#ifdef __ANDROID__
    // Use getExternalFilesDir() path
    std::string savePath = FileSystem::GetExternalDataPath() + "saves/";
#elif TARGET_OS_IOS
    // Use Documents directory
    std::string savePath = FileSystem::GetExternalDataPath() + "saves/";
#else
    // Desktop - use relative or absolute path
    std::string savePath = "./saves/";
#endif
```

## Example: Loading Shaders

```cpp
#include "file_system.h"

bool loadShaders() {
    // Read from internal assets (cross-platform)
    std::string vertexSource = FileSystem::ReadAssetText("shaders/triangle.vert");
    std::string fragmentSource = FileSystem::ReadAssetText("shaders/triangle.frag");
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Failed to load shaders" << std::endl;
        return false;
    }
    
    // Compile shaders...
    return true;
}
```

## Example: Save/Load Game State

```cpp
#include "file_system.h"
#include <nlohmann/json.hpp>  // Example JSON library

void saveGameState(const GameState& state) {
    // Convert state to JSON
    nlohmann::json j;
    j["level"] = state.level;
    j["score"] = state.score;
    
    std::string json = j.dump();
    
    // Write to external storage
    std::string savePath = FileSystem::GetExternalDataPath() + "save.json";
    if (!FileSystem::WriteExternalText(savePath.c_str(), json)) {
        std::cerr << "Failed to save game state" << std::endl;
    }
}

GameState loadGameState() {
    std::string savePath = FileSystem::GetExternalDataPath() + "save.json";
    
    if (!FileSystem::ExternalFileExists(savePath.c_str())) {
        return GameState{};  // Return default state
    }
    
    std::string json = FileSystem::ReadExternalText(savePath.c_str());
    if (json.empty()) {
        return GameState{};
    }
    
    // Parse JSON
    auto j = nlohmann::json::parse(json);
    GameState state;
    state.level = j["level"];
    state.score = j["score"];
    return state;
}
```

## API Reference

See [`include/file_system.h`](../include/file_system.h) for complete API documentation.

### Key Functions

- `Initialize(assetPath)` - Initialize file system
- `Shutdown()` - Cleanup resources
- `ReadAssetText(filename)` - Read text from internal assets
- `ReadAssetBinary(filename)` - Read binary from internal assets
- `ReadExternalText(filepath)` - Read text from external file
- `ReadExternalBinary(filepath)` - Read binary from external file
- `WriteExternalText(filepath, content)` - Write text to external file
- `WriteExternalBinary(filepath, data)` - Write binary to external file
- `AssetExists(filename)` - Check if internal asset exists
- `ExternalFileExists(filepath)` - Check if external file exists
- `DeleteExternalFile(filepath)` - Delete external file
- `GetAssetPath()` - Get internal assets directory
- `GetExternalDataPath()` - Get writable data directory

## Troubleshooting

### Assets Not Found (Desktop)

**Problem:** `Failed to open asset: shaders/triangle.vert`

**Solution:**
- Verify `assets/` directory exists next to executable
- Check CMake copied files: `build/bin/assets/shaders/`
- Verify initialization: `FileSystem::Initialize("assets/")`

### Assets Not Found (Android)

**Problem:** `AAssetManager not set`

**Solution:**
```cpp
FileSystem::SetAndroidAssetManager(state->activity->assetManager);
```

### Cannot Write Files

**Problem:** Write operations fail

**Solution:**
- Use `GetExternalDataPath()` for writable location
- Don't try to write to assets directory
- Check file permissions on mobile platforms

## Thread Safety

**Not thread-safe** - The FileSystem class uses static members and should be accessed from the main thread or protected with mutex if used from multiple threads.

```cpp
// If using from multiple threads:
static std::mutex g_fileSystemMutex;

void workerThread() {
    std::lock_guard<std::mutex> lock(g_fileSystemMutex);
    auto data = FileSystem::ReadAssetBinary("data.bin");
}
```
