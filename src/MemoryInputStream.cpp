#include "MemoryInputStream.h"
#include <algorithm>
#include <cstring>

namespace ului {

MemoryInputStream::MemoryInputStream(const std::vector<uint8_t>& data)
    : m_data(data)
    , m_position(0)
    , m_markPosition(0)
{
}

MemoryInputStream::MemoryInputStream(std::vector<uint8_t>&& data)
    : m_data(std::move(data))
    , m_position(0)
    , m_markPosition(0)
{
}

MemoryInputStream::MemoryInputStream(const uint8_t* data, size_t length)
    : m_data(data, data + length)
    , m_position(0)
    , m_markPosition(0)
{
}

MemoryInputStream::~MemoryInputStream() = default;

MemoryInputStream::MemoryInputStream(MemoryInputStream&& other) noexcept
    : m_data(std::move(other.m_data))
    , m_position(other.m_position)
    , m_markPosition(other.m_markPosition)
{
    other.m_position = 0;
    other.m_markPosition = 0;
}

MemoryInputStream& MemoryInputStream::operator=(MemoryInputStream&& other) noexcept {
    if (this != &other) {
        m_data = std::move(other.m_data);
        m_position = other.m_position;
        m_markPosition = other.m_markPosition;
        
        other.m_position = 0;
        other.m_markPosition = 0;
    }
    return *this;
}

int MemoryInputStream::Read() {
    if (m_position >= m_data.size()) {
        return -1;
    }
    return static_cast<int>(m_data[m_position++]);
}

int MemoryInputStream::Read(uint8_t* buffer, size_t offset, size_t length) {
    if (!buffer || length == 0) {
        return 0;
    }
    
    if (m_position >= m_data.size()) {
        return -1;
    }
    
    size_t available = m_data.size() - m_position;
    size_t toRead = std::min(length, available);
    
    std::memcpy(buffer + offset, m_data.data() + m_position, toRead);
    m_position += toRead;
    
    return static_cast<int>(toRead);
}

int MemoryInputStream::Available() {
    if (m_position >= m_data.size()) {
        return 0;
    }
    return static_cast<int>(m_data.size() - m_position);
}

void MemoryInputStream::Close() {
    m_data.clear();
    m_position = 0;
    m_markPosition = 0;
}

bool MemoryInputStream::MarkSupported() const {
    return true;
}

void MemoryInputStream::Mark(int readlimit) {
    // readlimit is ignored for memory streams
    (void)readlimit;
    m_markPosition = m_position;
}

void MemoryInputStream::Reset() {
    m_position = m_markPosition;
}

} // namespace ului
