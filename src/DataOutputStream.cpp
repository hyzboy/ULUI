#include "DataOutputStream.h"
#include <stdexcept>
#include <cstring>

namespace ului {

DataOutputStream::DataOutputStream(std::unique_ptr<OutputStream> out)
    : m_out(std::move(out))
    , m_written(0)
{
    if (!m_out) {
        throw std::invalid_argument("OutputStream cannot be null");
    }
}

DataOutputStream::~DataOutputStream() = default;

DataOutputStream::DataOutputStream(DataOutputStream&& other) noexcept
    : m_out(std::move(other.m_out))
    , m_written(other.m_written)
{
    other.m_written = 0;
}

DataOutputStream& DataOutputStream::operator=(DataOutputStream&& other) noexcept {
    if (this != &other) {
        m_out = std::move(other.m_out);
        m_written = other.m_written;
        other.m_written = 0;
    }
    return *this;
}

void DataOutputStream::WriteBoolean(bool v) {
    m_out->Write(v ? 1 : 0);
    m_written++;
}

void DataOutputStream::WriteByte(int8_t v) {
    m_out->Write(static_cast<uint8_t>(v));
    m_written++;
}

void DataOutputStream::WriteShort(int16_t v) {
    WriteUInt16BigEndian(static_cast<uint16_t>(v));
}

void DataOutputStream::WriteInt(int32_t v) {
    WriteUInt32BigEndian(static_cast<uint32_t>(v));
}

void DataOutputStream::WriteLong(int64_t v) {
    WriteUInt64BigEndian(static_cast<uint64_t>(v));
}

void DataOutputStream::WriteFloat(float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(float));
    WriteUInt32BigEndian(bits);
}

void DataOutputStream::WriteDouble(double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(double));
    WriteUInt64BigEndian(bits);
}

void DataOutputStream::WriteUTF(const std::string& str) {
    // Java's DataOutputStream.writeUTF() has a 65535 byte limit for UTF-8 encoded data
    if (str.length() > 65535) {
        throw std::invalid_argument("String exceeds 65535 bytes for UTF encoding");
    }
    
    WriteUInt16BigEndian(static_cast<uint16_t>(str.length()));
    
    if (!str.empty()) {
        m_out->Write(reinterpret_cast<const uint8_t*>(str.data()), 0, str.length());
        m_written += str.length();
    }
}

void DataOutputStream::Write(const uint8_t* buffer, size_t length) {
    m_out->Write(buffer, 0, length);
    m_written += length;
}

void DataOutputStream::Flush() {
    m_out->Flush();
}

void DataOutputStream::Close() {
    if (m_out) {
        m_out->Close();
    }
}

void DataOutputStream::WriteUInt16BigEndian(uint16_t v) {
    uint8_t buffer[2];
    buffer[0] = static_cast<uint8_t>(v >> 8);
    buffer[1] = static_cast<uint8_t>(v);
    m_out->Write(buffer, 0, 2);
    m_written += 2;
}

void DataOutputStream::WriteUInt32BigEndian(uint32_t v) {
    uint8_t buffer[4];
    buffer[0] = static_cast<uint8_t>(v >> 24);
    buffer[1] = static_cast<uint8_t>(v >> 16);
    buffer[2] = static_cast<uint8_t>(v >> 8);
    buffer[3] = static_cast<uint8_t>(v);
    m_out->Write(buffer, 0, 4);
    m_written += 4;
}

void DataOutputStream::WriteUInt64BigEndian(uint64_t v) {
    uint8_t buffer[8];
    buffer[0] = static_cast<uint8_t>(v >> 56);
    buffer[1] = static_cast<uint8_t>(v >> 48);
    buffer[2] = static_cast<uint8_t>(v >> 40);
    buffer[3] = static_cast<uint8_t>(v >> 32);
    buffer[4] = static_cast<uint8_t>(v >> 24);
    buffer[5] = static_cast<uint8_t>(v >> 16);
    buffer[6] = static_cast<uint8_t>(v >> 8);
    buffer[7] = static_cast<uint8_t>(v);
    m_out->Write(buffer, 0, 8);
    m_written += 8;
}

} // namespace ului
