#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include <cstdint>
#include <cstddef>
#include <memory>

namespace ului {

/**
 * @brief Abstract base class for reading streams of bytes
 * 
 * Similar to Java's InputStream, this class provides a basic interface
 * for reading bytes from various sources.
 */
class InputStream {
public:
    virtual ~InputStream() = default;

    /**
     * @brief Read a single byte
     * @return The byte read (0-255), or -1 if end of stream
     */
    virtual int Read() = 0;

    /**
     * @brief Read bytes into a buffer
     * @param buffer Buffer to read into
     * @param offset Starting offset in buffer
     * @param length Maximum number of bytes to read
     * @return Number of bytes actually read, or -1 if end of stream
     */
    virtual int Read(uint8_t* buffer, size_t offset, size_t length) = 0;

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
     * @brief Skip over and discard n bytes
     * @param n Number of bytes to skip
     * @return Actual number of bytes skipped
     */
    virtual int64_t Skip(int64_t n);

    /**
     * @brief Get number of bytes available for reading without blocking
     * @return Number of available bytes
     */
    virtual int Available();

    /**
     * @brief Close the stream and release resources
     */
    virtual void Close() = 0;

    /**
     * @brief Check if mark/reset is supported
     * @return true if mark/reset is supported
     */
    virtual bool MarkSupported() const {
        return false;
    }

    /**
     * @brief Mark current position in stream
     * @param readlimit Maximum bytes that can be read before mark becomes invalid
     */
    virtual void Mark(int readlimit) {
        // Default implementation does nothing
    }

    /**
     * @brief Reset stream to last marked position
     */
    virtual void Reset() {
        // Default implementation does nothing
    }

protected:
    InputStream() = default;
};

} // namespace ului

#endif // INPUT_STREAM_H
