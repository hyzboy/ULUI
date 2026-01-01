#pragma once

#include "LogLevel.h"
#include <string>
#include <vector>
#include <source_location>
#include <typeinfo>
#include <format>
#include <cstdarg>
#include <cstring>

/**
 * 对象日志输出类（使用C++20 STL容器）
 */
class ObjectLogger
{
    const std::type_info*   object_type_info;       // 对象数据类型
    std::string             object_type_name;       // 类型名称
    std::string             object_instance_name;   // 实例名称
    
    std::vector<char>       log_buffer;             // 日志缓冲区

public:
    ObjectLogger();
    ObjectLogger(const std::type_info* type_info);
    ObjectLogger(const char* name);
    
    virtual ~ObjectLogger() = default;

    const std::string& GetLoggerTypeName() const
    {
        return object_type_name;
    }

    size_t GetLoggerTypeHash() const
    {
        return object_type_info->hash_code();
    }

    void SetLoggerInstanceName(const std::string& name)
    {
        object_instance_name = name;
    }

    const std::string& GetLoggerInstanceName() const
    {
        return object_instance_name;
    }

public:
    // 核心日志方法
    void LogString(const std::source_location& loc, LogLevel level, const char* str, size_t len);
    void LogPrintf(const std::source_location& loc, LogLevel level, const char* fmt, va_list args);

    // C++20 std::format 版本（推荐）
    template<typename... Args>
    void LogFormat(const std::source_location& loc, LogLevel level, std::format_string<Args...> fmt, Args&&... args)
    {
        try {
            std::string message = std::format(fmt, std::forward<Args>(args)...);
            LogString(loc, level, message.c_str(), message.length());
        } catch (const std::format_error& e) {
            LogString(loc, LogLevel::Error, e.what(), std::strlen(e.what()));
        }
    }

    // 便捷日志方法（使用宏生成）
    // Note: Template version has priority for type safety
#define LOG_METHOD_DECL(LEVEL_NAME) \
    template<typename... Args> \
    requires (sizeof...(Args) > 0) \
    void LEVEL_NAME(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args) \
    { \
        LogFormat(loc, LogLevel::LEVEL_NAME, fmt, std::forward<Args>(args)...); \
    } \
    void LEVEL_NAME(const std::source_location& loc, const char* msg) \
    { \
        if (!msg || !*msg) return; \
        LogString(loc, LogLevel::LEVEL_NAME, msg, std::strlen(msg)); \
    }

    LOG_METHOD_DECL(Verbose)
    LOG_METHOD_DECL(Debug)
    LOG_METHOD_DECL(Info)
    LOG_METHOD_DECL(Notice)
    LOG_METHOD_DECL(Warning)
    LOG_METHOD_DECL(Error)
    LOG_METHOD_DECL(Fatal)

#undef LOG_METHOD_DECL
};

// 全局日志对象
extern ObjectLogger GlobalLogger;

// 类成员日志宏
#define OBJECT_LOGGER ObjectLogger Log{&typeid(*this)};

#ifdef _DEBUG
    #define LogVerbose(...) this->Log.Verbose(std::source_location::current(), __VA_ARGS__)
    #define LogDebug(...)   this->Log.Debug(std::source_location::current(), __VA_ARGS__)
#else
    #define LogVerbose(...) ((void)0)
    #define LogDebug(...)   ((void)0)
#endif

#define LogInfo(...)    this->Log.Info(std::source_location::current(), __VA_ARGS__)
#define LogNotice(...)  this->Log.Notice(std::source_location::current(), __VA_ARGS__)
#define LogWarning(...) this->Log.Warning(std::source_location::current(), __VA_ARGS__)
#define LogError(...)   this->Log.Error(std::source_location::current(), __VA_ARGS__)
#define LogFatal(...)   this->Log.Fatal(std::source_location::current(), __VA_ARGS__)

// 全局日志宏
#define GLOBAL_LOG_VERBOSE(...) GlobalLogger.Verbose(std::source_location::current(), __VA_ARGS__)
#define GLOBAL_LOG_DEBUG(...)   GlobalLogger.Debug(std::source_location::current(), __VA_ARGS__)
#define GLOBAL_LOG_INFO(...)    GlobalLogger.Info(std::source_location::current(), __VA_ARGS__)
#define GLOBAL_LOG_NOTICE(...)  GlobalLogger.Notice(std::source_location::current(), __VA_ARGS__)
#define GLOBAL_LOG_WARNING(...) GlobalLogger.Warning(std::source_location::current(), __VA_ARGS__)
#define GLOBAL_LOG_ERROR(...)   GlobalLogger.Error(std::source_location::current(), __VA_ARGS__)
#define GLOBAL_LOG_FATAL(...)   GlobalLogger.Fatal(std::source_location::current(), __VA_ARGS__)
