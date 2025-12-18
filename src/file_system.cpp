#include "file_system.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IOS
#include <Foundation/Foundation.h>
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

namespace ului {

// Static member initialization
std::string FileSystem::s_assetPath;
bool FileSystem::s_initialized = false;

#ifdef __ANDROID__
void* FileSystem::s_assetManager = nullptr;
#endif

void FileSystem::Initialize(const char* assetPath) {
    if (s_initialized) {
        return;
    }
    
#ifdef __ANDROID__
    // On Android, assetPath should be nullptr, we'll get AAssetManager from android_app
    // This will be set by the main application
    s_assetPath = ""; // Assets accessed via AAssetManager
#elif TARGET_OS_IOS
    // On iOS, get the resource bundle path
    @autoreleasepool {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* resourcePath = [mainBundle resourcePath];
        s_assetPath = [resourcePath UTF8String];
        s_assetPath += "/";
    }
#else
    // On desktop platforms, use provided path or default
    if (assetPath && assetPath[0] != '\0') {
        s_assetPath = assetPath;
    } else {
        // Default to "assets" directory relative to executable
        s_assetPath = "assets/";
    }
#endif
    
    s_initialized = true;
    std::cout << "FileSystem initialized. Asset path: " << s_assetPath << std::endl;
}

void FileSystem::Shutdown() {
    s_initialized = false;
#ifdef __ANDROID__
    s_assetManager = nullptr;
#endif
}

// ===== Internal Assets (Read-Only) =====

std::string FileSystem::ReadAssetText(const char* filename) {
    auto data = ReadAssetBinary(filename);
    if (data.empty()) {
        return "";
    }
    return std::string(data.begin(), data.end());
}

std::vector<uint8_t> FileSystem::ReadAssetBinary(const char* filename) {
    if (!s_initialized) {
        Initialize();
    }
    
#ifdef __ANDROID__
    if (!s_assetManager) {
        std::cerr << "AAssetManager not set. Call SetAssetManager() first." << std::endl;
        return {};
    }
    
    AAssetManager* mgr = static_cast<AAssetManager*>(s_assetManager);
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
    
    if (!asset) {
        std::cerr << "Failed to open asset: " << filename << std::endl;
        return {};
    }
    
    off_t length = AAsset_getLength(asset);
    std::vector<uint8_t> buffer(length);
    
    int result = AAsset_read(asset, buffer.data(), length);
    AAsset_close(asset);
    
    if (result < 0 || result != length) {
        std::cerr << "Failed to read asset: " << filename << std::endl;
        return {};
    }
    
    return buffer;
    
#elif TARGET_OS_IOS
    // On iOS, assets are in the app bundle
    @autoreleasepool {
        NSString* nsFilename = [NSString stringWithUTF8String:filename];
        NSString* path = [[NSBundle mainBundle] pathForResource:nsFilename ofType:nil];
        
        if (!path) {
            std::cerr << "Failed to find asset: " << filename << std::endl;
            return {};
        }
        
        NSData* data = [NSData dataWithContentsOfFile:path];
        if (!data) {
            std::cerr << "Failed to read asset: " << filename << std::endl;
            return {};
        }
        
        const uint8_t* bytes = static_cast<const uint8_t*>([data bytes]);
        return std::vector<uint8_t>(bytes, bytes + [data length]);
    }
    
#else
    // On desktop, read from assets directory
    std::string fullPath = s_assetPath + filename;
    return ReadExternalBinary(fullPath.c_str());
#endif
}

bool FileSystem::AssetExists(const char* filename) {
    if (!s_initialized) {
        Initialize();
    }
    
#ifdef __ANDROID__
    if (!s_assetManager) {
        return false;
    }
    
    AAssetManager* mgr = static_cast<AAssetManager*>(s_assetManager);
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_UNKNOWN);
    
    if (asset) {
        AAsset_close(asset);
        return true;
    }
    return false;
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSString* nsFilename = [NSString stringWithUTF8String:filename];
        NSString* path = [[NSBundle mainBundle] pathForResource:nsFilename ofType:nil];
        return path != nil;
    }
    
#else
    std::string fullPath = s_assetPath + filename;
    return ExternalFileExists(fullPath.c_str());
#endif
}

// ===== External Files (Read-Write) =====

std::string FileSystem::ReadExternalText(const char* filepath) {
    auto data = ReadExternalBinary(filepath);
    if (data.empty()) {
        return "";
    }
    return std::string(data.begin(), data.end());
}

std::vector<uint8_t> FileSystem::ReadExternalBinary(const char* filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read file: " << filepath << std::endl;
        return {};
    }
    
    return buffer;
}

bool FileSystem::WriteExternalText(const char* filepath, const std::string& content) {
    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    file << content;
    return file.good();
}

bool FileSystem::WriteExternalBinary(const char* filepath, const std::vector<uint8_t>& data) {
    std::ofstream file(filepath, std::ios::binary | std::ios::out | std::ios::trunc);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

bool FileSystem::ExternalFileExists(const char* filepath) {
    std::ifstream file(filepath);
    return file.good();
}

bool FileSystem::DeleteExternalFile(const char* filepath) {
    return std::remove(filepath) == 0;
}

// ===== Utility Functions =====

std::string FileSystem::GetAssetPath() {
    if (!s_initialized) {
        Initialize();
    }
    return s_assetPath;
}

std::string FileSystem::GetExternalDataPath() {
#ifdef __ANDROID__
    // On Android, this would typically be set by JNI
    // For now, return a placeholder that can be set by the app
    return "/sdcard/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, 
                                                              NSUserDomainMask, YES);
        NSString* documentsDirectory = [paths objectAtIndex:0];
        std::string path = [documentsDirectory UTF8String];
        path += "/";
        return path;
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return ".\\";
    
#else
    // Linux/macOS desktop
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/";
        return path;
    }
    return "./";
#endif
}

#ifdef __ANDROID__
void FileSystem::SetAndroidAssetManager(void* assetManager) {
    s_assetManager = assetManager;
    std::cout << "Android AAssetManager set" << std::endl;
}
#endif

} // namespace ului
