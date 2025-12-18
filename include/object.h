#pragma once

#include "logger.h"
#include <string>
#include <typeinfo>

namespace ului {

/**
 * Base class for all functionality classes in the ULUI framework.
 * Provides automatic class name detection and TAG-based logging.
 * 
 * Log format: [LogLevel][ClassName][TAG]message
 * 
 * Example usage:
 *   class MyClass : public Object {
 *   public:
 *       MyClass() : Object("MyTag") {}
 *       void DoSomething() {
 *           LogV("Verbose message");
 *           LogD("Debug: value=%d", 42);
 *           LogI("Info message");
 *           LogW("Warning message");
 *           LogE("Error message");
 *           LogF("Fatal error");
 *       }
 *   };
 */
class Object {
public:
    /**
     * Constructor with TAG parameter
     * @param tag User-defined tag for this object instance
     */
    explicit Object(const char* tag);
    
    virtual ~Object() = default;
    
    /**
     * Get the TAG for this object
     */
    const char* GetTag() const { return m_tag.c_str(); }
    
    /**
     * Get the class type name for this object
     */
    const char* GetClassName() const { return m_className.c_str(); }

protected:
    /**
     * Logging methods with automatic class name and TAG prefix
     * Format: [LogLevel][ClassName][TAG]message
     */
    void LogV(const char* format, ...) const;  // Verbose
    void LogD(const char* format, ...) const;  // Debug
    void LogI(const char* format, ...) const;  // Info
    void LogW(const char* format, ...) const;  // Warning
    void LogE(const char* format, ...) const;  // Error
    void LogF(const char* format, ...) const;  // Fatal
    
private:
    std::string m_tag;
    std::string m_className;
    
    // Helper to extract simple class name from typeid
    static std::string ExtractClassName(const std::type_info& typeInfo);
    
    // Internal logging helper
    void LogInternal(Logger::LogLevel level, const char* format, va_list args) const;
};

} // namespace ului
