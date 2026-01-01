cmake_minimum_required(VERSION 3.20)

# Common source files
set(SOURCES
    main.cpp
    triangle_app.cpp
    file_system.cpp
    path.cpp
    logger_common.cpp
    object.cpp
    log/ObjectLogger.cpp
    gl/ShaderProgram.cpp
    gl/VBO.cpp
    gl/VAO.cpp
    gl/UBO.cpp
    gl/SSBO.cpp
    gl/Texture2D.cpp
    gl/FBO.cpp
    gl/RenderTarget.cpp
    Bitmap.cpp
    BitmapFormat.cpp
)

# Add Android MediaCodec sources if building for Android
if(PLATFORM_ANDROID)
    list(APPEND SOURCES
        android/MediaCodecEncoder.cpp
        android/MediaCodecDecoder.cpp
        android/Camera2Manager.cpp
    )
endif()
    
# Platform-specific logger implementation
if(WIN32)
    list(APPEND SOURCES logger_windows.cpp)
elseif(ANDROID)
    list(APPEND SOURCES logger_android.cpp)
elseif(APPLE)
    list(APPEND SOURCES logger_apple.cpp)
else()
    # Linux and other Unix-like systems
    list(APPEND SOURCES logger_unix.cpp)
endif()

# Create executable or shared library based on platform
if(PLATFORM_ANDROID)
    # For Android, always create a shared library
    add_library(ului_app SHARED ${SOURCES})
else()
    # For other platforms, create an executable
    add_executable(ului_app ${SOURCES})
endif()

# Link libraries
if(PLATFORM_IOS OR PLATFORM_ANDROID)
    # For mobile platforms, link native libraries
    if(PLATFORM_ANDROID)
        # Add native_app_glue for Android shared library
        add_library(native_app_glue STATIC
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
        )
        target_include_directories(native_app_glue PUBLIC
            ${ANDROID_NDK}/sources/android/native_app_glue
        )
        target_link_libraries(native_app_glue PUBLIC log)
        
        target_link_libraries(ului_app PRIVATE 
            angle_egl
            native_app_glue
            android
            log
            EGL
            GLESv3
            mediandk
            camera2ndk
        )
    elseif(PLATFORM_IOS)
        target_link_libraries(ului_app PRIVATE 
            angle_egl
            "-framework Foundation"
            "-framework UIKit"
            "-framework QuartzCore"
        )
    endif()
else()
    # For desktop platforms, use GLFW
    target_link_libraries(ului_app PRIVATE 
        glfw
        angle_egl
    )
    
    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(ului_app PRIVATE gdi32 user32 shell32 ws2_32)
    elseif(APPLE)
        target_link_libraries(ului_app PRIVATE 
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
        )
    elseif(UNIX)
        find_package(X11 REQUIRED)
        target_link_libraries(ului_app PRIVATE ${X11_LIBRARIES} ${CMAKE_DL_LIBS})
    endif()
endif()

# Include directories
target_include_directories(ului_app PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/angle/include
)

# Compile definitions
target_compile_definitions(ului_app PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Copy shaders to build directory
# For desktop platforms, copy to assets subdirectory
# For mobile platforms, they will be in the app bundle/APK assets
if(NOT PLATFORM_ANDROID AND NOT PLATFORM_IOS)
    add_custom_command(TARGET ului_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:ului_app>/assets/shaders
        COMMENT "Copying shaders to assets directory"
    )
endif()
