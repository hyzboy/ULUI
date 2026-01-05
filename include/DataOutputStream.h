#ifndef DATA_OUTPUT_STREAM_H
#define DATA_OUTPUT_STREAM_H

#include "OutputStream.h"
#include <memory>
#include <string>

namespace ului {

/**
 * @brief Data output stream for writing primitive types
 * 
 * Similar to Java's DataOutputStream, provides methods for writing
 * primitive data types to an underlying output stream.
 * Data is written in big-endian format by default.
 */
class DataOutputStream {
public:
    /**
     * @brief Construct from an output stream
     * @param out The underlying output stream (takes ownership)
     */
    explicit DataOutputStream(std::unique_ptr<OutputStream> out);

    /**
     * @brief Destructor
     */
    ~DataOutputStream();

    // Disable copy
    DataOutputStream(const DataOutputStream&) = delete;
    DataOutputStream& operator=(const DataOutputStream&) = delete;

    // Enable move
    DataOutputStream(DataOutputStream&& other) noexcept;
    DataOutputStream& operator=(DataOutputStream&& other) noexcept;

    /**
     * @brief Write a boolean value (1 byte)
     * @param v The boolean value
     */
    void WriteBoolean(bool v);

    /**
     * @brief Write a signed byte (1 byte)
     * @param v The byte value
     */
    void WriteByte(int8_t v);

    /**
     * @brief Write a signed short (2 bytes, big-endian)
     * @param v The short value
     */
    void WriteShort(int16_t v);

    /**
     * @brief Write a signed int (4 bytes, big-endian)
     * @param v The int value
     */
    void WriteInt(int32_t v);

    /**
     * @brief Write a signed long (8 bytes, big-endian)
     * @param v The long value
     */
    void WriteLong(int64_t v);

    /**
     * @brief Write a float (4 bytes, IEEE 754, big-endian)
     * @param v The float value
     */
    void WriteFloat(float v);

    /**
     * @brief Write a double (8 bytes, IEEE 754, big-endian)
     * @param v The double value
     */
    void WriteDouble(double v);

    /**
     * @brief Write a UTF-8 string
     * @param str The string to write
     * 
     * Format: 2-byte length (big-endian) followed by UTF-8 bytes
     */
    void WriteUTF(const std::string& str);

    /**
     * @brief Write bytes from a buffer
     * @param buffer Buffer containing data to write
     * @param length Number of bytes to write
     */
    void Write(const uint8_t* buffer, size_t length);

    /**
     * @brief Flush the underlying stream
     */
    void Flush();

    /**
     * @brief Close the underlying stream
     */
    void Close();

    /**
     * @brief Get number of bytes written
     * @return Total bytes written
     */
    size_t GetBytesWritten() const { return m_written; }

    /**
     * @brief Get the underlying output stream
     * @return Reference to the output stream
     */
    OutputStream& GetOutputStream() { return *m_out; }

private:
    std::unique_ptr<OutputStream> m_out;
    size_t m_written;

    void WriteUInt16BigEndian(uint16_t v);
    void WriteUInt32BigEndian(uint32_t v);
    void WriteUInt64BigEndian(uint64_t v);
};

} // namespace ului

#endif // DATA_OUTPUT_STREAM_H
