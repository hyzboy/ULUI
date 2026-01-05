#include "FileOutputStream.h"
#include <cstdio>

namespace ului {

FileOutputStream::FileOutputStream(const Path& path, bool append)
    : m_path(path)
    , m_isOpen(false)
    , m_fileHandle(nullptr)
{
    const char* mode = append ? "ab" : "wb";
    FILE* file = std::fopen(path.c_str(), mode);
    
    if (file) {
        m_fileHandle = file;
        m_isOpen = true;
    }
}

FileOutputStream::~FileOutputStream() {
    Close();
}

FileOutputStream::FileOutputStream(FileOutputStream&& other) noexcept
    : m_path(std::move(other.m_path))
    , m_isOpen(other.m_isOpen)
    , m_fileHandle(other.m_fileHandle)
{
    other.m_isOpen = false;
    other.m_fileHandle = nullptr;
}

FileOutputStream& FileOutputStream::operator=(FileOutputStream&& other) noexcept {
    if (this != &other) {
        Close();
        
        m_path = std::move(other.m_path);
        m_isOpen = other.m_isOpen;
        m_fileHandle = other.m_fileHandle;
        
        other.m_isOpen = false;
        other.m_fileHandle = nullptr;
    }
    return *this;
}

void FileOutputStream::Write(int b) {
    if (!m_isOpen || !m_fileHandle) {
        return;
    }
    
    std::fputc(b & 0xFF, static_cast<FILE*>(m_fileHandle));
}

void FileOutputStream::Write(const uint8_t* buffer, size_t offset, size_t length) {
    if (!m_isOpen || !m_fileHandle || !buffer || length == 0) {
        return;
    }
    
    std::fwrite(buffer + offset, 1, length, static_cast<FILE*>(m_fileHandle));
}

void FileOutputStream::Flush() {
    if (!m_isOpen || !m_fileHandle) {
        return;
    }
    
    std::fflush(static_cast<FILE*>(m_fileHandle));
}

void FileOutputStream::Close() {
    if (!m_isOpen) {
        return;
    }
    
    if (m_fileHandle) {
        std::fclose(static_cast<FILE*>(m_fileHandle));
        m_fileHandle = nullptr;
    }
    
    m_isOpen = false;
}

} // namespace ului
