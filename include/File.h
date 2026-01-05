#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "path.h"

namespace ului {

/**
 * @brief Unified file access class for both asset and external files
 * 
 * Provides standard file I/O operations (read, write, seek, tell) with support for:
 * - Reading from internal asset packages (read-only)
 * - Reading/writing external files (read-write)
 * 
 * The class automatically determines whether to access assets or external files
 * based on the file path and available file system capabilities.
 */
class File {
public:
    /**
     * @brief File open mode flags
     */
    enum class OpenMode {
        Read,           ///< Open for reading only
        Write,          ///< Open for writing only (truncate if exists)
        ReadWrite,      ///< Open for reading and writing
        Append          ///< Open for writing, append to end if exists
    };

    /**
     * @brief File seek origin
     */
    enum class SeekOrigin {
        Begin,          ///< Seek from beginning of file
        Current,        ///< Seek from current position
        End             ///< Seek from end of file
    };

    /**
     * @brief Default constructor - creates an unopened file
     */
    File();

    /**
     * @brief Constructor with path and open mode
     * @param path File path (can be asset or external file)
     * @param mode Open mode (default: Read)
     * @param preferAsset If true, try to open as asset first (default: true)
     * 
     * Attempts to open the file. Check IsOpen() to verify success.
     */
    explicit File(const Path& path, OpenMode mode = OpenMode::Read, bool preferAsset = true);

    /**
     * @brief Destructor - closes the file if open
     */
    ~File();

    // Disable copy (file handles are unique)
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    // Enable move
    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;

    // ===== File Operations =====

    /**
     * @brief Open a file with specified mode
     * @param path File path
     * @param mode Open mode
     * @param preferAsset If true, try to open as asset first (default: true)
     * @return true if successful, false otherwise
     * 
     * For read mode: tries asset first (if preferAsset=true), then external file
     * For write modes: only opens external files
     */
    bool Open(const Path& path, OpenMode mode = OpenMode::Read, bool preferAsset = true);

    /**
     * @brief Close the file
     */
    void Close();

    /**
     * @brief Check if file is currently open
     * @return true if file is open, false otherwise
     */
    bool IsOpen() const;

    /**
     * @brief Read data from file
     * @param buffer Buffer to read into
     * @param size Number of bytes to read
     * @return Number of bytes actually read
     */
    size_t Read(void* buffer, size_t size);

    /**
     * @brief Read data from file into vector
     * @param size Number of bytes to read
     * @return Vector containing read data (empty if failed)
     */
    std::vector<uint8_t> Read(size_t size);

    /**
     * @brief Read entire file contents
     * @return Vector containing all file data (empty if failed)
     */
    std::vector<uint8_t> ReadAll();

    /**
     * @brief Read entire file contents as text
     * @return String containing all file text (empty if failed)
     */
    std::string ReadAllText();

    /**
     * @brief Write data to file
     * @param buffer Buffer to write from
     * @param size Number of bytes to write
     * @return Number of bytes actually written
     */
    size_t Write(const void* buffer, size_t size);

    /**
     * @brief Write data from vector to file
     * @param data Data to write
     * @return Number of bytes actually written
     */
    size_t Write(const std::vector<uint8_t>& data);

    /**
     * @brief Write string to file
     * @param text Text to write
     * @return Number of bytes actually written
     */
    size_t Write(const std::string& text);

    /**
     * @brief Seek to position in file
     * @param offset Offset in bytes
     * @param origin Origin for seek operation
     * @return true if successful, false otherwise
     */
    bool Seek(int64_t offset, SeekOrigin origin = SeekOrigin::Begin);

    /**
     * @brief Get current position in file
     * @return Current position in bytes, or -1 if failed
     */
    int64_t Tell() const;

    /**
     * @brief Flush any buffered writes to disk
     * @return true if successful, false otherwise
     */
    bool Flush();

    // ===== File Properties =====

    /**
     * @brief Get total file length
     * @return File length in bytes, or -1 if failed
     */
    int64_t GetLength() const;

    /**
     * @brief Check if file can be read
     * @return true if file is open for reading
     */
    bool CanRead() const;

    /**
     * @brief Check if file can be written
     * @return true if file is open for writing
     */
    bool CanWrite() const;

    /**
     * @brief Check if file is an asset (read-only from package)
     * @return true if file is from asset package
     */
    bool IsAsset() const;

    /**
     * @brief Check if file is an external file
     * @return true if file is from external file system
     */
    bool IsExternal() const;

    /**
     * @brief Check if end of file has been reached
     * @return true if at end of file
     */
    bool IsEof() const;

    /**
     * @brief Get the file path
     * @return File path
     */
    const Path& GetPath() const;

private:
    Path m_path;
    OpenMode m_mode;
    bool m_isOpen;
    bool m_isAsset;
    int64_t m_position;    // Current position for asset files
    
    // For external files, use FILE*
    void* m_fileHandle;    // FILE* for external files
    
    // For asset files, cache the entire content
    std::vector<uint8_t> m_assetData;
    
    /**
     * @brief Try to open as asset file
     * @param path File path
     * @return true if successful
     */
    bool OpenAsAsset(const Path& path);
    
    /**
     * @brief Try to open as external file
     * @param path File path
     * @param mode Open mode
     * @return true if successful
     */
    bool OpenAsExternal(const Path& path, OpenMode mode);
};

} // namespace ului

#endif // FILE_H
