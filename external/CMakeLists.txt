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
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
    
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
        message(STATUS "ANGLE libraries not found, using system OpenGL/GLES")
        # Create interface library for headers
        add_library(angle_egl INTERFACE)
        
        # Try to link against available OpenGL implementations
        if(WIN32)
            target_link_libraries(angle_egl INTERFACE opengl32)
        else()
            # On Linux/macOS, try to find OpenGL ES first, then fall back to OpenGL
            find_library(GLES2_LIBRARY NAMES GLESv2)
            find_library(EGL_SYS_LIBRARY NAMES EGL)
            
            if(GLES2_LIBRARY)
                message(STATUS "Using system GLESv2: ${GLES2_LIBRARY}")
                target_link_libraries(angle_egl INTERFACE ${GLES2_LIBRARY})
                if(EGL_SYS_LIBRARY)
                    target_link_libraries(angle_egl INTERFACE ${EGL_SYS_LIBRARY})
                endif()
            else()
                # Fall back to desktop OpenGL
                find_package(OpenGL REQUIRED)
                message(STATUS "Using system OpenGL: ${OPENGL_LIBRARIES}")
                target_link_libraries(angle_egl INTERFACE ${OPENGL_LIBRARIES})
            endif()
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
