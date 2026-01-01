#include "log/ObjectLogger.h"
#include <thread>
#include <chrono>

class MyComponent
{
    OBJECT_LOGGER  // 自动创建 this->Log 成员

public:
    MyComponent()
    {
        Log.SetLoggerInstanceName("Component-001");
    }

    void DoWork()
    {
        LogInfo("Starting work...");
        
        for (int i = 0; i < 5; ++i) {
            // C++20 std::format 方式（推荐）
            LogDebug("Processing item {}/{}", i + 1, 5);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // C++20 std::format 方式
        LogNotice("Work completed after {} iterations", 5);
        
        // 警告和错误
        if (true) {
            LogWarning("Memory usage: {:.2f}MB", 128.5);
        }
    }
};

int main()
{
    // 全局日志
    GLOBAL_LOG_INFO("Application started");
    
    MyComponent component;
    component.DoWork();
    
    GLOBAL_LOG_INFO("Application finished");
    
    return 0;
}
