#ifndef FILE_OUTPUT_STREAM_H
#define FILE_OUTPUT_STREAM_H

#include "OutputStream.h"
#include "path.h"

namespace ului {

/**
 * @brief Output stream for writing to files
 * 
 * Similar to Java's FileOutputStream, provides byte-level writing to files.
 * Only supports external files (assets are read-only).
 */
class FileOutputStream : public OutputStream {
public:
    /**
     * @brief Construct from file path
     * @param path File path
     * @param append If true, append to existing file; if false, truncate (default: false)
     */
    explicit FileOutputStream(const Path& path, bool append = false);

    /**
     * @brief Destructor - closes the stream
     */
    ~FileOutputStream() override;

    // Disable copy
    FileOutputStream(const FileOutputStream&) = delete;
    FileOutputStream& operator=(const FileOutputStream&) = delete;

    // Enable move
    FileOutputStream(FileOutputStream&& other) noexcept;
    FileOutputStream& operator=(FileOutputStream&& other) noexcept;

    /**
     * @brief Write a single byte
     * @param b The byte to write (0-255)
     */
    void Write(int b) override;

    /**
     * @brief Write bytes from a buffer
     * @param buffer Buffer containing data to write
     * @param offset Starting offset in buffer
     * @param length Number of bytes to write
     */
    void Write(const uint8_t* buffer, size_t offset, size_t length) override;

    /**
     * @brief Write bytes from a buffer (simplified version)
     * @param buffer Buffer containing data to write
     * @param length Number of bytes to write
     */
    void Write(const uint8_t* buffer, size_t length) {
        Write(buffer, 0, length);
    }

    /**
     * @brief Flush buffered output
     */
    void Flush() override;

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
    void* m_fileHandle;  // FILE*
};

} // namespace ului

#endif // FILE_OUTPUT_STREAM_H
