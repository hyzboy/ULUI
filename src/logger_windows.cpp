#include "logger.h"
#include "file_system.h"
#include <cstring>
#include <chrono>
#include <windows.h>
#include <io.h>

namespace Logger {

// External references to global state
extern std::vector<std::shared_ptr<LogOutput>> g_outputs;

// Helper function to create directory if it doesn't exist
bool CreateDirectoryIfNeeded(const std::string& path) {
    // Windows: use CreateDirectoryA
    DWORD attrs = GetFileAttributesA(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        // Directory doesn't exist, create it
        if (CreateDirectoryA(path.c_str(), NULL) != 0) {
            return true;
        }
        return GetLastError() == ERROR_ALREADY_EXISTS;
    }
    return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// Helper function to get log directory path
std::string GetLogDirectory() {
    std::string baseDir = ului::FileSystem::GetExternalStorageDirectory();
    
    // If GetExternalStorageDirectory returns empty (typically on desktop platforms
    // or when external storage is unavailable), fall back to other appropriate directories
    if (baseDir.empty()) {
        // Use local app data for Windows
        baseDir = ului::FileSystem::GetLocalAppDataDirectory();
        if (!baseDir.empty()) {
            baseDir += "ului\\";
            CreateDirectoryIfNeeded(baseDir);
        }
    }
    
    // Add Log subdirectory
    std::string logDir = baseDir + "Log\\";
    
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
    // Windows: use localtime_s
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
    
    std::string logFile = logDir + "ului_" + buffer + ".log";
    return logFile;
}

// Initialize platform-specific output
void InitializePlatformOutput() {
    // Windows: Add console output
    auto consoleOutput = std::make_shared<ConsoleOutput>();
    g_outputs.push_back(consoleOutput);
}

// ConsoleOutput implementation
ConsoleOutput::ConsoleOutput(bool colorized) : m_colorized(colorized) {
    if (m_colorized) {
        // Enable ANSI color codes on Windows 10+
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
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
    
    send(m_socket, logLine.c_str(), (int)logLine.size(), 0);
}

void NetworkOutput::Flush() {
    // UDP doesn't need flushing
}

bool NetworkOutput::InitializeSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
    
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
        closesocket(m_socket);
        WSACleanup();
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
    
    if (!m_pipe) return;
    
    std::string logLine = Log::GetTimestamp() + "/" + 
                         Log::GetLevelString(level) + " " +
                         tag + ": " + message + "\n";
    
    DWORD written;
    WriteFile(m_pipe, logLine.c_str(), (DWORD)logLine.size(), &written, nullptr);
}

void PipeOutput::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pipe) {
        FlushFileBuffers(m_pipe);
    }
}

bool PipeOutput::OpenPipe() {
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
}

void PipeOutput::ClosePipe() {
    if (m_pipe && m_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipe);
        m_pipe = nullptr;
    }
}

} // namespace Logger
