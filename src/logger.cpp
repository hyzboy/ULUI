#include "logger.h"
#include "file_system.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __APPLE__
#include <os/log.h>
#endif

namespace Logger {

// Static members
static std::vector<std::shared_ptr<LogOutput>> g_outputs;
static std::mutex g_outputsMutex;
static LogLevel g_minLogLevel = LogLevel::VERBOSE;
static std::string g_tagFilter;
static bool g_initialized = false;

// Helper function to create directory if it doesn't exist
static bool CreateDirectoryIfNeeded(const std::string& path) {
#ifdef _WIN32
    // Windows: use CreateDirectoryA
    DWORD attrs = GetFileAttributesA(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        // Directory doesn't exist, create it
        return CreateDirectoryA(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
    }
    return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    // Unix-like systems
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    // Directory doesn't exist, create it
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

// Helper function to get log directory path
static std::string GetLogDirectory() {
    std::string baseDir = ului::FileSystem::GetExternalStorageDirectory();
    
    // If GetExternalStorageDirectory returns empty (desktop platforms),
    // fall back to other appropriate directories
    if (baseDir.empty()) {
#ifdef _WIN32
        // Use local app data for Windows
        baseDir = ului::FileSystem::GetLocalAppDataDirectory();
        if (!baseDir.empty()) {
            baseDir += "ULUI\\";
            CreateDirectoryIfNeeded(baseDir);
        }
#elif defined(__APPLE__)
        // Use app data directory for macOS/iOS
        baseDir = ului::FileSystem::GetAppDataDirectory();
        if (!baseDir.empty()) {
            baseDir += "ULUI/";
            CreateDirectoryIfNeeded(baseDir);
        }
#else
        // Linux and other Unix-like systems
        baseDir = ului::FileSystem::GetAppDataDirectory();
        if (baseDir.empty()) {
            baseDir = "./";
        } else {
            baseDir += "ului/";
            CreateDirectoryIfNeeded(baseDir);
        }
#endif
    }
    
    // Add Log subdirectory
    std::string logDir = baseDir + "Log";
#ifdef _WIN32
    logDir += "\\";
#else
    logDir += "/";
#endif
    
    return logDir;
}

// Helper function to get log file path
static std::string GetLogFilePath() {
    std::string logDir = GetLogDirectory();
    
    // Create the Log directory if it doesn't exist
    CreateDirectoryIfNeeded(logDir);
    
    // Generate log filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&time_t));
    
    std::string logFile = logDir + "ului_" + buffer + ".log";
    return logFile;
}

// Log implementation
void Log::Initialize() {
    if (g_initialized) return;
    
    std::lock_guard<std::mutex> lock(g_outputsMutex);
    
    // Get log file path
    std::string logFilePath = GetLogFilePath();
    
#ifdef _WIN32
    // Windows: Add console output + file output
    auto consoleOutput = std::make_shared<ConsoleOutput>();
    g_outputs.push_back(consoleOutput);
    
    auto fileOutput = std::make_shared<FileOutput>(logFilePath.c_str(), false);
    g_outputs.push_back(fileOutput);
    
#elif defined(__ANDROID__)
    // Android: Add logcat output + file output
    auto androidOutput = std::make_shared<AndroidOutput>();
    g_outputs.push_back(androidOutput);
    
    auto fileOutput = std::make_shared<FileOutput>(logFilePath.c_str(), false);
    g_outputs.push_back(fileOutput);
    
#elif defined(__APPLE__)
    // Apple: Add unified logging + file output
    auto appleOutput = std::make_shared<AppleOutput>();
    g_outputs.push_back(appleOutput);
    
    auto fileOutput = std::make_shared<FileOutput>(logFilePath.c_str(), false);
    g_outputs.push_back(fileOutput);
    
#else
    // Other Unix-like systems: Add console output + file output
    auto consoleOutput = std::make_shared<ConsoleOutput>();
    g_outputs.push_back(consoleOutput);
    
    auto fileOutput = std::make_shared<FileOutput>(logFilePath.c_str(), false);
    g_outputs.push_back(fileOutput);
#endif
    
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

// ConsoleOutput implementation
ConsoleOutput::ConsoleOutput(bool colorized) : m_colorized(colorized) {
#ifdef _WIN32
    if (m_colorized) {
        // Enable ANSI color codes on Windows 10+
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= 0x0004;  // ENABLE_VIRTUAL_TERMINAL_PROCESSING
        SetConsoleMode(hOut, dwMode);
    }
#endif
}

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

// NetworkOutput implementation
NetworkOutput::NetworkOutput(const char* host, uint16_t port)
    : m_host(host), m_port(port) {
    InitializeSocket();
}

NetworkOutput::~NetworkOutput() {
    CloseSocket();
}

void NetworkOutput::Write(LogLevel level, const char* tag, const char* message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_socket < 0) return;
    
    std::string logLine = Log::GetTimestamp() + "/" + 
                         Log::GetLevelString(level) + " " +
                         tag + ": " + message + "\n";
    
#ifdef _WIN32
    send(m_socket, logLine.c_str(), (int)logLine.size(), 0);
#else
    send(m_socket, logLine.c_str(), logLine.size(), 0);
#endif
}

void NetworkOutput::Flush() {
    // UDP doesn't need flushing
}

bool NetworkOutput::InitializeSocket() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif
    
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket < 0) {
        return false;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = inet_addr(m_host.c_str());
    
    if (connect(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        CloseSocket();
        return false;
    }
    
    return true;
}

void NetworkOutput::CloseSocket() {
    if (m_socket >= 0) {
#ifdef _WIN32
        closesocket(m_socket);
        WSACleanup();
#else
        close(m_socket);
#endif
        m_socket = -1;
    }
}

// PipeOutput implementation
PipeOutput::PipeOutput(const char* pipeName) : m_pipeName(pipeName) {
    OpenPipe();
}

PipeOutput::~PipeOutput() {
    ClosePipe();
}

void PipeOutput::Write(LogLevel level, const char* tag, const char* message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
#ifdef _WIN32
    if (!m_pipe) return;
    
    std::string logLine = Log::GetTimestamp() + "/" + 
                         Log::GetLevelString(level) + " " +
                         tag + ": " + message + "\n";
    
    DWORD written;
    WriteFile(m_pipe, logLine.c_str(), (DWORD)logLine.size(), &written, nullptr);
#else
    if (m_pipe < 0) return;
    
    std::string logLine = Log::GetTimestamp() + "/" +
                         Log::GetLevelString(level) + " " +
                         tag + ": " + message + "\n";
    
    ssize_t written = write(m_pipe, logLine.c_str(), logLine.size());
    (void)written;  // Suppress unused warning
#endif
}

void PipeOutput::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
#ifdef _WIN32
    if (m_pipe) {
        FlushFileBuffers(m_pipe);
    }
#else
    if (m_pipe >= 0) {
        fsync(m_pipe);
    }
#endif
}

bool PipeOutput::OpenPipe() {
#ifdef _WIN32
    std::string pipePath = "\\\\.\\pipe\\" + m_pipeName;
    m_pipe = CreateFileA(
        pipePath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );
    return m_pipe != INVALID_HANDLE_VALUE;
#else
    m_pipe = open(m_pipeName.c_str(), O_WRONLY | O_NONBLOCK);
    return m_pipe >= 0;
#endif
}

void PipeOutput::ClosePipe() {
#ifdef _WIN32
    if (m_pipe && m_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipe);
        m_pipe = nullptr;
    }
#else
    if (m_pipe >= 0) {
        close(m_pipe);
        m_pipe = -1;
    }
#endif
}

#ifdef __ANDROID__
// AndroidOutput implementation
void AndroidOutput::Write(LogLevel level, const char* tag, const char* message) {
    __android_log_write(GetAndroidPriority(level), tag, message);
}

void AndroidOutput::Flush() {
    // Android logcat doesn't need flushing
}

int AndroidOutput::GetAndroidPriority(LogLevel level) {
    switch (level) {
        case LogLevel::VERBOSE: return ANDROID_LOG_VERBOSE;
        case LogLevel::DEBUG:   return ANDROID_LOG_DEBUG;
        case LogLevel::INFO:    return ANDROID_LOG_INFO;
        case LogLevel::WARNING: return ANDROID_LOG_WARN;
        case LogLevel::ERROR:   return ANDROID_LOG_ERROR;
        case LogLevel::FATAL:   return ANDROID_LOG_FATAL;
        default:                return ANDROID_LOG_UNKNOWN;
    }
}
#endif

#ifdef __APPLE__
// AppleOutput implementation
AppleOutput::AppleOutput() {
    m_osLog = (void*)os_log_create("com.ului.app", "default");
}

AppleOutput::~AppleOutput() {
    // os_log_t is managed by the system, no need to release
    m_osLog = nullptr;
}

void AppleOutput::Write(LogLevel level, const char* tag, const char* message) {
    if (!m_osLog) return;
    
    os_log_t log = (os_log_t)m_osLog;
    os_log_type_t type = (os_log_type_t)GetOSLogType(level);
    
    os_log_with_type(log, type, "%s: %s", tag, message);
}

void AppleOutput::Flush() {
    // os_log doesn't need flushing
}

int AppleOutput::GetOSLogType(LogLevel level) {
    switch (level) {
        case LogLevel::VERBOSE: return OS_LOG_TYPE_DEBUG;
        case LogLevel::DEBUG:   return OS_LOG_TYPE_DEBUG;
        case LogLevel::INFO:    return OS_LOG_TYPE_INFO;
        case LogLevel::WARNING: return OS_LOG_TYPE_DEFAULT;
        case LogLevel::ERROR:   return OS_LOG_TYPE_ERROR;
        case LogLevel::FATAL:   return OS_LOG_TYPE_FAULT;
        default:                return OS_LOG_TYPE_DEFAULT;
    }
}
#endif

} // namespace Logger
