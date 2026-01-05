#include "MemoryOutputStream.h"
#include <algorithm>

namespace ului {

MemoryOutputStream::MemoryOutputStream()
    : m_closed(false)
{
}

MemoryOutputStream::MemoryOutputStream(size_t initialCapacity)
    : m_closed(false)
{
    m_buffer.reserve(initialCapacity);
}

MemoryOutputStream::~MemoryOutputStream() = default;

MemoryOutputStream::MemoryOutputStream(MemoryOutputStream&& other) noexcept
    : m_buffer(std::move(other.m_buffer))
    , m_closed(other.m_closed)
{
    other.m_closed = true;
}

MemoryOutputStream& MemoryOutputStream::operator=(MemoryOutputStream&& other) noexcept {
    if (this != &other) {
        m_buffer = std::move(other.m_buffer);
        m_closed = other.m_closed;
        
        other.m_closed = true;
    }
    return *this;
}

void MemoryOutputStream::Write(int b) {
    if (m_closed) {
        return;
    }
    m_buffer.push_back(static_cast<uint8_t>(b & 0xFF));
}

void MemoryOutputStream::Write(const uint8_t* buffer, size_t offset, size_t length) {
    if (m_closed || !buffer || length == 0) {
        return;
    }
    
    m_buffer.insert(m_buffer.end(), buffer + offset, buffer + offset + length);
}

void MemoryOutputStream::Flush() {
    // No-op for memory streams
}

void MemoryOutputStream::Close() {
    m_closed = true;
}

std::vector<uint8_t> MemoryOutputStream::ToByteArray() const {
    return m_buffer;
}

std::vector<uint8_t> MemoryOutputStream::ToByteArrayAndClear() {
    std::vector<uint8_t> result = std::move(m_buffer);
    m_buffer.clear();
    return result;
}

void MemoryOutputStream::Reset() {
    m_buffer.clear();
    m_closed = false;
}

void MemoryOutputStream::Reserve(size_t capacity) {
    m_buffer.reserve(capacity);
}

} // namespace ului
