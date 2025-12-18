#include "triangle_app.h"
#include "file_system.h"
#include <iostream>
#include <cstdlib>

using namespace ului;

#ifndef PLATFORM_IOS
#ifndef PLATFORM_ANDROID

// Desktop platforms using GLFW
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #define GLFW_EXPOSE_NATIVE_NSGL
#else
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3native.h>

static void errorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    std::cout << "ULUI - OpenGL ES 3.0 Triangle Example with ANGLE" << std::endl;
    
    // Initialize FileSystem with default asset path
    FileSystem::Initialize("assets/");
    
    // Initialize GLFW
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Configure GLFW for OpenGL ES 3.0
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    
    // Create window
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
                                          "ULUI - Triangle Example", nullptr, nullptr);
    
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    // Set callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    
    // Make context current
    glfwMakeContextCurrent(window);
    
    // Enable vsync
    glfwSwapInterval(1);
    
    // Get framebuffer size (might differ from window size on high DPI displays)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Initialize triangle app
    TriangleApp app;
    if (!app.initialize(fbWidth, fbHeight)) {
        std::cerr << "Failed to initialize triangle app" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    std::cout << "Starting render loop..." << std::endl;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Render
        app.render();
        
        // Swap buffers
        glfwSwapBuffers(window);
        
        // Poll events
        glfwPollEvents();
    }
    
    // Cleanup
    app.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    FileSystem::Shutdown();
    
    std::cout << "Application terminated successfully" << std::endl;
    return EXIT_SUCCESS;
}

#else // PLATFORM_ANDROID

// Android platform implementation
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ULUI", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ULUI", __VA_ARGS__))

struct AndroidApp {
    android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    TriangleApp triangleApp;
    bool initialized;
};

static int initializeEGL(AndroidApp* androidApp)
{
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    
    androidApp->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(androidApp->display, nullptr, nullptr);
    
    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(androidApp->display, attribs, &config, 1, &numConfigs);
    
    EGLint format;
    eglGetConfigAttrib(androidApp->display, config, EGL_NATIVE_VISUAL_ID, &format);
    
    androidApp->surface = eglCreateWindowSurface(androidApp->display, config,
                                                 androidApp->app->window, nullptr);
    
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    
    androidApp->context = eglCreateContext(androidApp->display, config, nullptr, contextAttribs);
    
    if (eglMakeCurrent(androidApp->display, androidApp->surface, 
                       androidApp->surface, androidApp->context) == EGL_FALSE) {
        LOGE("Unable to eglMakeCurrent");
        return -1;
    }
    
    EGLint width, height;
    eglQuerySurface(androidApp->display, androidApp->surface, EGL_WIDTH, &width);
    eglQuerySurface(androidApp->display, androidApp->surface, EGL_HEIGHT, &height);
    
    androidApp->triangleApp.initialize(width, height);
    androidApp->initialized = true;
    
    return 0;
}

static void terminateEGL(AndroidApp* androidApp)
{
    if (androidApp->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(androidApp->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (androidApp->context != EGL_NO_CONTEXT) {
            eglDestroyContext(androidApp->display, androidApp->context);
        }
        if (androidApp->surface != EGL_NO_SURFACE) {
            eglDestroySurface(androidApp->display, androidApp->surface);
        }
        eglTerminate(androidApp->display);
    }
    androidApp->display = EGL_NO_DISPLAY;
    androidApp->context = EGL_NO_CONTEXT;
    androidApp->surface = EGL_NO_SURFACE;
    androidApp->initialized = false;
}

static void handleCmd(android_app* app, int32_t cmd)
{
    AndroidApp* androidApp = (AndroidApp*)app->userData;
    
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr) {
                initializeEGL(androidApp);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            terminateEGL(androidApp);
            break;
    }
}

void android_main(android_app* state)
{
    AndroidApp androidApp = {};
    androidApp.app = state;
    state->userData = &androidApp;
    state->onAppCmd = handleCmd;
    
    // Initialize FileSystem with Android asset manager
    FileSystem::Initialize(nullptr);
#ifdef __ANDROID__
    FileSystem::SetAndroidAssetManager(state->activity->assetManager);
#endif
    
    while (true) {
        int events;
        android_poll_source* source;
        
        while (ALooper_pollAll(androidApp.initialized ? 0 : -1, nullptr, &events, 
                               (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(state, source);
            }
            
            if (state->destroyRequested != 0) {
                terminateEGL(&androidApp);
                FileSystem::Shutdown();
                return;
            }
        }
        
        if (androidApp.initialized) {
            androidApp.triangleApp.render();
            eglSwapBuffers(androidApp.display, androidApp.surface);
        }
    }
}

#endif // PLATFORM_ANDROID

#endif // PLATFORM_IOS
