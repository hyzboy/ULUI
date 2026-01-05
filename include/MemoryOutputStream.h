#ifndef MEMORY_OUTPUT_STREAM_H
#define MEMORY_OUTPUT_STREAM_H

#include "OutputStream.h"
#include <vector>
#include <cstdint>

namespace ului {

/**
 * @brief Output stream that writes to a memory buffer
 * 
 * Provides byte-level writing to an in-memory byte array.
 * Buffer grows automatically as data is written.
 */
class MemoryOutputStream : public OutputStream {
public:
    /**
     * @brief Construct with default capacity
     */
    MemoryOutputStream();

    /**
     * @brief Construct with initial capacity
     * @param initialCapacity Initial buffer capacity in bytes
     */
    explicit MemoryOutputStream(size_t initialCapacity);

    /**
     * @brief Destructor
     */
    ~MemoryOutputStream() override;

    // Disable copy
    MemoryOutputStream(const MemoryOutputStream&) = delete;
    MemoryOutputStream& operator=(const MemoryOutputStream&) = delete;

    // Enable move
    MemoryOutputStream(MemoryOutputStream&& other) noexcept;
    MemoryOutputStream& operator=(MemoryOutputStream&& other) noexcept;

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
     * @brief Flush (no-op for memory streams)
     */
    void Flush() override;

    /**
     * @brief Close the stream (does not clear buffer)
     */
    void Close() override;

    /**
     * @brief Get the written data as a vector (copy)
     * @return Vector containing all written data
     */
    std::vector<uint8_t> ToByteArray() const;

    /**
     * @brief Get the written data as a vector (move)
     * @return Vector containing all written data (stream is cleared)
     */
    std::vector<uint8_t> ToByteArrayAndClear();

    /**
     * @brief Get pointer to internal buffer
     * @return Pointer to data
     */
    const uint8_t* GetData() const { return m_buffer.data(); }

    /**
     * @brief Get size of written data
     * @return Size in bytes
     */
    size_t GetSize() const { return m_buffer.size(); }

    /**
     * @brief Reset the buffer (clear all data)
     */
    void Reset();

    /**
     * @brief Reserve capacity in the buffer
     * @param capacity Capacity in bytes
     */
    void Reserve(size_t capacity);

private:
    std::vector<uint8_t> m_buffer;
    bool m_closed;
};

} // namespace ului

#endif // MEMORY_OUTPUT_STREAM_H
