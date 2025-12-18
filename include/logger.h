#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>

namespace Logger {

// Log levels (similar to Android)
enum class LogLevel {
    VERBOSE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    FATAL = 5
};

// Forward declarations
class LogOutput;

// Main Logger class
class Log {
public:
    // Initialize logger with default settings
    static void Initialize();
    
    // Shutdown logger and cleanup resources
    static void Shutdown();
    
    // Add/remove output targets
    static void AddOutput(std::shared_ptr<LogOutput> output);
    static void RemoveOutput(std::shared_ptr<LogOutput> output);
    static void ClearOutputs();
    
    // Set minimum log level (logs below this level will be ignored)
    static void SetMinLogLevel(LogLevel level);
    static LogLevel GetMinLogLevel();
    
    // Set tag filter (if set, only logs with this tag will be output)
    static void SetTagFilter(const char* tag);
    static void ClearTagFilter();
    
    // Main logging functions (Android-style)
    static void V(const char* tag, const char* format, ...);  // Verbose
    static void D(const char* tag, const char* format, ...);  // Debug
    static void I(const char* tag, const char* format, ...);  // Info
    static void W(const char* tag, const char* format, ...);  // Warning
    static void E(const char* tag, const char* format, ...);  // Error
    static void F(const char* tag, const char* format, ...);  // Fatal
    
    // Generic log function
    static void Write(LogLevel level, const char* tag, const char* format, ...);
    
    // Helper methods (public for output targets to use)
    static const char* GetLevelString(LogLevel level);
    static std::string GetTimestamp();
    
private:
    static void LogInternal(LogLevel level, const char* tag, const char* message);
    static std::string FormatMessage(const char* format, va_list args);
};

// Base class for log output targets
class LogOutput {
public:
    virtual ~LogOutput() = default;
    
    // Write log entry to output target
    virtual void Write(LogLevel level, const char* tag, const char* message) = 0;
    
    // Flush any buffered output
    virtual void Flush() = 0;
    
    // Enable/disable this output
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }
    
protected:
    bool m_enabled = true;
};

// Console output (stdout/stderr)
class ConsoleOutput : public LogOutput {
public:
    ConsoleOutput(bool colorized = true);
    
    void Write(LogLevel level, const char* tag, const char* message) override;
    void Flush() override;
    
private:
    bool m_colorized;
    std::mutex m_mutex;
    
    const char* GetColorCode(LogLevel level);
};

// File output
class FileOutput : public LogOutput {
public:
    // Constructor with file path
    explicit FileOutput(const char* filepath, bool append = false);
    ~FileOutput() override;
    
    void Write(LogLevel level, const char* tag, const char* message) override;
    void Flush() override;
    
    // Rotate log file when it exceeds maxSize (in bytes)
    void SetMaxFileSize(size_t maxSize);
    void SetMaxBackupFiles(int count);
    
private:
    std::string m_filepath;
    FILE* m_file = nullptr;
    size_t m_maxFileSize = 10 * 1024 * 1024;  // 10MB default
    int m_maxBackupFiles = 5;
    size_t m_currentSize = 0;
    std::mutex m_mutex;
    
    void OpenFile(bool append);
    void CloseFile();
    void RotateFile();
};

// Network output (UDP)
class NetworkOutput : public LogOutput {
public:
    NetworkOutput(const char* host, uint16_t port);
    ~NetworkOutput() override;
    
    void Write(LogLevel level, const char* tag, const char* message) override;
    void Flush() override;
    
private:
    std::string m_host;
    uint16_t m_port;
    int m_socket = -1;
    std::mutex m_mutex;
    
    bool InitializeSocket();
    void CloseSocket();
};

// Named pipe output (IPC on local machine)
class PipeOutput : public LogOutput {
public:
    explicit PipeOutput(const char* pipeName);
    ~PipeOutput() override;
    
    void Write(LogLevel level, const char* tag, const char* message) override;
    void Flush() override;
    
private:
    std::string m_pipeName;
#ifdef _WIN32
    void* m_pipe = nullptr;  // HANDLE
#else
    int m_pipe = -1;
#endif
    std::mutex m_mutex;
    
    bool OpenPipe();
    void ClosePipe();
};

#ifdef __ANDROID__
// Android logcat output (uses __android_log_write)
class AndroidOutput : public LogOutput {
public:
    AndroidOutput() = default;
    
    void Write(LogLevel level, const char* tag, const char* message) override;
    void Flush() override;
    
private:
    int GetAndroidPriority(LogLevel level);
};
#endif

#ifdef __APPLE__
// iOS/macOS unified logging output (uses os_log)
class AppleOutput : public LogOutput {
public:
    AppleOutput();
    ~AppleOutput() override;
    
    void Write(LogLevel level, const char* tag, const char* message) override;
    void Flush() override;
    
private:
    void* m_osLog = nullptr;  // os_log_t
    
    int GetOSLogType(LogLevel level);
};
#endif

} // namespace Logger

// Convenience macros
#define LOG_V(tag, ...) Logger::Log::V(tag, __VA_ARGS__)
#define LOG_D(tag, ...) Logger::Log::D(tag, __VA_ARGS__)
#define LOG_I(tag, ...) Logger::Log::I(tag, __VA_ARGS__)
#define LOG_W(tag, ...) Logger::Log::W(tag, __VA_ARGS__)
#define LOG_E(tag, ...) Logger::Log::E(tag, __VA_ARGS__)
#define LOG_F(tag, ...) Logger::Log::F(tag, __VA_ARGS__)
