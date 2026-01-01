#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>
#include <vector>
#include <memory>

namespace ului {

/**
 * @brief Cross-platform file I/O abstraction layer
 * 
 * Provides unified interface for reading files from:
 * - Internal assets (read-only): Platform-specific asset bundles
 * - External files (read-write): Standard file system
 */
class FileSystem {
public:
    /**
     * @brief Initialize the file system
     * @param assetPath Path to internal assets directory (platform-specific)
     * 
     * Platform-specific behavior:
     * - Android: Uses AAssetManager for internal assets
     * - iOS: Uses NSBundle for internal assets
     * - Desktop: Uses relative path from executable
     */
    static void Initialize(const char* assetPath = nullptr);
    
    /**
     * @brief Shutdown and cleanup file system resources
     */
    static void Shutdown();
    
    // ===== Internal Assets (Read-Only) =====
    
    /**
     * @brief Read entire internal asset file to string
     * @param filename Path relative to asset directory
     * @return File contents as string, empty if failed
     * 
     * Internal assets are read-only and located in:
     * - Android: APK assets/
     * - iOS: App bundle resources
     * - Desktop: assets/ subdirectory next to executable
     */
    static std::string ReadAssetText(const char* filename);
    
    /**
     * @brief Read entire internal asset file to binary buffer
     * @param filename Path relative to asset directory
     * @return File contents as byte vector, empty if failed
     */
    static std::vector<uint8_t> ReadAssetBinary(const char* filename);
    
    /**
     * @brief Check if internal asset exists
     * @param filename Path relative to asset directory
     * @return true if asset exists, false otherwise
     */
    static bool AssetExists(const char* filename);
    
    // ===== External Files (Read-Write) =====
    
    /**
     * @brief Read entire external file to string
     * @param filepath Absolute or relative path to file
     * @return File contents as string, empty if failed
     * 
     * External files use standard file system access
     */
    static std::string ReadExternalText(const char* filepath);
    
    /**
     * @brief Read entire external file to binary buffer
     * @param filepath Absolute or relative path to file
     * @return File contents as byte vector, empty if failed
     */
    static std::vector<uint8_t> ReadExternalBinary(const char* filepath);
    
    /**
     * @brief Write text to external file
     * @param filepath Absolute or relative path to file
     * @param content Text content to write
     * @return true if successful, false otherwise
     */
    static bool WriteExternalText(const char* filepath, const std::string& content);
    
    /**
     * @brief Write binary data to external file
     * @param filepath Absolute or relative path to file
     * @param data Binary data to write
     * @return true if successful, false otherwise
     */
    static bool WriteExternalBinary(const char* filepath, const std::vector<uint8_t>& data);
    
    /**
     * @brief Check if external file exists
     * @param filepath Absolute or relative path to file
     * @return true if file exists, false otherwise
     */
    static bool ExternalFileExists(const char* filepath);
    
    /**
     * @brief Delete external file
     * @param filepath Absolute or relative path to file
     * @return true if successful, false otherwise
     */
    static bool DeleteExternalFile(const char* filepath);
    
    // ===== Utility Functions =====
    
    /**
     * @brief Get the path to internal assets directory
     * @return Platform-specific assets directory path
     */
    static std::string GetAssetPath();
    
    /**
     * @brief Get platform-specific writable directory for external files
     * @return Platform-specific documents/data directory
     * 
     * Returns:
     * - Android: getExternalFilesDir()
     * - iOS: Documents directory
     * - Desktop: Current working directory or user documents
     */
    static std::string GetExternalDataPath();
    
    // ===== Special Directory APIs =====
    
    /**
     * @brief Get temporary directory for application temporary files
     * @return Platform-specific temporary directory
     * 
     * Returns:
     * - Windows: %LOCALAPPDATA%\Temp or %TEMP%
     * - Linux/macOS: /tmp or $TMPDIR
     * - Android: Context.getCacheDir()
     * - iOS: NSTemporaryDirectory()
     */
    static std::string GetTempDirectory();
    
    /**
     * @brief Get persistent application data directory (roaming/synced)
     * @return Platform-specific persistent app data directory
     * 
     * Returns:
     * - Windows: %APPDATA% (Roaming profile)
     * - Linux: $XDG_CONFIG_HOME or ~/.config
     * - macOS: ~/Library/Application Support
     * - Android: Context.getFilesDir()
     * - iOS: Application Support directory
     */
    static std::string GetAppDataDirectory();
    
    /**
     * @brief Get local persistent application data directory (non-roaming)
     * @return Platform-specific local app data directory
     * 
     * Returns:
     * - Windows: %LOCALAPPDATA%
     * - Linux: $XDG_DATA_HOME or ~/.local/share
     * - macOS: ~/Library/Application Support
     * - Android: Same as GetAppDataDirectory()
     * - iOS: Same as GetAppDataDirectory()
     */
    static std::string GetLocalAppDataDirectory();
    
    /**
     * @brief Get public persistent directory for user-visible data
     * @return Platform-specific public documents directory
     * 
     * User-visible directory for configs, mods, DLC, etc.
     * Returns:
     * - Windows: %USERPROFILE%\Documents or My Documents
     * - Linux: $XDG_DOCUMENTS_DIR or ~/Documents
     * - macOS: ~/Documents
     * - Android: getExternalFilesDir(DIRECTORY_DOCUMENTS)
     * - iOS: Documents directory (user-visible in Files app)
     */
    static std::string GetPublicDocumentsDirectory();
    
    /**
     * @brief Get external storage app data directory (mobile only)
     * @return Platform-specific external storage directory
     * 
     * Returns:
     * - Android: getExternalFilesDir(null) - SD card/external storage
     * - iOS: Not applicable, returns empty string
     * - Desktop: Not applicable, returns empty string
     */
    static std::string GetExternalStorageDirectory();
    
    // ===== Common User Directories =====
    
    /**
     * @brief Get user's Documents directory
     * @return Platform-specific Documents directory
     */
    static std::string GetUserDocumentsDirectory();
    
    /**
     * @brief Get user's Pictures/Photos directory
     * @return Platform-specific Pictures directory
     */
    static std::string GetUserPicturesDirectory();
    
    /**
     * @brief Get user's Music directory
     * @return Platform-specific Music directory
     */
    static std::string GetUserMusicDirectory();
    
    /**
     * @brief Get user's Videos directory
     * @return Platform-specific Videos directory
     */
    static std::string GetUserVideosDirectory();
    
    /**
     * @brief Get user's Downloads directory
     * @return Platform-specific Downloads directory
     */
    static std::string GetUserDownloadsDirectory();
    
    /**
     * @brief Get user's home directory
     * @return Platform-specific home directory
     * 
     * Returns:
     * - Windows: %USERPROFILE%
     * - Linux/macOS: $HOME or ~
     * - Android: Not applicable, returns empty string
     * - iOS: Not applicable, returns empty string
     */
    static std::string GetUserHomeDirectory();

#ifdef __ANDROID__
    /**
     * @brief Initialize FileSystem for Android platform
     * @param assetManager Pointer to AAssetManager from android_app
     * @param packageName The Android application package name (e.g., "com.example.myapp")
     * 
     * This is the recommended way to initialize FileSystem on Android.
     * It sets both the asset manager and package name in one call.
     * 
     * Example usage in android_main:
     * @code
     * FileSystem::InitializeAndroid(state->activity->assetManager, "com.example.myapp");
     * @endcode
     */
    static void InitializeAndroid(void* assetManager, const char* packageName);
    
    /**
     * @brief Set Android AAssetManager for asset access
     * @param assetManager Pointer to AAssetManager from android_app
     * 
     * This must be called on Android before accessing internal assets.
     * Typically called from android_main with state->activity->assetManager
     */
    static void SetAndroidAssetManager(void* assetManager);
    
    /**
     * @brief Set Android package name for path composition
     * @param packageName The Android application package name (e.g., "com.example.myapp")
     * 
     * This should be called on Android to set the package name used for composing
     * app-specific directories. If not set, a default package name will be used.
     */
    static void SetAndroidPackageName(const char* packageName);
#endif

private:
    static std::string s_assetPath;
    static bool s_initialized;
    
#ifdef __ANDROID__
    static void* s_assetManager;  // AAssetManager*
    static std::string s_packageName;  // Android package name
#endif
};

} // namespace ului

#endif // FILE_SYSTEM_H
