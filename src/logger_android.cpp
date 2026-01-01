#include "logger.h"
#include "file_system.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <android/log.h>

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
        // Android fallback
        baseDir = ului::FileSystem::GetAppDataDirectory();
        if (baseDir.empty()) {
            baseDir = "./";
        } else {
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
    // Android: Add logcat output
    auto androidOutput = std::make_shared<AndroidOutput>();
    g_outputs.push_back(androidOutput);
}

// ConsoleOutput implementation
ConsoleOutput::ConsoleOutput(bool colorized) : m_colorized(colorized) {
    // Android systems support ANSI colors by default
}

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
