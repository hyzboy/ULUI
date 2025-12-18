#include "triangle_app.h"
#include "file_system.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace ului;

TriangleApp::TriangleApp()
    : m_width(0)
    , m_height(0)
    , m_shaderProgram(0)
    , m_vertexShader(0)
    , m_fragmentShader(0)
    , m_vbo(0)
    , m_positionAttrib(-1)
    , m_colorAttrib(-1)
{
}

TriangleApp::~TriangleApp()
{
    cleanup();
}

std::string TriangleApp::readShaderFile(const char* filename)
{
    // Use FileSystem to read shader from internal assets
    return FileSystem::ReadAssetText(filename);
}

bool TriangleApp::compileShader(GLuint shader, const char* source)
{
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        LOG_E("TriangleApp", "Shader compilation failed: %s", log.data());
        return false;
    }
    
    LOG_D("TriangleApp", "Shader compiled successfully");
    return true;
}

bool TriangleApp::linkProgram(GLuint program)
{
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        LOG_E("TriangleApp", "Program linking failed: %s", log.data());
        return false;
    }
    
    LOG_D("TriangleApp", "Program linked successfully");
    return true;
}

bool TriangleApp::initialize(int width, int height)
{
    m_width = width;
    m_height = height;
    
    // Print OpenGL ES information
    LOG_I("TriangleApp", "OpenGL Vendor: %s", glGetString(GL_VENDOR));
    LOG_I("TriangleApp", "OpenGL Renderer: %s", glGetString(GL_RENDERER));
    LOG_I("TriangleApp", "OpenGL Version: %s", glGetString(GL_VERSION));
    LOG_I("TriangleApp", "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // Load shaders
    LOG_D("TriangleApp", "Loading shader files");
    std::string vertexShaderSource = readShaderFile("shaders/triangle.vert");
    std::string fragmentShaderSource = readShaderFile("shaders/triangle.frag");
    
    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        LOG_E("TriangleApp", "Failed to load shader files");
        return false;
    }
    LOG_D("TriangleApp", "Shaders loaded successfully");
    
    // Create and compile vertex shader
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(m_vertexShader, vertexShaderSource.c_str())) {
        return false;
    }
    
    // Create and compile fragment shader
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(m_fragmentShader, fragmentShaderSource.c_str())) {
        return false;
    }
    
    // Create program and link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    
    if (!linkProgram(m_shaderProgram)) {
        return false;
    }
    
    // Get attribute locations
    m_positionAttrib = glGetAttribLocation(m_shaderProgram, "aPosition");
    m_colorAttrib = glGetAttribLocation(m_shaderProgram, "aColor");
    
    // Triangle vertices (position + color)
    float vertices[] = {
        // Position (x, y, z)    Color (r, g, b)
         0.0f,  0.5f, 0.0f,      1.0f, 0.0f, 0.0f,  // Top (red)
        -0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f,  // Bottom-left (green)
         0.5f, -0.5f, 0.0f,      0.0f, 0.0f, 1.0f   // Bottom-right (blue)
    };
    
    // Create and bind VBO
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set viewport
    glViewport(0, 0, m_width, m_height);
    
    // Set clear color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    std::cout << "Triangle app initialized successfully" << std::endl;
    return true;
}

void TriangleApp::render()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use shader program
    glUseProgram(m_shaderProgram);
    
    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Set vertex attribute pointers
    glEnableVertexAttribArray(m_positionAttrib);
    glVertexAttribPointer(m_positionAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(m_colorAttrib);
    glVertexAttribPointer(m_colorAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // Draw triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // Disable vertex attribute arrays
    glDisableVertexAttribArray(m_positionAttrib);
    glDisableVertexAttribArray(m_colorAttrib);
}

void TriangleApp::cleanup()
{
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    
    if (m_vertexShader) {
        glDeleteShader(m_vertexShader);
        m_vertexShader = 0;
    }
    
    if (m_fragmentShader) {
        glDeleteShader(m_fragmentShader);
        m_fragmentShader = 0;
    }
}
