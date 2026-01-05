#ifndef FILE_INPUT_STREAM_H
#define FILE_INPUT_STREAM_H

#include "InputStream.h"
#include "path.h"
#include <vector>

namespace ului {

/**
 * @brief Input stream for reading from files
 * 
 * Similar to Java's FileInputStream, provides byte-level reading from files.
 * Supports both asset files (read-only) and external files.
 */
class FileInputStream : public InputStream {
public:
    /**
     * @brief Construct from file path
     * @param path File path
     * @param preferAsset If true, try to open as asset first (default: true)
     */
    explicit FileInputStream(const Path& path, bool preferAsset = true);

    /**
     * @brief Destructor - closes the stream
     */
    ~FileInputStream() override;

    // Disable copy
    FileInputStream(const FileInputStream&) = delete;
    FileInputStream& operator=(const FileInputStream&) = delete;

    // Enable move
    FileInputStream(FileInputStream&& other) noexcept;
    FileInputStream& operator=(FileInputStream&& other) noexcept;

    /**
     * @brief Read a single byte
     * @return The byte read (0-255), or -1 if end of stream
     */
    int Read() override;

    /**
     * @brief Read bytes into a buffer
     * @param buffer Buffer to read into
     * @param offset Starting offset in buffer
     * @param length Maximum number of bytes to read
     * @return Number of bytes actually read, or -1 if end of stream
     */
    int Read(uint8_t* buffer, size_t offset, size_t length) override;

    /**
     * @brief Read bytes into a buffer (simplified version)
     * @param buffer Buffer to read into
     * @param length Maximum number of bytes to read
     * @return Number of bytes actually read, or -1 if end of stream
     */
    int Read(uint8_t* buffer, size_t length) {
        return Read(buffer, 0, length);
    }

    /**
     * @brief Get number of bytes available for reading
     * @return Number of available bytes
     */
    int Available() override;

    /**
     * @brief Close the stream
     */
    void Close() override;

    /**
     * @brief Check if stream is open
     * @return true if open
     */
    bool IsOpen() const { return m_isOpen; }

private:
    Path m_path;
    bool m_isOpen;
    bool m_isAsset;
    int64_t m_position;
    void* m_fileHandle;  // FILE* for external files
    std::vector<uint8_t> m_assetData;  // Cached data for assets

    bool OpenAsAsset(const Path& path);
    bool OpenAsExternal(const Path& path);
};

} // namespace ului

#endif // FILE_INPUT_STREAM_H
