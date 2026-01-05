#include "FileInputStream.h"
#include "file_system.h"
#include <cstdio>
#include <algorithm>

namespace ului {

FileInputStream::FileInputStream(const Path& path, bool preferAsset)
    : m_path(path)
    , m_isOpen(false)
    , m_isAsset(false)
    , m_position(0)
    , m_fileHandle(nullptr)
{
    // Try asset first if preferred
    if (preferAsset && OpenAsAsset(path)) {
        return;
    }
    
    // Try external file
    if (OpenAsExternal(path)) {
        return;
    }
    
    // If not preferred asset, try as fallback
    if (!preferAsset && OpenAsAsset(path)) {
        return;
    }
}

FileInputStream::~FileInputStream() {
    Close();
}

FileInputStream::FileInputStream(FileInputStream&& other) noexcept
    : m_path(std::move(other.m_path))
    , m_isOpen(other.m_isOpen)
    , m_isAsset(other.m_isAsset)
    , m_position(other.m_position)
    , m_fileHandle(other.m_fileHandle)
    , m_assetData(std::move(other.m_assetData))
{
    other.m_isOpen = false;
    other.m_fileHandle = nullptr;
    other.m_position = 0;
}

FileInputStream& FileInputStream::operator=(FileInputStream&& other) noexcept {
    if (this != &other) {
        Close();
        
        m_path = std::move(other.m_path);
        m_isOpen = other.m_isOpen;
        m_isAsset = other.m_isAsset;
        m_position = other.m_position;
        m_fileHandle = other.m_fileHandle;
        m_assetData = std::move(other.m_assetData);
        
        other.m_isOpen = false;
        other.m_fileHandle = nullptr;
        other.m_position = 0;
    }
    return *this;
}

int FileInputStream::Read() {
    if (!m_isOpen) {
        return -1;
    }
    
    if (m_isAsset) {
        if (m_position >= static_cast<int64_t>(m_assetData.size())) {
            return -1;
        }
        return static_cast<int>(m_assetData[m_position++]);
    } else {
        if (!m_fileHandle) {
            return -1;
        }
        int ch = std::fgetc(static_cast<FILE*>(m_fileHandle));
        return (ch == EOF) ? -1 : ch;
    }
}

int FileInputStream::Read(uint8_t* buffer, size_t offset, size_t length) {
    if (!m_isOpen || !buffer || length == 0) {
        return -1;
    }
    
    if (m_isAsset) {
        size_t remaining = m_assetData.size() - static_cast<size_t>(m_position);
        if (remaining == 0) {
            return -1;
        }
        
        size_t toRead = std::min(length, remaining);
        std::copy(m_assetData.begin() + m_position, 
                  m_assetData.begin() + m_position + toRead,
                  buffer + offset);
        m_position += toRead;
        return static_cast<int>(toRead);
    } else {
        if (!m_fileHandle) {
            return -1;
        }
        
        size_t bytesRead = std::fread(buffer + offset, 1, length, static_cast<FILE*>(m_fileHandle));
        if (bytesRead == 0 && std::feof(static_cast<FILE*>(m_fileHandle))) {
            return -1;
        }
        return static_cast<int>(bytesRead);
    }
}

int FileInputStream::Available() {
    if (!m_isOpen) {
        return 0;
    }
    
    if (m_isAsset) {
        return static_cast<int>(m_assetData.size() - m_position);
    } else {
        if (!m_fileHandle) {
            return 0;
        }
        
        // Get current position and file size
        FILE* file = static_cast<FILE*>(m_fileHandle);
#ifdef _WIN32
        int64_t currentPos = _ftelli64(file);
        _fseeki64(file, 0, SEEK_END);
        int64_t endPos = _ftelli64(file);
        _fseeki64(file, currentPos, SEEK_SET);
#else
        int64_t currentPos = ftello(file);
        fseeko(file, 0, SEEK_END);
        int64_t endPos = ftello(file);
        fseeko(file, currentPos, SEEK_SET);
#endif
        return static_cast<int>(endPos - currentPos);
    }
}

void FileInputStream::Close() {
    if (!m_isOpen) {
        return;
    }
    
    if (m_isAsset) {
        m_assetData.clear();
    } else {
        if (m_fileHandle) {
            std::fclose(static_cast<FILE*>(m_fileHandle));
            m_fileHandle = nullptr;
        }
    }
    
    m_isOpen = false;
    m_position = 0;
}

bool FileInputStream::OpenAsAsset(const Path& path) {
    m_assetData = FileSystem::ReadAssetBinary(path);
    
    if (m_assetData.empty()) {
        if (!FileSystem::AssetExists(path)) {
            return false;
        }
    }
    
    m_isAsset = true;
    m_isOpen = true;
    m_position = 0;
    return true;
}

bool FileInputStream::OpenAsExternal(const Path& path) {
    FILE* file = std::fopen(path.c_str(), "rb");
    
    if (!file) {
        return false;
    }
    
    m_fileHandle = file;
    m_isAsset = false;
    m_isOpen = true;
    m_position = 0;
    
    return true;
}

} // namespace ului
