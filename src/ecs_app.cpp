#include "ecs_app.h"
#include "logger.h"
#include <GLES3/gl3.h>
#include <vector>
#include <cmath>

using namespace ului;
using namespace ului::ecs;

// Vertex shader for rounded rectangles
static const char* vertexShaderSource = R"(
#version 300 es
precision mediump float;

in vec2 aPosition;
out vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aPosition * 0.5 + 0.5;
}
)";

// Fragment shader for rounded rectangles with SDF
static const char* fragmentShaderSource = R"(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
out vec4 fragColor;

uniform vec2 uPosition;      // Position in screen space (0-1)
uniform vec2 uSize;          // Size in screen space (0-1)
uniform float uCornerRadius; // Corner radius in screen space
uniform vec4 uColor;         // RGBA color
uniform vec2 uResolution;    // Screen resolution

// SDF for rounded rectangle
float sdRoundedBox(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    // Convert to pixel coordinates
    vec2 pixelCoord = vTexCoord * uResolution;
    vec2 rectCenter = uPosition * uResolution + uSize * uResolution * 0.5;
    vec2 halfSize = uSize * uResolution * 0.5;
    
    // Calculate distance to rounded rectangle
    vec2 p = pixelCoord - rectCenter;
    float d = sdRoundedBox(p, halfSize, uCornerRadius * uResolution.x);
    
    // Anti-aliasing
    float alpha = 1.0 - smoothstep(-1.0, 1.0, d);
    
    fragColor = vec4(uColor.rgb, uColor.a * alpha);
}
)";

ECSApp::ECSApp()
    : Object("ECSRenderer")
    , m_width(0)
    , m_height(0)
    , m_shaderProgram(0)
    , m_vertexShader(0)
    , m_fragmentShader(0)
    , m_vbo(0)
    , m_vao(0)
    , m_positionUniform(-1)
    , m_sizeUniform(-1)
    , m_cornerRadiusUniform(-1)
    , m_colorUniform(-1)
    , m_resolutionUniform(-1)
{
    LogD("ECSApp constructed");
}

ECSApp::~ECSApp()
{
    cleanup();
}

bool ECSApp::compileShader(GLuint shader, const char* source)
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
        LogE("Shader compilation failed: %s", log.data());
        return false;
    }
    
    LogD("Shader compiled successfully");
    return true;
}

bool ECSApp::linkProgram(GLuint program)
{
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        LogE("Program linking failed: %s", log.data());
        return false;
    }
    
    LogD("Program linked successfully");
    return true;
}

void ECSApp::createShaders()
{
    // Create and compile vertex shader
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(m_vertexShader, vertexShaderSource)) {
        return;
    }
    
    // Create and compile fragment shader
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(m_fragmentShader, fragmentShaderSource)) {
        return;
    }
    
    // Create program and link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    
    if (!linkProgram(m_shaderProgram)) {
        return;
    }
    
    // Get uniform locations
    m_positionUniform = glGetUniformLocation(m_shaderProgram, "uPosition");
    m_sizeUniform = glGetUniformLocation(m_shaderProgram, "uSize");
    m_cornerRadiusUniform = glGetUniformLocation(m_shaderProgram, "uCornerRadius");
    m_colorUniform = glGetUniformLocation(m_shaderProgram, "uColor");
    m_resolutionUniform = glGetUniformLocation(m_shaderProgram, "uResolution");
}

void ECSApp::createRoundedRectEntities()
{
    // Create multiple rounded rectangles with different colors and positions
    
    // Large blue rectangle in the center
    Entity rect1 = m_scene->CreateEntity();
    m_scene->AddComponent(rect1, std::make_unique<Transform2D>(&m_scene->GetTransformStorage2D(), 400.0f, 300.0f));
    m_scene->AddComponent(rect1, std::make_unique<RoundedRect2D>(200.0f, 150.0f, 20.0f, 0.2f, 0.4f, 0.8f, 1.0f));
    
    // Red rectangle top-left
    Entity rect2 = m_scene->CreateEntity();
    m_scene->AddComponent(rect2, std::make_unique<Transform2D>(&m_scene->GetTransformStorage2D(), 150.0f, 100.0f));
    m_scene->AddComponent(rect2, std::make_unique<RoundedRect2D>(120.0f, 80.0f, 15.0f, 0.9f, 0.2f, 0.2f, 1.0f));
    
    // Green rectangle top-right
    Entity rect3 = m_scene->CreateEntity();
    m_scene->AddComponent(rect3, std::make_unique<Transform2D>(&m_scene->GetTransformStorage2D(), 650.0f, 100.0f));
    m_scene->AddComponent(rect3, std::make_unique<RoundedRect2D>(120.0f, 80.0f, 15.0f, 0.2f, 0.9f, 0.3f, 1.0f));
    
    // Yellow rectangle bottom-left
    Entity rect4 = m_scene->CreateEntity();
    m_scene->AddComponent(rect4, std::make_unique<Transform2D>(&m_scene->GetTransformStorage2D(), 150.0f, 500.0f));
    m_scene->AddComponent(rect4, std::make_unique<RoundedRect2D>(100.0f, 60.0f, 10.0f, 0.9f, 0.9f, 0.2f, 1.0f));
    
    // Purple rectangle bottom-right
    Entity rect5 = m_scene->CreateEntity();
    m_scene->AddComponent(rect5, std::make_unique<Transform2D>(&m_scene->GetTransformStorage2D(), 650.0f, 500.0f));
    m_scene->AddComponent(rect5, std::make_unique<RoundedRect2D>(100.0f, 60.0f, 10.0f, 0.7f, 0.2f, 0.9f, 1.0f));
    
    // Small orange rectangle near center
    Entity rect6 = m_scene->CreateEntity();
    m_scene->AddComponent(rect6, std::make_unique<Transform2D>(&m_scene->GetTransformStorage2D(), 300.0f, 250.0f));
    m_scene->AddComponent(rect6, std::make_unique<RoundedRect2D>(80.0f, 50.0f, 12.0f, 1.0f, 0.6f, 0.2f, 1.0f));
    
    LogI("Created %zu rounded rectangle entities", m_scene->GetAllEntities().size());
}

bool ECSApp::initialize(int width, int height)
{
    m_width = width;
    m_height = height;
    
    // Print OpenGL ES information
    LogI("OpenGL Vendor: %s", glGetString(GL_VENDOR));
    LogI("OpenGL Renderer: %s", glGetString(GL_RENDERER));
    LogI("OpenGL Version: %s", glGetString(GL_VERSION));
    LogI("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // Create shaders
    createShaders();
    
    // Create fullscreen quad vertices
    float vertices[] = {
        // Position
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };
    
    // Create VAO and VBO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    GLint positionAttrib = glGetAttribLocation(m_shaderProgram, "aPosition");
    glEnableVertexAttribArray(positionAttrib);
    glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    
    // Set viewport
    glViewport(0, 0, m_width, m_height);
    
    // Set clear color to dark gray
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    
    // Create ECS scene
    m_scene = std::make_unique<Scene>();
    
    // Create rounded rectangle entities
    createRoundedRectEntities();
    
    LogI("ECS app initialized successfully with size %dx%d", m_width, m_height);
    return true;
}

void ECSApp::render()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use shader program
    glUseProgram(m_shaderProgram);
    
    // Set resolution uniform
    glUniform2f(m_resolutionUniform, static_cast<float>(m_width), static_cast<float>(m_height));
    
    // Bind VAO
    glBindVertexArray(m_vao);
    
    // Get all entities with Transform2D and RoundedRect2D components
    auto entities = m_scene->GetEntitiesWithComponent<Transform2D>();
    
    for (Entity entity : entities) {
        if (m_scene->HasComponent<RoundedRect2D>(entity)) {
            Transform2D* transform = m_scene->GetComponent<Transform2D>(entity);
            RoundedRect2D* rect = m_scene->GetComponent<RoundedRect2D>(entity);
            
            if (transform && rect) {
                // Convert pixel coordinates to normalized coordinates (0-1)
                float posX = transform->GetX() / m_width;
                float posY = 1.0f - (transform->GetY() / m_height); // Flip Y
                float sizeX = rect->width / m_width;
                float sizeY = rect->height / m_height;
                float cornerRadius = rect->cornerRadius / m_width;
                
                // Set uniforms
                glUniform2f(m_positionUniform, posX, posY);
                glUniform2f(m_sizeUniform, sizeX, sizeY);
                glUniform1f(m_cornerRadiusUniform, cornerRadius);
                glUniform4f(m_colorUniform, rect->colorR, rect->colorG, rect->colorB, rect->colorA);
                
                // Draw fullscreen quad (shader will clip to rounded rectangle)
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    }
    
    glBindVertexArray(0);
}

void ECSApp::cleanup()
{
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
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
    
    m_scene.reset();
}
