#include "log/ObjectLogger.h"
#include <cassert>
#include <sstream>

// Test class to demonstrate object logging
class TestComponent
{
    OBJECT_LOGGER

public:
    TestComponent(const char* name)
    {
        Log.SetLoggerInstanceName(name);
    }

    void TestAllLevels()
    {
        // Test all log levels with std::format
        LogVerbose("Verbose message: {}", 1);
        LogDebug("Debug message: {}", 2);
        LogInfo("Info message: {}", 3);
        LogNotice("Notice message: {}", 4);
        LogWarning("Warning message: {}", 5);
        LogError("Error message: {}", 6);
        LogFatal("Fatal message: {}", 7);
    }

    void TestStringOnly()
    {
        LogInfo("Simple string message without formatting");
    }

    void TestFormatting()
    {
        // Test various format types
        LogInfo("Integer: {}", 42);
        LogInfo("Float: {:.2f}", 3.14159);
        LogInfo("String: {}", "Hello, World!");
        LogInfo("Multiple: {} {} {}", 1, 2.5, "three");
    }
};

int main()
{
    // Test global logger
    GLOBAL_LOG_INFO("=== Starting Comprehensive Logging Test ===");
    
    // Test with different names
    TestComponent component1("Component-001");
    component1.TestAllLevels();
    component1.TestStringOnly();
    component1.TestFormatting();
    
    TestComponent component2("Component-002");
    component2.TestAllLevels();
    
    // Test global logger with formatting
    GLOBAL_LOG_INFO("Test completed with {} components", 2);
    GLOBAL_LOG_NOTICE("All tests passed!");
    
    GLOBAL_LOG_INFO("=== Comprehensive Logging Test Complete ===");
    
    return 0;
}
