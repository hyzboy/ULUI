#ifndef MEMORY_INPUT_STREAM_H
#define MEMORY_INPUT_STREAM_H

#include "InputStream.h"
#include <vector>
#include <cstdint>

namespace ului {

/**
 * @brief Input stream that reads from a memory buffer
 * 
 * Provides byte-level reading from an in-memory byte array.
 * Useful for reading data from memory without file I/O.
 */
class MemoryInputStream : public InputStream {
public:
    /**
     * @brief Construct from a byte vector (copies data)
     * @param data Data to read from
     */
    explicit MemoryInputStream(const std::vector<uint8_t>& data);

    /**
     * @brief Construct from a byte vector (moves data)
     * @param data Data to read from
     */
    explicit MemoryInputStream(std::vector<uint8_t>&& data);

    /**
     * @brief Construct from raw buffer (copies data)
     * @param data Pointer to data
     * @param length Length of data in bytes
     */
    MemoryInputStream(const uint8_t* data, size_t length);

    /**
     * @brief Destructor
     */
    ~MemoryInputStream() override;

    // Disable copy
    MemoryInputStream(const MemoryInputStream&) = delete;
    MemoryInputStream& operator=(const MemoryInputStream&) = delete;

    // Enable move
    MemoryInputStream(MemoryInputStream&& other) noexcept;
    MemoryInputStream& operator=(MemoryInputStream&& other) noexcept;

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
     * @brief Close the stream (clears buffer)
     */
    void Close() override;

    /**
     * @brief Check if mark/reset is supported
     * @return true (memory streams support mark/reset)
     */
    bool MarkSupported() const override;

    /**
     * @brief Mark current position in stream
     * @param readlimit Not used for memory streams (can read any amount)
     */
    void Mark(int readlimit) override;

    /**
     * @brief Reset stream to last marked position
     */
    void Reset() override;

    /**
     * @brief Get current position in stream
     * @return Current position in bytes
     */
    size_t GetPosition() const { return m_position; }

    /**
     * @brief Get total size of the buffer
     * @return Size in bytes
     */
    size_t GetSize() const { return m_data.size(); }

private:
    std::vector<uint8_t> m_data;
    size_t m_position;
    size_t m_markPosition;
};

} // namespace ului

#endif // MEMORY_INPUT_STREAM_H
