#include "DataInputStream.h"
#include <stdexcept>
#include <cstring>

namespace ului {

DataInputStream::DataInputStream(std::unique_ptr<InputStream> in)
    : m_in(std::move(in))
{
    if (!m_in) {
        throw std::invalid_argument("InputStream cannot be null");
    }
}

DataInputStream::~DataInputStream() = default;

DataInputStream::DataInputStream(DataInputStream&& other) noexcept
    : m_in(std::move(other.m_in))
{
}

DataInputStream& DataInputStream::operator=(DataInputStream&& other) noexcept {
    if (this != &other) {
        m_in = std::move(other.m_in);
    }
    return *this;
}

bool DataInputStream::ReadBoolean() {
    int b = m_in->Read();
    if (b < 0) {
        throw std::runtime_error("End of stream");
    }
    return b != 0;
}

int8_t DataInputStream::ReadByte() {
    int b = m_in->Read();
    if (b < 0) {
        throw std::runtime_error("End of stream");
    }
    return static_cast<int8_t>(b);
}

uint8_t DataInputStream::ReadUnsignedByte() {
    int b = m_in->Read();
    if (b < 0) {
        throw std::runtime_error("End of stream");
    }
    return static_cast<uint8_t>(b);
}

int16_t DataInputStream::ReadShort() {
    return static_cast<int16_t>(ReadUInt16BigEndian());
}

uint16_t DataInputStream::ReadUnsignedShort() {
    return ReadUInt16BigEndian();
}

int32_t DataInputStream::ReadInt() {
    return static_cast<int32_t>(ReadUInt32BigEndian());
}

int64_t DataInputStream::ReadLong() {
    return static_cast<int64_t>(ReadUInt64BigEndian());
}

float DataInputStream::ReadFloat() {
    uint32_t bits = ReadUInt32BigEndian();
    float result;
    std::memcpy(&result, &bits, sizeof(float));
    return result;
}

double DataInputStream::ReadDouble() {
    uint64_t bits = ReadUInt64BigEndian();
    double result;
    std::memcpy(&result, &bits, sizeof(double));
    return result;
}

std::string DataInputStream::ReadUTF() {
    uint16_t length = ReadUInt16BigEndian();
    
    if (length == 0) {
        return "";
    }
    
    std::string result(length, '\0');
    ReadFully(reinterpret_cast<uint8_t*>(&result[0]), length);
    
    return result;
}

int DataInputStream::Read(uint8_t* buffer, size_t length) {
    return m_in->Read(buffer, 0, length);
}

void DataInputStream::ReadFully(uint8_t* buffer, size_t length) {
    size_t offset = 0;
    while (offset < length) {
        int bytesRead = m_in->Read(buffer, offset, length - offset);
        if (bytesRead <= 0) {  // Check for both error (-1) and EOF (0)
            throw std::runtime_error("End of stream");
        }
        offset += bytesRead;
    }
}

int64_t DataInputStream::Skip(int64_t n) {
    return m_in->Skip(n);
}

void DataInputStream::Close() {
    if (m_in) {
        m_in->Close();
    }
}

uint16_t DataInputStream::ReadUInt16BigEndian() {
    uint8_t buffer[2];
    ReadFully(buffer, 2);
    return (static_cast<uint16_t>(buffer[0]) << 8) |
           (static_cast<uint16_t>(buffer[1]));
}

uint32_t DataInputStream::ReadUInt32BigEndian() {
    uint8_t buffer[4];
    ReadFully(buffer, 4);
    return (static_cast<uint32_t>(buffer[0]) << 24) |
           (static_cast<uint32_t>(buffer[1]) << 16) |
           (static_cast<uint32_t>(buffer[2]) << 8) |
           (static_cast<uint32_t>(buffer[3]));
}

uint64_t DataInputStream::ReadUInt64BigEndian() {
    uint8_t buffer[8];
    ReadFully(buffer, 8);
    return (static_cast<uint64_t>(buffer[0]) << 56) |
           (static_cast<uint64_t>(buffer[1]) << 48) |
           (static_cast<uint64_t>(buffer[2]) << 40) |
           (static_cast<uint64_t>(buffer[3]) << 32) |
           (static_cast<uint64_t>(buffer[4]) << 24) |
           (static_cast<uint64_t>(buffer[5]) << 16) |
           (static_cast<uint64_t>(buffer[6]) << 8) |
           (static_cast<uint64_t>(buffer[7]));
}

} // namespace ului
