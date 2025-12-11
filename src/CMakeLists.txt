cmake_minimum_required(VERSION 3.20)

# Source files
set(SOURCES
    main.cpp
    triangle_app.cpp
)

# Create executable
add_executable(ului_app ${SOURCES})

# Link libraries
if(PLATFORM_IOS OR PLATFORM_ANDROID)
    # For mobile platforms, link native libraries
    if(PLATFORM_ANDROID)
        target_link_libraries(ului_app PRIVATE 
            angle_egl
            android
            log
            EGL
            GLESv3
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
        target_link_libraries(ului_app PRIVATE gdi32 user32 shell32)
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
add_custom_command(TARGET ului_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:ului_app>/shaders
    COMMENT "Copying shaders to build directory"
)
