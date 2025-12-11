cmake_minimum_required(VERSION 3.20)

project(ULUI VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    if(IOS)
        set(PLATFORM_IOS TRUE)
    else()
        set(PLATFORM_MACOS TRUE)
    endif()
elseif(ANDROID)
    set(PLATFORM_ANDROID TRUE)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(USE_ANGLE "Use ANGLE for OpenGL ES implementation" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add external dependencies
add_subdirectory(external)

# Add source files
add_subdirectory(src)

# Installation rules
install(TARGETS ului_app
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
