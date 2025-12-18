#include "object.h"
#include <cstdarg>
#include <cstring>

namespace ului {

Object::Object(const char* tag)
    : m_tag(tag ? tag : "")
    , m_className(ExtractClassName(typeid(*this)))
{
}

std::string Object::ExtractClassName(const std::type_info& typeInfo)
{
    const char* name = typeInfo.name();
    std::string result = name;
    
#ifdef __GNUC__
    // GCC/Clang mangles names, try to demangle
    // For simple cases, just strip leading digits and namespace
    const char* start = name;
    
    // Skip leading numbers (GCC prefixes with length)
    while (*start >= '0' && *start <= '9') {
        start++;
    }
    
    // Look for the last part after namespace separator
    const char* lastPart = start;
    for (const char* p = start; *p; p++) {
        if (*p == ':' && *(p+1) == ':') {
            lastPart = p + 2;
        }
    }
    
    if (*lastPart) {
        result = lastPart;
    }
#elif defined(_MSC_VER)
    // MSVC includes "class " prefix
    if (result.find("class ") == 0) {
        result = result.substr(6);
    }
    
    // Remove namespace if present
    size_t lastColon = result.rfind("::");
    if (lastColon != std::string::npos) {
        result = result.substr(lastColon + 2);
    }
#endif
    
    return result;
}

void Object::LogInternal(Logger::LogLevel level, const char* format, va_list args) const
{
    // Format the user message with bounds checking
    char buffer[4096];  // Increased buffer size for longer messages
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    
    // Check if message was truncated
    if (result >= static_cast<int>(sizeof(buffer))) {
        // Message was truncated, indicate this in the buffer
        const char* truncated = "... [TRUNCATED]";
        size_t truncLen = strlen(truncated);
        if (sizeof(buffer) > truncLen) {
            strcpy(buffer + sizeof(buffer) - truncLen - 1, truncated);
        }
    }
    
    // Create the full tag with [ClassName][TAG] format
    // Limit class name and tag lengths to prevent overflow
    std::string fullTag = "[";
    fullTag += (m_className.length() > 128 ? m_className.substr(0, 128) : m_className);
    fullTag += "][";
    fullTag += (m_tag.length() > 128 ? m_tag.substr(0, 128) : m_tag);
    fullTag += "]";
    
    // Use the logger's write function
    Logger::Log::Write(level, fullTag.c_str(), "%s", buffer);
}

void Object::LogV(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    LogInternal(Logger::LogLevel::VERBOSE, format, args);
    va_end(args);
}

void Object::LogD(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    LogInternal(Logger::LogLevel::DEBUG, format, args);
    va_end(args);
}

void Object::LogI(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    LogInternal(Logger::LogLevel::INFO, format, args);
    va_end(args);
}

void Object::LogW(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    LogInternal(Logger::LogLevel::WARNING, format, args);
    va_end(args);
}

void Object::LogE(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    LogInternal(Logger::LogLevel::ERROR, format, args);
    va_end(args);
}

void Object::LogF(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    LogInternal(Logger::LogLevel::FATAL, format, args);
    va_end(args);
}

} // namespace ului
