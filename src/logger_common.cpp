#include "logger.h"
#include "file_system.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <chrono>
#include <algorithm>

namespace Logger {

// Constants for log file configuration
static const char* LOG_DIR_NAME = "Log";
static const char* LOG_FILE_PREFIX = "ului_";
static const char* LOG_FILE_EXTENSION = ".log";

// Static members - g_outputs needs to be accessible from platform-specific files
std::vector<std::shared_ptr<LogOutput>> g_outputs;
static std::mutex g_outputsMutex;
static LogLevel g_minLogLevel = LogLevel::VERBOSE;
static std::string g_tagFilter;
static bool g_initialized = false;

// Forward declarations of platform-specific functions
bool CreateDirectoryIfNeeded(const std::string& path);
std::string GetLogDirectory();
std::string GetLogFilePath();
void InitializePlatformOutput();

// Log implementation
void Log::Initialize() {
    if (g_initialized) return;
    
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    
    // Add platform-specific output
    InitializePlatformOutput();
    
    // Add file output for all platforms
    // Note: append parameter is set to false, so log files are created fresh on each initialization
    // This prevents log files from growing indefinitely across application sessions
    std::string logFilePath = GetLogFilePath();
    auto fileOutput = std::make_shared<FileOutput>(logFilePath.c_str(), false);
    g_outputs.push_back(fileOutput);
    
    g_initialized = true;
}

void Log::Shutdown() {
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    
    // Flush all outputs
    for (auto& output : g_outputs) {
        output->Flush();
    }
    
    g_outputs.clear();
    g_initialized = false;
}

void Log::AddOutput(std::shared_ptr<LogOutput> output) {
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    g_outputs.push_back(output);
}

void Log::RemoveOutput(std::shared_ptr<LogOutput> output) {
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    g_outputs.erase(
        std::remove(g_outputs.begin(), g_outputs.end(), output),
        g_outputs.end()
    );
}

void Log::ClearOutputs() {
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    g_outputs.clear();
}

void Log::SetMinLogLevel(LogLevel level) {
    g_minLogLevel = level;
}

LogLevel Log::GetMinLogLevel() {
    return g_minLogLevel;
}

void Log::SetTagFilter(const char* tag) {
    g_tagFilter = tag ? tag : "";
}

void Log::ClearTagFilter() {
    g_tagFilter.clear();
}

void Log::V(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(LogLevel::VERBOSE, tag, message.c_str());
}

void Log::D(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(LogLevel::DEBUG, tag, message.c_str());
}

void Log::I(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(LogLevel::INFO, tag, message.c_str());
}

void Log::W(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(LogLevel::WARNING, tag, message.c_str());
}

void Log::E(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(LogLevel::ERROR, tag, message.c_str());
}

void Log::F(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(LogLevel::FATAL, tag, message.c_str());
}

void Log::Write(LogLevel level, const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatMessage(format, args);
    va_end(args);
    LogInternal(level, tag, message.c_str());
}

void Log::LogInternal(LogLevel level, const char* tag, const char* message) {
    if (!g_initialized) {
        Initialize();
    }
    
    // Check minimum log level
    if (level < g_minLogLevel) {
        return;
    }
    
    // Check tag filter
    if (!g_tagFilter.empty() && g_tagFilter != tag) {
        return;
    }
    
    // Write to all enabled outputs
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    for (auto& output : g_outputs) {
        if (output->IsEnabled()) {
            output->Write(level, tag, message);
        }
    }
}

std::string Log::FormatMessage(const char* format, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    
    int size = std::vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);
    
    if (size <= 0) {
        return "";
    }
    
    std::string result(size + 1, '\0');
    std::vsnprintf(&result[0], result.size(), format, args);
    result.resize(size);
    
    return result;
}

const char* Log::GetLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::VERBOSE: return "V";
        case LogLevel::DEBUG:   return "D";
        case LogLevel::INFO:    return "I";
        case LogLevel::WARNING: return "W";
        case LogLevel::ERROR:   return "E";
        case LogLevel::FATAL:   return "F";
        default:                return "?";
    }
}

std::string Log::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
    
    char result[80];
    std::snprintf(result, sizeof(result), "%s.%03d", buffer, (int)ms.count());
    
    return result;
}

// FileOutput implementation
FileOutput::FileOutput(const char* filepath, bool append) 
    : m_filepath(filepath) {
    OpenFile(append);
}

FileOutput::~FileOutput() {
    CloseFile();
}

void FileOutput::Write(LogLevel level, const char* tag, const char* message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_file) return;
    
    std::string logLine = Log::GetTimestamp() + "/" + 
                         Log::GetLevelString(level) + " " +
                         tag + ": " + message + "\n";
    
    size_t written = fwrite(logLine.c_str(), 1, logLine.size(), m_file);
    m_currentSize += written;
    
    // Check if rotation is needed
    if (m_currentSize >= m_maxFileSize) {
        RotateFile();
    }
}

void FileOutput::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file) {
        fflush(m_file);
    }
}

void FileOutput::SetMaxFileSize(size_t maxSize) {
    m_maxFileSize = maxSize;
}

void FileOutput::SetMaxBackupFiles(int count) {
    m_maxBackupFiles = count;
}

void FileOutput::OpenFile(bool append) {
    m_file = fopen(m_filepath.c_str(), append ? "a" : "w");
    if (m_file) {
        fseek(m_file, 0, SEEK_END);
        m_currentSize = ftell(m_file);
    }
}

void FileOutput::CloseFile() {
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
}

void FileOutput::RotateFile() {
    CloseFile();
    
    // Remove oldest backup
    if (m_maxBackupFiles > 0) {
        std::string oldestBackup = m_filepath + "." + std::to_string(m_maxBackupFiles);
        remove(oldestBackup.c_str());
        
        // Shift backup files
        for (int i = m_maxBackupFiles - 1; i >= 1; --i) {
            std::string oldName = m_filepath + "." + std::to_string(i);
            std::string newName = m_filepath + "." + std::to_string(i + 1);
            rename(oldName.c_str(), newName.c_str());
        }
        
        // Rename current file to .1
        std::string backup = m_filepath + ".1";
        rename(m_filepath.c_str(), backup.c_str());
    }
    
    // Open new file
    OpenFile(false);
    m_currentSize = 0;
}

// ConsoleOutput implementation - platform-agnostic parts
void ConsoleOutput::Write(LogLevel level, const char* tag, const char* message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    FILE* out = (level >= LogLevel::ERROR) ? stderr : stdout;
    
    if (m_colorized) {
        fprintf(out, "%s%s/%s %s: %s%s\n",
                GetColorCode(level),
                Log::GetTimestamp().c_str(),
                Log::GetLevelString(level),
                tag,
                message,
                "\033[0m");  // Reset color
    } else {
        fprintf(out, "%s/%s %s: %s\n",
                Log::GetTimestamp().c_str(),
                Log::GetLevelString(level),
                tag,
                message);
    }
}

void ConsoleOutput::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    fflush(stdout);
    fflush(stderr);
}

const char* ConsoleOutput::GetColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::VERBOSE: return "\033[37m";  // White
        case LogLevel::DEBUG:   return "\033[36m";  // Cyan
        case LogLevel::INFO:    return "\033[32m";  // Green
        case LogLevel::WARNING: return "\033[33m";  // Yellow
        case LogLevel::ERROR:   return "\033[31m";  // Red
        case LogLevel::FATAL:   return "\033[35m";  // Magenta
        default:                return "\033[0m";   // Reset
    }
}

} // namespace Logger
