#include "log/ObjectLogger.h"
#include "log/LogMessage.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

// 全局日志对象定义
ObjectLogger GlobalLogger;

ObjectLogger::ObjectLogger()
    : object_type_info(&typeid(void))
    , object_type_name("GlobalLogger")
{
    log_buffer.reserve(4096);
}

ObjectLogger::ObjectLogger(const std::type_info* type_info)
    : object_type_info(type_info)
{
    if (type_info) {
        object_type_name = type_info->name();
    }
    log_buffer.reserve(4096);
}

ObjectLogger::ObjectLogger(const char* name)
    : object_type_info(&typeid(void))
    , object_type_name(name ? name : "Unknown")
{
    log_buffer.reserve(4096);
}

void ObjectLogger::LogString(const std::source_location& loc, LogLevel level, const char* str, size_t len)
{
    if (!str || len == 0) return;

    LogMessage msg;
    msg.object_type_info = object_type_info;
    msg.object_instance_name = object_instance_name;
    msg.source_location = loc;
    msg.level = level;
    msg.message.assign(str, len);

    // TODO: 调用实际的 Logger 输出
    // 这里需要一个全局的 Logger 管理器来分发日志
    // DefaultLoggerManager::Write(&msg);
    
    // 临时实现：直接输出到控制台
    const char* level_str[] = {"VERBOSE", "DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "FATAL"};
    fprintf(stderr, "[%s][%s:%u][%s] %s\n",
            level_str[static_cast<int>(level)],
            loc.file_name(),
            loc.line(),
            object_type_name.c_str(),
            msg.message.c_str());
}

void ObjectLogger::LogPrintf(const std::source_location& loc, LogLevel level, const char* fmt, va_list args)
{
    if (!fmt || !*fmt) return;

    // 首次尝试使用固定缓冲区
    log_buffer.resize(1024);
    
    va_list args_copy;
    va_copy(args_copy, args);
    
    int result = std::vsnprintf(log_buffer.data(), log_buffer.size(), fmt, args);
    
    if (result < 0) {
        va_end(args_copy);
        return;
    }

    // 如果缓冲区不够，重新分配
    if (static_cast<size_t>(result) >= log_buffer.size()) {
        log_buffer.resize(result + 1);
        result = std::vsnprintf(log_buffer.data(), log_buffer.size(), fmt, args_copy);
    }
    
    va_end(args_copy);

    if (result > 0) {
        LogString(loc, level, log_buffer.data(), static_cast<size_t>(result));
    }
}
