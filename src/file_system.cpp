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

// ===== Special Directory APIs =====

std::string FileSystem::GetTempDirectory() {
#ifdef __ANDROID__
    // Android cache directory - would need JNI to get proper path
    return "/data/local/tmp/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSString* tmpDir = NSTemporaryDirectory();
        return std::string([tmpDir UTF8String]);
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    DWORD result = GetTempPathA(MAX_PATH, path);
    if (result > 0 && result < MAX_PATH) {
        return std::string(path);
    }
    // Fallback to TEMP environment variable
    const char* temp = getenv("TEMP");
    if (temp) {
        std::string tempPath = temp;
        tempPath += "\\";
        return tempPath;
    }
    return ".\\temp\\";
    
#else
    // Linux/macOS
    const char* tmpdir = getenv("TMPDIR");
    if (tmpdir) {
        std::string path = tmpdir;
        if (path.back() != '/') path += '/';
        return path;
    }
    return "/tmp/";
#endif
}

std::string FileSystem::GetAppDataDirectory() {
#ifdef __ANDROID__
    // Android internal files directory
    return "/data/data/com.example.ului/files/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory,
                                                              NSUserDomainMask, YES);
        if ([paths count] > 0) {
            NSString* appSupportDir = [paths objectAtIndex:0];
            std::string path = [appSupportDir UTF8String];
            path += "/";
            return path;
        }
        return "";
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    // Get AppData\Roaming
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return "";
    
#elif defined(__APPLE__)
    // macOS
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/Library/Application Support/";
        return path;
    }
    return "";
    
#else
    // Linux
    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig) {
        std::string path = xdgConfig;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/.config/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetLocalAppDataDirectory() {
#ifdef __ANDROID__
    // Same as GetAppDataDirectory on Android
    return GetAppDataDirectory();
    
#elif TARGET_OS_IOS
    // Same as GetAppDataDirectory on iOS
    return GetAppDataDirectory();
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    // Get AppData\Local
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return "";
    
#elif defined(__APPLE__)
    // macOS - same as roaming
    return GetAppDataDirectory();
    
#else
    // Linux
    const char* xdgData = getenv("XDG_DATA_HOME");
    if (xdgData) {
        std::string path = xdgData;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/.local/share/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetPublicDocumentsDirectory() {
#ifdef __ANDROID__
    // Android external documents directory
    return "/sdcard/Documents/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                              NSUserDomainMask, YES);
        if ([paths count] > 0) {
            NSString* docsDir = [paths objectAtIndex:0];
            std::string path = [docsDir UTF8String];
            path += "/";
            return path;
        }
        return "";
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return "";
    
#else
    // Linux/macOS desktop
    const char* xdgDocs = getenv("XDG_DOCUMENTS_DIR");
    if (xdgDocs) {
        std::string path = xdgDocs;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/Documents/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetExternalStorageDirectory() {
#ifdef __ANDROID__
    // Android external storage app directory
    return "/sdcard/Android/data/com.example.ului/files/";
    
#elif TARGET_OS_IOS
    // Not applicable on iOS
    return "";
    
#else
    // Not applicable on desktop
    return "";
#endif
}

// ===== Common User Directories =====

std::string FileSystem::GetUserDocumentsDirectory() {
    return GetPublicDocumentsDirectory();
}

std::string FileSystem::GetUserPicturesDirectory() {
#ifdef __ANDROID__
    return "/sdcard/Pictures/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSPicturesDirectory,
                                                              NSUserDomainMask, YES);
        if ([paths count] > 0) {
            NSString* picsDir = [paths objectAtIndex:0];
            std::string path = [picsDir UTF8String];
            path += "/";
            return path;
        }
        return "";
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYPICTURES, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return "";
    
#else
    // Linux/macOS
    const char* xdgPics = getenv("XDG_PICTURES_DIR");
    if (xdgPics) {
        std::string path = xdgPics;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/Pictures/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetUserMusicDirectory() {
#ifdef __ANDROID__
    return "/sdcard/Music/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSMusicDirectory,
                                                              NSUserDomainMask, YES);
        if ([paths count] > 0) {
            NSString* musicDir = [paths objectAtIndex:0];
            std::string path = [musicDir UTF8String];
            path += "/";
            return path;
        }
        return "";
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYMUSIC, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return "";
    
#else
    // Linux/macOS
    const char* xdgMusic = getenv("XDG_MUSIC_DIR");
    if (xdgMusic) {
        std::string path = xdgMusic;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/Music/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetUserVideosDirectory() {
#ifdef __ANDROID__
    return "/sdcard/Movies/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSMoviesDirectory,
                                                              NSUserDomainMask, YES);
        if ([paths count] > 0) {
            NSString* moviesDir = [paths objectAtIndex:0];
            std::string path = [moviesDir UTF8String];
            path += "/";
            return path;
        }
        return "";
    }
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYVIDEO, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    return "";
    
#else
    // Linux/macOS
    const char* xdgVideos = getenv("XDG_VIDEOS_DIR");
    if (xdgVideos) {
        std::string path = xdgVideos;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/Videos/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetUserDownloadsDirectory() {
#ifdef __ANDROID__
    return "/sdcard/Download/";
    
#elif TARGET_OS_IOS
    @autoreleasepool {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory,
                                                              NSUserDomainMask, YES);
        if ([paths count] > 0) {
            NSString* downloadsDir = [paths objectAtIndex:0];
            std::string path = [downloadsDir UTF8String];
            path += "/";
            return path;
        }
        return "";
    }
    
#elif defined(_WIN32)
    // Windows Vista and later
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        std::string result = path;
        result += "\\Downloads\\";
        return result;
    }
    return "";
    
#else
    // Linux/macOS
    const char* xdgDownload = getenv("XDG_DOWNLOAD_DIR");
    if (xdgDownload) {
        std::string path = xdgDownload;
        if (path.back() != '/') path += '/';
        return path;
    }
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        path += "/Downloads/";
        return path;
    }
    return "";
#endif
}

std::string FileSystem::GetUserHomeDirectory() {
#ifdef __ANDROID__
    // Not applicable on Android
    return "";
    
#elif TARGET_OS_IOS
    // Not applicable on iOS (sandboxed)
    return "";
    
#elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        std::string result = path;
        result += "\\";
        return result;
    }
    const char* userprofile = getenv("USERPROFILE");
    if (userprofile) {
        std::string result = userprofile;
        result += "\\";
        return result;
    }
    return "";
    
#else
    // Linux/macOS
    const char* home = getenv("HOME");
    if (home) {
        std::string path = home;
        if (path.back() != '/') path += '/';
        return path;
    }
    // Fallback to getpwuid
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        std::string path = pw->pw_dir;
        if (path.back() != '/') path += '/';
        return path;
    }
    return "";
#endif
}

#ifdef __ANDROID__
void FileSystem::SetAndroidAssetManager(void* assetManager) {
    s_assetManager = assetManager;
    std::cout << "Android AAssetManager set" << std::endl;
}
#endif

} // namespace ului
