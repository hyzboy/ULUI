cmake_minimum_required(VERSION 3.20)

# GLFW for window management (desktop platforms)
if(NOT PLATFORM_IOS AND NOT PLATFORM_ANDROID)
    include(FetchContent)
    
    # Fetch GLFW
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(glfw)
endif()

# ANGLE setup
if(USE_ANGLE)
    # For this example, we'll use system-provided ANGLE or assume it's installed
    # In a real project, you might want to build ANGLE from source or use prebuilt binaries
    
    # Try to find ANGLE libraries
    find_library(EGL_LIBRARY NAMES EGL libEGL)
    find_library(GLES_LIBRARY NAMES GLESv2 libGLESv2)
    
    if(EGL_LIBRARY AND GLES_LIBRARY)
        message(STATUS "Found ANGLE libraries: ${EGL_LIBRARY}, ${GLES_LIBRARY}")
        add_library(angle_egl INTERFACE)
        target_link_libraries(angle_egl INTERFACE ${EGL_LIBRARY} ${GLES_LIBRARY})
    else()
        message(STATUS "ANGLE libraries not found, will use headers only")
        # Create interface library for headers
        add_library(angle_egl INTERFACE)
        
        # On Windows, we might need to link against system OpenGL
        if(WIN32)
            target_link_libraries(angle_egl INTERFACE opengl32)
        endif()
    endif()
    
    # Set include directories for ANGLE headers
    target_include_directories(angle_egl INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/angle/include
    )
else()
    # Use native OpenGL ES
    find_package(OpenGLES3 REQUIRED)
    add_library(angle_egl ALIAS OpenGLES3::OpenGLES3)
endif()
