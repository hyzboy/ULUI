#pragma once

#include <string>
#include <source_location>
#include <typeinfo>
#include "LogLevel.h"

struct LogMessage
{
    const std::type_info*       object_type_info;       // 对象数据类型的type_info指针
    std::string                 object_instance_name;   // 对象实例名称
    std::source_location        source_location;        // 源码位置（C++20）
    LogLevel                    level;                  // 日志级别
    std::string                 message;                // 消息内容
};
