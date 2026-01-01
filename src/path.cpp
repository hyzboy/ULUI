#include "path.h"
#include <algorithm>
#include <cctype>

namespace ului {

// ===== Helper Functions =====

char Path::GetSeparator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

bool Path::IsSeparator(char c) {
    return c == '/' || c == '\\';
}

void Path::NormalizeSeparators() {
    char sep = GetSeparator();
    for (size_t i = 0; i < m_path.length(); ++i) {
        if (IsSeparator(m_path[i])) {
            m_path[i] = sep;
        }
    }
}

// ===== Constructors =====

Path::Path() : m_path() {
}

Path::Path(const char* path) : m_path(path ? path : "") {
    NormalizeSeparators();
}

Path::Path(const std::string& path) : m_path(path) {
    NormalizeSeparators();
}

Path::Path(const Path& other) : m_path(other.m_path) {
}

Path& Path::operator=(const Path& other) {
    if (this != &other) {
        m_path = other.m_path;
    }
    return *this;
}

// ===== Path Combination =====

Path Path::operator/(const Path& other) const {
    return Append(other);
}

Path Path::operator/(const char* other) const {
    return Append(Path(other));
}

Path& Path::operator/=(const Path& other) {
    *this = Append(other);
    return *this;
}

Path Path::Append(const Path& other) const {
    // If other is absolute, return it as-is
    if (other.IsAbsolute()) {
        return other;
    }
    
    // If this path is empty, return other
    if (m_path.empty()) {
        return other;
    }
    
    // If other is empty, return this
    if (other.m_path.empty()) {
        return *this;
    }
    
    // Check if we need to add a separator
    bool thisEndsWith = IsSeparator(m_path.back());
    bool otherStartsWith = IsSeparator(other.m_path.front());
    
    std::string result = m_path;
    
    if (!thisEndsWith && !otherStartsWith) {
        // Need to add separator
        result += GetSeparator();
        result += other.m_path;
    } else if (thisEndsWith && otherStartsWith) {
        // Remove duplicate separator
        result += other.m_path.substr(1);
    } else {
        // One has separator, just concatenate
        result += other.m_path;
    }
    
    return Path(result);
}

// ===== Path Decomposition =====

Path Path::GetParent() const {
    if (m_path.empty()) {
        return Path();
    }
    
    // Find last separator (ignoring trailing separator)
    size_t lastPos = m_path.length();
    
    // Skip trailing separators
    while (lastPos > 0 && IsSeparator(m_path[lastPos - 1])) {
        --lastPos;
    }
    
    if (lastPos == 0) {
        return Path();
    }
    
    // Find the separator before the last component
    size_t sepPos = m_path.find_last_of("/\\", lastPos - 1);
    
    if (sepPos == std::string::npos) {
        // No separator found, no parent
        return Path();
    }
    
    // Return path up to (and including) the separator
    return Path(m_path.substr(0, sepPos));
}

std::string Path::GetFileName() const {
    if (m_path.empty()) {
        return "";
    }
    
    // Find last separator
    size_t lastPos = m_path.length();
    
    // Skip trailing separators
    while (lastPos > 0 && IsSeparator(m_path[lastPos - 1])) {
        --lastPos;
    }
    
    if (lastPos == 0) {
        return "";
    }
    
    // Find the separator before the last component
    size_t sepPos = m_path.find_last_of("/\\", lastPos - 1);
    
    if (sepPos == std::string::npos) {
        // No separator, entire path is the filename
        return m_path.substr(0, lastPos);
    }
    
    // Return component after the last separator
    return m_path.substr(sepPos + 1, lastPos - sepPos - 1);
}

std::string Path::GetExtension() const {
    std::string filename = GetFileName();
    
    if (filename.empty()) {
        return "";
    }
    
    // Find last dot
    size_t dotPos = filename.find_last_of('.');
    
    // No dot, or dot is at the beginning (hidden file on Unix)
    if (dotPos == std::string::npos || dotPos == 0) {
        return "";
    }
    
    // Return extension including the dot
    return filename.substr(dotPos);
}

std::string Path::GetFileNameWithoutExtension() const {
    std::string filename = GetFileName();
    
    if (filename.empty()) {
        return "";
    }
    
    // Find last dot
    size_t dotPos = filename.find_last_of('.');
    
    // No dot, or dot is at the beginning (hidden file on Unix)
    if (dotPos == std::string::npos || dotPos == 0) {
        return filename;
    }
    
    // Return filename without extension
    return filename.substr(0, dotPos);
}

// ===== Query Methods =====

bool Path::IsAbsolute() const {
    if (m_path.empty()) {
        return false;
    }
    
#ifdef _WIN32
    // Windows: Check for drive letter (C:\) or UNC path (\\)
    if (m_path.length() >= 2) {
        // Check for drive letter
        if (isalpha(m_path[0]) && m_path[1] == ':') {
            return true;
        }
        // Check for UNC path
        if (IsSeparator(m_path[0]) && IsSeparator(m_path[1])) {
            return true;
        }
    }
    return false;
#else
    // Unix: Check if starts with /
    return IsSeparator(m_path[0]);
#endif
}

bool Path::IsEmpty() const {
    return m_path.empty();
}

// ===== Conversion Methods =====

const char* Path::c_str() const {
    return m_path.c_str();
}

const std::string& Path::ToString() const {
    return m_path;
}

Path::operator std::string() const {
    return m_path;
}

// ===== Normalization =====

Path Path::Normalize() const {
    if (m_path.empty()) {
        return Path();
    }
    
    std::string result;
    result.reserve(m_path.length());
    
    char sep = GetSeparator();
    bool lastWasSep = false;
    
    for (size_t i = 0; i < m_path.length(); ++i) {
        char c = m_path[i];
        
        if (IsSeparator(c)) {
            // Convert to platform separator and avoid duplicates
            if (!lastWasSep) {
                result += sep;
                lastWasSep = true;
            }
        } else {
            result += c;
            lastWasSep = false;
        }
    }
    
    // Remove trailing separator unless it's the root
    if (result.length() > 1 && IsSeparator(result.back())) {
        result.pop_back();
    }
    
    // Create path without calling NormalizeSeparators again
    // This is safe because 'result' is already normalized by the loop above
    Path normalized;
    normalized.m_path = result;
    return normalized;
}

} // namespace ului
