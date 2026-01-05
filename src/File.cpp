#include "File.h"
#include "file_system.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace ului {

// ===== Constructor / Destructor =====

File::File()
    : m_mode(OpenMode::Read)
    , m_isOpen(false)
    , m_isAsset(false)
    , m_position(0)
    , m_fileHandle(nullptr)
{
}

File::File(const Path& path, OpenMode mode, bool preferAsset)
    : m_path(path)
    , m_mode(mode)
    , m_isOpen(false)
    , m_isAsset(false)
    , m_position(0)
    , m_fileHandle(nullptr)
{
    Open(path, mode, preferAsset);
}

File::~File() {
    Close();
}

// ===== Move Semantics =====

File::File(File&& other) noexcept
    : m_path(std::move(other.m_path))
    , m_mode(other.m_mode)
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

File& File::operator=(File&& other) noexcept {
    if (this != &other) {
        Close();
        
        m_path = std::move(other.m_path);
        m_mode = other.m_mode;
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

// ===== File Operations =====

bool File::Open(const Path& path, OpenMode mode, bool preferAsset) {
    // Close any previously opened file
    Close();
    
    m_path = path;
    m_mode = mode;
    
    // For read mode, try asset first (if preferred), then external
    if (mode == OpenMode::Read) {
        if (preferAsset && OpenAsAsset(path)) {
            return true;
        }
        if (OpenAsExternal(path, mode)) {
            return true;
        }
        // If preferAsset was false, try asset as fallback
        if (!preferAsset && OpenAsAsset(path)) {
            return true;
        }
        return false;
    }
    
    // For write modes, only use external files
    return OpenAsExternal(path, mode);
}

void File::Close() {
    if (!m_isOpen) {
        return;
    }
    
    if (m_isAsset) {
        // Asset files just need to clear the cached data
        m_assetData.clear();
    } else {
        // External files need to close the file handle
        if (m_fileHandle) {
            std::fclose(static_cast<FILE*>(m_fileHandle));
            m_fileHandle = nullptr;
        }
    }
    
    m_isOpen = false;
    m_position = 0;
}

bool File::IsOpen() const {
    return m_isOpen;
}

size_t File::Read(void* buffer, size_t size) {
    if (!m_isOpen || !CanRead()) {
        return 0;
    }
    
    if (m_isAsset) {
        // Read from cached asset data
        size_t remaining = m_assetData.size() - static_cast<size_t>(m_position);
        size_t toRead = std::min(size, remaining);
        
        if (toRead > 0) {
            std::memcpy(buffer, m_assetData.data() + m_position, toRead);
            m_position += toRead;
        }
        
        return toRead;
    } else {
        // Read from external file
        if (!m_fileHandle) {
            return 0;
        }
        
        size_t bytesRead = std::fread(buffer, 1, size, static_cast<FILE*>(m_fileHandle));
        return bytesRead;
    }
}

std::vector<uint8_t> File::Read(size_t size) {
    std::vector<uint8_t> buffer(size);
    size_t bytesRead = Read(buffer.data(), size);
    buffer.resize(bytesRead);
    return buffer;
}

std::vector<uint8_t> File::ReadAll() {
    if (!m_isOpen || !CanRead()) {
        return {};
    }
    
    if (m_isAsset) {
        // Return copy of all asset data from current position
        std::vector<uint8_t> result(
            m_assetData.begin() + m_position,
            m_assetData.end()
        );
        m_position = m_assetData.size();
        return result;
    } else {
        // Read all remaining data from external file
        int64_t currentPos = Tell();
        int64_t length = GetLength();
        
        if (currentPos < 0 || length < 0) {
            return {};
        }
        
        size_t remaining = static_cast<size_t>(length - currentPos);
        return Read(remaining);
    }
}

std::string File::ReadAllText() {
    auto data = ReadAll();
    if (data.empty()) {
        return "";
    }
    return std::string(data.begin(), data.end());
}

size_t File::Write(const void* buffer, size_t size) {
    if (!m_isOpen || !CanWrite()) {
        return 0;
    }
    
    if (m_isAsset) {
        // Cannot write to asset files
        return 0;
    }
    
    if (!m_fileHandle) {
        return 0;
    }
    
    size_t bytesWritten = std::fwrite(buffer, 1, size, static_cast<FILE*>(m_fileHandle));
    return bytesWritten;
}

size_t File::Write(const std::vector<uint8_t>& data) {
    return Write(data.data(), data.size());
}

size_t File::Write(const std::string& text) {
    return Write(text.data(), text.size());
}

bool File::Seek(int64_t offset, SeekOrigin origin) {
    if (!m_isOpen) {
        return false;
    }
    
    if (m_isAsset) {
        // Handle seeking in asset data
        int64_t newPos = m_position;
        
        switch (origin) {
            case SeekOrigin::Begin:
                newPos = offset;
                break;
            case SeekOrigin::Current:
                newPos = m_position + offset;
                break;
            case SeekOrigin::End:
                newPos = static_cast<int64_t>(m_assetData.size()) + offset;
                break;
        }
        
        // Clamp to valid range
        if (newPos < 0) {
            newPos = 0;
        } else if (newPos > static_cast<int64_t>(m_assetData.size())) {
            newPos = static_cast<int64_t>(m_assetData.size());
        }
        
        m_position = newPos;
        return true;
    } else {
        // Handle seeking in external file
        if (!m_fileHandle) {
            return false;
        }
        
        int whence;
        switch (origin) {
            case SeekOrigin::Begin:
                whence = SEEK_SET;
                break;
            case SeekOrigin::Current:
                whence = SEEK_CUR;
                break;
            case SeekOrigin::End:
                whence = SEEK_END;
                break;
            default:
                return false;
        }
        
#ifdef _WIN32
        return _fseeki64(static_cast<FILE*>(m_fileHandle), offset, whence) == 0;
#else
        return fseeko(static_cast<FILE*>(m_fileHandle), offset, whence) == 0;
#endif
    }
}

int64_t File::Tell() const {
    if (!m_isOpen) {
        return -1;
    }
    
    if (m_isAsset) {
        return m_position;
    } else {
        if (!m_fileHandle) {
            return -1;
        }
        
#ifdef _WIN32
        return _ftelli64(static_cast<FILE*>(m_fileHandle));
#else
        return ftello(static_cast<FILE*>(m_fileHandle));
#endif
    }
}

bool File::Flush() {
    if (!m_isOpen || m_isAsset || !m_fileHandle) {
        return false;
    }
    
    return std::fflush(static_cast<FILE*>(m_fileHandle)) == 0;
}

// ===== File Properties =====

int64_t File::GetLength() const {
    if (!m_isOpen) {
        return -1;
    }
    
    if (m_isAsset) {
        return static_cast<int64_t>(m_assetData.size());
    } else {
        if (!m_fileHandle) {
            return -1;
        }
        
        // Save current position
        int64_t currentPos = Tell();
        if (currentPos < 0) {
            return -1;
        }
        
        // Seek to end to get size
        FILE* file = static_cast<FILE*>(m_fileHandle);
#ifdef _WIN32
        if (_fseeki64(file, 0, SEEK_END) != 0) {
            return -1;
        }
        int64_t length = _ftelli64(file);
        _fseeki64(file, currentPos, SEEK_SET);
#else
        if (fseeko(file, 0, SEEK_END) != 0) {
            return -1;
        }
        int64_t length = ftello(file);
        fseeko(file, currentPos, SEEK_SET);
#endif
        
        return length;
    }
}

bool File::CanRead() const {
    if (!m_isOpen) {
        return false;
    }
    
    return m_mode == OpenMode::Read || m_mode == OpenMode::ReadWrite;
}

bool File::CanWrite() const {
    if (!m_isOpen) {
        return false;
    }
    
    // Asset files are read-only
    if (m_isAsset) {
        return false;
    }
    
    return m_mode == OpenMode::Write || 
           m_mode == OpenMode::ReadWrite || 
           m_mode == OpenMode::Append;
}

bool File::IsAsset() const {
    return m_isOpen && m_isAsset;
}

bool File::IsExternal() const {
    return m_isOpen && !m_isAsset;
}

bool File::IsEof() const {
    if (!m_isOpen) {
        return true;
    }
    
    if (m_isAsset) {
        return m_position >= static_cast<int64_t>(m_assetData.size());
    } else {
        if (!m_fileHandle) {
            return true;
        }
        return std::feof(static_cast<FILE*>(m_fileHandle)) != 0;
    }
}

const Path& File::GetPath() const {
    return m_path;
}

// ===== Private Methods =====

bool File::OpenAsAsset(const Path& path) {
    // Try to load as asset using FileSystem
    m_assetData = FileSystem::ReadAssetBinary(path);
    
    if (m_assetData.empty()) {
        // Check if the file exists but is empty, or doesn't exist
        if (!FileSystem::AssetExists(path)) {
            return false;
        }
        // File exists but is empty - that's still valid
    }
    
    m_isAsset = true;
    m_isOpen = true;
    m_position = 0;
    return true;
}

bool File::OpenAsExternal(const Path& path, OpenMode mode) {
    const char* modeStr = nullptr;
    
    switch (mode) {
        case OpenMode::Read:
            modeStr = "rb";
            break;
        case OpenMode::Write:
            modeStr = "wb";
            break;
        case OpenMode::ReadWrite:
            modeStr = "rb+";
            // If file doesn't exist, try "wb+"
            break;
        case OpenMode::Append:
            modeStr = "ab";
            break;
        default:
            return false;
    }
    
    FILE* file = std::fopen(path.c_str(), modeStr);
    
    // For ReadWrite mode, try alternate mode if file doesn't exist
    if (!file && mode == OpenMode::ReadWrite) {
        file = std::fopen(path.c_str(), "wb+");
    }
    
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
