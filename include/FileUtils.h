#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "path.h"
#include <vector>
#include <string>
#include <cstdint>

namespace ului {

/**
 * @brief Utility functions for common file operations
 * 
 * Provides convenient functions for loading entire files into memory
 * and saving memory buffers to files.
 */
namespace FileUtils {

    /**
     * @brief Load entire file into memory as binary data
     * @param path File path (can be asset or external file)
     * @param preferAsset If true, try to load as asset first (default: true)
     * @return Vector containing file data, empty if failed
     */
    std::vector<uint8_t> LoadFileToMemory(const Path& path, bool preferAsset = true);

    /**
     * @brief Load entire file into memory as text
     * @param path File path (can be asset or external file)
     * @param preferAsset If true, try to load as asset first (default: true)
     * @return String containing file text, empty if failed
     */
    std::string LoadFileToString(const Path& path, bool preferAsset = true);

    /**
     * @brief Save binary data to file
     * @param path File path (external file only)
     * @param data Data to save
     * @return true if successful, false otherwise
     */
    bool SaveMemoryToFile(const Path& path, const std::vector<uint8_t>& data);

    /**
     * @brief Save binary data to file (from raw buffer)
     * @param path File path (external file only)
     * @param data Pointer to data
     * @param size Size of data in bytes
     * @return true if successful, false otherwise
     */
    bool SaveMemoryToFile(const Path& path, const uint8_t* data, size_t size);

    /**
     * @brief Save string to file
     * @param path File path (external file only)
     * @param text Text to save
     * @return true if successful, false otherwise
     */
    bool SaveStringToFile(const Path& path, const std::string& text);

    /**
     * @brief Append binary data to file
     * @param path File path (external file only)
     * @param data Data to append
     * @return true if successful, false otherwise
     */
    bool AppendMemoryToFile(const Path& path, const std::vector<uint8_t>& data);

    /**
     * @brief Append string to file
     * @param path File path (external file only)
     * @param text Text to append
     * @return true if successful, false otherwise
     */
    bool AppendStringToFile(const Path& path, const std::string& text);

    /**
     * @brief Get file size
     * @param path File path (can be asset or external file)
     * @param preferAsset If true, try asset first (default: true)
     * @return File size in bytes, or -1 if failed
     */
    int64_t GetFileSize(const Path& path, bool preferAsset = true);

    /**
     * @brief Check if file exists
     * @param path File path (can be asset or external file)
     * @param preferAsset If true, try asset first (default: true)
     * @return true if file exists, false otherwise
     */
    bool FileExists(const Path& path, bool preferAsset = true);

    /**
     * @brief Read specific number of bytes from file
     * @param path File path (can be asset or external file)
     * @param offset Starting offset in file
     * @param length Number of bytes to read
     * @param preferAsset If true, try asset first (default: true)
     * @return Vector containing read data, empty if failed
     */
    std::vector<uint8_t> ReadFileRange(const Path& path, int64_t offset, size_t length, bool preferAsset = true);

    /**
     * @brief Copy file from source to destination
     * @param sourcePath Source file path
     * @param destPath Destination file path
     * @param preferAsset If true, try to load source as asset first (default: true)
     * @return true if successful, false otherwise
     */
    bool CopyFile(const Path& sourcePath, const Path& destPath, bool preferAsset = true);

} // namespace FileUtils

} // namespace ului

#endif // FILE_UTILS_H
