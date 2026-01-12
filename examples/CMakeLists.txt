cmake_minimum_required(VERSION 3.20)

project(LogExample VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/../include)

# Source files
add_executable(log_example 
    log_example.cpp
    ../src/log/ObjectLogger.cpp
)

# Compile definitions
target_compile_definitions(log_example PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Comprehensive test
add_executable(comprehensive_log_test
    comprehensive_log_test.cpp
    ../src/log/ObjectLogger.cpp
)

# Compile definitions
target_compile_definitions(comprehensive_log_test PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# ECS example
add_executable(ecs_example
    ecs_example.cpp
    ../src/ecs/ComponentManager.cpp
)

# Compile definitions
target_compile_definitions(ecs_example PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
