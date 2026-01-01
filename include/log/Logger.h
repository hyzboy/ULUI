#pragma once

#include "LogLevel.h"

struct LogMessage;

/**
 * 日志输出基类
 */
class Logger
{
protected:
    LogLevel    min_level;      // 最小输出级别
    Logger*     parent;         // 上级输出器（责任链模式）

public:
    Logger(LogLevel level, Logger* parent_logger = nullptr)
        : min_level(level), parent(parent_logger)
    {}
    
    virtual ~Logger() = default;

    LogLevel GetLevel() const { return min_level; }
    Logger* GetParent() { return parent; }

    virtual void Close() = 0;
    virtual void Write(const LogMessage* msg) = 0;
};

bool InitLogger(const std::string& app_name);
