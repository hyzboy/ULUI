#include "logger.h"
#include "file_system.h"
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <os/log.h>

namespace Logger {

// External references to global state
extern std::vector<std::shared_ptr<LogOutput>> g_outputs;

// Helper function to create directory if it doesn't exist
bool CreateDirectoryIfNeeded(const std::string& path) {
    // Unix-like systems
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    // Directory doesn't exist, create it
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

// Helper function to get log directory path
std::string GetLogDirectory() {
    std::string baseDir = ului::FileSystem::GetExternalStorageDirectory();
    
    // If GetExternalStorageDirectory returns empty (typically on desktop platforms
    // or when external storage is unavailable), fall back to other appropriate directories
    if (baseDir.empty()) {
        // Use app data directory for macOS/iOS
        baseDir = ului::FileSystem::GetAppDataDirectory();
        if (!baseDir.empty()) {
            baseDir += "ului/";
            CreateDirectoryIfNeeded(baseDir);
        }
    }
    
    // Add Log subdirectory
    std::string logDir = baseDir + "Log/";
    
    return logDir;
}

// Helper function to get log file path
std::string GetLogFilePath() {
    std::string logDir = GetLogDirectory();
    
    // Create the Log directory if it doesn't exist
    CreateDirectoryIfNeeded(logDir);
    
    // Generate log filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    char buffer[64];
    // Unix: use localtime_r
    struct tm timeinfo;
    localtime_r(&time_t, &timeinfo);
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
    
    std::string logFile = logDir + "ului_" + buffer + ".log";
    return logFile;
}

// Initialize platform-specific output
void InitializePlatformOutput() {
    // Apple: Add unified logging
    auto appleOutput = std::make_shared<AppleOutput>();
    g_outputs.push_back(appleOutput);
}

// ConsoleOutput implementation
ConsoleOutput::ConsoleOutput(bool colorized) : m_colorized(colorized) {
    // Apple systems support ANSI colors by default
}

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
    
    send(m_socket, logLine.c_str(), logLine.size(), 0);
}

void NetworkOutput::Flush() {
    // UDP doesn't need flushing
}

bool NetworkOutput::InitializeSocket() {
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
        close(m_socket);
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
    
    if (m_pipe < 0) return;
    
    std::string logLine = Log::GetTimestamp() + "/" +
                         Log::GetLevelString(level) + " " +
                         tag + ": " + message + "\n";
    
    ssize_t written = write(m_pipe, logLine.c_str(), logLine.size());
    (void)written;  // Suppress unused warning
}

void PipeOutput::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pipe >= 0) {
        fsync(m_pipe);
    }
}

bool PipeOutput::OpenPipe() {
    m_pipe = open(m_pipeName.c_str(), O_WRONLY | O_NONBLOCK);
    return m_pipe >= 0;
}

void PipeOutput::ClosePipe() {
    if (m_pipe >= 0) {
        close(m_pipe);
        m_pipe = -1;
    }
}

} // namespace Logger
