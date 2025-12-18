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

#ifdef __ANDROID__
    /**
     * @brief Set Android AAssetManager for asset access
     * @param assetManager Pointer to AAssetManager from android_app
     * 
     * This must be called on Android before accessing internal assets.
     * Typically called from android_main with state->activity->assetManager
     */
    static void SetAndroidAssetManager(void* assetManager);
#endif

private:
    static std::string s_assetPath;
    static bool s_initialized;
    
#ifdef __ANDROID__
    static void* s_assetManager;  // AAssetManager*
#endif
};

} // namespace ului

#endif // FILE_SYSTEM_H
