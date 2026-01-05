#ifndef DATA_INPUT_STREAM_H
#define DATA_INPUT_STREAM_H

#include "InputStream.h"
#include <memory>
#include <string>

namespace ului {

/**
 * @brief Data input stream for reading primitive types
 * 
 * Similar to Java's DataInputStream, provides methods for reading
 * primitive data types from an underlying input stream.
 * Data is read in native byte order (platform-specific endianness).
 */
class DataInputStream {
public:
    /**
     * @brief Construct from an input stream
     * @param in The underlying input stream (takes ownership)
     */
    explicit DataInputStream(std::unique_ptr<InputStream> in);

    /**
     * @brief Destructor
     */
    ~DataInputStream();

    // Disable copy
    DataInputStream(const DataInputStream&) = delete;
    DataInputStream& operator=(const DataInputStream&) = delete;

    // Enable move
    DataInputStream(DataInputStream&& other) noexcept;
    DataInputStream& operator=(DataInputStream&& other) noexcept;

    /**
     * @brief Read a boolean value (1 byte)
     * @return The boolean value
     */
    bool ReadBoolean();

    /**
     * @brief Read a signed byte (1 byte)
     * @return The byte value
     */
    int8_t ReadByte();

    /**
     * @brief Read an unsigned byte (1 byte)
     * @return The unsigned byte value
     */
    uint8_t ReadUnsignedByte();

    /**
     * @brief Read a signed short (2 bytes, native byte order)
     * @return The short value
     */
    int16_t ReadShort();

    /**
     * @brief Read an unsigned short (2 bytes, native byte order)
     * @return The unsigned short value
     */
    uint16_t ReadUnsignedShort();

    /**
     * @brief Read a signed int (4 bytes, native byte order)
     * @return The int value
     */
    int32_t ReadInt();

    /**
     * @brief Read a signed long (8 bytes, native byte order)
     * @return The long value
     */
    int64_t ReadLong();

    /**
     * @brief Read a float (4 bytes, IEEE 754, native byte order)
     * @return The float value
     */
    float ReadFloat();

    /**
     * @brief Read a double (8 bytes, IEEE 754, native byte order)
     * @return The double value
     */
    double ReadDouble();

    /**
     * @brief Read a UTF-8 string
     * @return The string value
     * 
     * Format: 2-byte length (native byte order) followed by UTF-8 bytes
     */
    std::string ReadUTF();

    /**
     * @brief Read bytes into a buffer
     * @param buffer Buffer to read into
     * @param length Number of bytes to read
     * @return Number of bytes actually read
     */
    int Read(uint8_t* buffer, size_t length);

    /**
     * @brief Fully read bytes into a buffer
     * @param buffer Buffer to read into
     * @param length Number of bytes to read
     * @throws std::runtime_error if cannot read all bytes
     */
    void ReadFully(uint8_t* buffer, size_t length);

    /**
     * @brief Skip bytes
     * @param n Number of bytes to skip
     * @return Actual number of bytes skipped
     */
    int64_t Skip(int64_t n);

    /**
     * @brief Close the underlying stream
     */
    void Close();

    /**
     * @brief Get the underlying input stream
     * @return Reference to the input stream
     */
    InputStream& GetInputStream() { return *m_in; }

private:
    std::unique_ptr<InputStream> m_in;

    uint16_t ReadUInt16BigEndian();
    uint32_t ReadUInt32BigEndian();
    uint64_t ReadUInt64BigEndian();
};

} // namespace ului

#endif // DATA_INPUT_STREAM_H
