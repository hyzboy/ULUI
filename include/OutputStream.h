#ifndef OUTPUT_STREAM_H
#define OUTPUT_STREAM_H

#include <cstdint>
#include <cstddef>

namespace ului {

/**
 * @brief Abstract base class for writing streams of bytes
 * 
 * Similar to Java's OutputStream, this class provides a basic interface
 * for writing bytes to various destinations.
 */
class OutputStream {
public:
    virtual ~OutputStream() = default;

    /**
     * @brief Write a single byte
     * @param b The byte to write (0-255)
     */
    virtual void Write(int b) = 0;

    /**
     * @brief Write bytes from a buffer
     * @param buffer Buffer containing data to write
     * @param offset Starting offset in buffer
     * @param length Number of bytes to write
     */
    virtual void Write(const uint8_t* buffer, size_t offset, size_t length) = 0;

    /**
     * @brief Write bytes from a buffer (simplified version)
     * @param buffer Buffer containing data to write
     * @param length Number of bytes to write
     */
    void Write(const uint8_t* buffer, size_t length) {
        Write(buffer, 0, length);
    }

    /**
     * @brief Flush any buffered output
     */
    virtual void Flush() = 0;

    /**
     * @brief Close the stream and release resources
     */
    virtual void Close() = 0;

protected:
    OutputStream() = default;
};

} // namespace ului

#endif // OUTPUT_STREAM_H
