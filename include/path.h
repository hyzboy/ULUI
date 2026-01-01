#ifndef PATH_H
#define PATH_H

#include <string>

namespace ului {

/**
 * @brief Cross-platform path handling class
 * 
 * Provides a unified interface for working with file system paths across different platforms.
 * Automatically handles platform-specific path separators (Windows: \, Unix: /).
 */
class Path {
public:
    /**
     * @brief Default constructor - creates an empty path
     */
    Path();
    
    /**
     * @brief Construct path from C-style string
     * @param path Path string
     */
    Path(const char* path);
    
    /**
     * @brief Construct path from std::string
     * @param path Path string
     */
    Path(const std::string& path);
    
    /**
     * @brief Copy constructor
     * @param other Path to copy
     */
    Path(const Path& other);
    
    /**
     * @brief Copy assignment operator
     * @param other Path to copy
     * @return Reference to this path
     */
    Path& operator=(const Path& other);
    
    // ===== Path Combination =====
    
    /**
     * @brief Append another path component
     * @param other Path to append
     * @return New combined path
     * 
     * Automatically adds the appropriate path separator between components.
     * Example: Path("a") / "b" / "c.txt" -> "a/b/c.txt"
     */
    Path operator/(const Path& other) const;
    
    /**
     * @brief Append path component from C-style string
     * @param other Path to append
     * @return New combined path
     */
    Path operator/(const char* other) const;
    
    /**
     * @brief Append path component in-place
     * @param other Path to append
     * @return Reference to this path
     */
    Path& operator/=(const Path& other);
    
    /**
     * @brief Append another path component (same as operator/)
     * @param other Path to append
     * @return New combined path
     */
    Path Append(const Path& other) const;
    
    // ===== Path Decomposition =====
    
    /**
     * @brief Get parent directory path
     * @return Parent directory path, or empty if no parent
     * 
     * Example: Path("a/b/c.txt").GetParent() -> "a/b"
     */
    Path GetParent() const;
    
    /**
     * @brief Get file name with extension
     * @return File name, or empty if path is empty or ends with separator
     * 
     * Example: Path("a/b/c.txt").GetFileName() -> "c.txt"
     */
    std::string GetFileName() const;
    
    /**
     * @brief Get file extension (including the dot)
     * @return Extension string, or empty if no extension
     * 
     * Example: Path("a/b/c.txt").GetExtension() -> ".txt"
     */
    std::string GetExtension() const;
    
    /**
     * @brief Get file name without extension
     * @return File name without extension
     * 
     * Example: Path("a/b/c.txt").GetFileNameWithoutExtension() -> "c"
     */
    std::string GetFileNameWithoutExtension() const;
    
    // ===== Query Methods =====
    
    /**
     * @brief Check if path is absolute
     * @return true if path is absolute, false otherwise
     * 
     * On Windows: starts with drive letter (C:\) or UNC path (\\)
     * On Unix: starts with /
     */
    bool IsAbsolute() const;
    
    /**
     * @brief Check if path is empty
     * @return true if path is empty, false otherwise
     */
    bool IsEmpty() const;
    
    // ===== Conversion Methods =====
    
    /**
     * @brief Get C-style string representation
     * @return Pointer to internal string buffer
     */
    const char* c_str() const;
    
    /**
     * @brief Get std::string representation
     * @return Path as std::string
     */
    const std::string& ToString() const;
    
    /**
     * @brief Implicit conversion to std::string
     * @return Path as std::string
     */
    operator std::string() const;
    
    // ===== Normalization =====
    
    /**
     * @brief Normalize path separators and remove redundant separators
     * @return Normalized path
     * 
     * - Converts all separators to platform-specific separator
     * - Removes duplicate separators (a//b -> a/b)
     * - Does not resolve . or .. components
     */
    Path Normalize() const;
    
private:
    std::string m_path;
    
    /**
     * @brief Normalize path separators to platform-specific separator
     */
    void NormalizeSeparators();
    
    /**
     * @brief Get platform-specific path separator
     * @return '\\' on Windows, '/' on other platforms
     */
    static char GetSeparator();
    
    /**
     * @brief Check if character is a path separator (/ or \)
     * @param c Character to check
     * @return true if character is a separator
     */
    static bool IsSeparator(char c);
};

} // namespace ului

#endif // PATH_H
