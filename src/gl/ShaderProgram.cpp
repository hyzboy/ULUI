#include "gl/ShaderProgram.h"
#include <vector>

// Define GL_INVALID_INDEX if not defined
#ifndef GL_INVALID_INDEX
#define GL_INVALID_INDEX 0xFFFFFFFFu
#endif

namespace ului {
namespace gl {

ShaderProgram::ShaderProgram()
    : Object("ShaderProgram")
    , m_program(0)
    , m_vertexShader(0)
    , m_fragmentShader(0)
{
    LogD("ShaderProgram constructed");
}

ShaderProgram::~ShaderProgram()
{
    Destroy();
}

bool ShaderProgram::CompileShader(GLuint shader, const std::string& source)
{
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
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

bool ShaderProgram::LinkProgram()
{
    glLinkProgram(m_program);
    
    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(m_program, logLength, nullptr, log.data());
        LogE("Program linking failed: %s", log.data());
        return false;
    }
    
    LogD("Program linked successfully");
    return true;
}

bool ShaderProgram::CreateFromSource(const std::string& vertexSource, 
                                     const std::string& fragmentSource)
{
    // Clean up any existing resources
    Destroy();
    
    // Create vertex shader
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!CompileShader(m_vertexShader, vertexSource)) {
        Destroy();
        return false;
    }
    
    // Create fragment shader
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!CompileShader(m_fragmentShader, fragmentSource)) {
        Destroy();
        return false;
    }
    
    // Create program and link
    m_program = glCreateProgram();
    glAttachShader(m_program, m_vertexShader);
    glAttachShader(m_program, m_fragmentShader);
    
    if (!LinkProgram()) {
        Destroy();
        return false;
    }
    
    LogI("Shader program created successfully");
    return true;
}

void ShaderProgram::Use() const
{
    if (m_program) {
        glUseProgram(m_program);
    }
}

void ShaderProgram::Unuse() const
{
    glUseProgram(0);
}

GLint ShaderProgram::GetAttribLocation(const char* name) const
{
    if (!m_program) {
        LogW("Trying to get attribute location from invalid program");
        return -1;
    }
    return glGetAttribLocation(m_program, name);
}

GLuint ShaderProgram::GetUniformBlockIndex(const char* name) const
{
    if (!m_program) {
        LogW("Trying to get uniform block index from invalid program");
        return GL_INVALID_INDEX;
    }
    return glGetUniformBlockIndex(m_program, name);
}

void ShaderProgram::BindUniformBlock(GLuint blockIndex, GLuint bindingPoint) const
{
    if (!m_program) {
        LogW("Trying to bind uniform block with invalid program");
        return;
    }
    if (blockIndex == GL_INVALID_INDEX) {
        LogW("Trying to bind invalid uniform block");
        return;
    }
    glUniformBlockBinding(m_program, blockIndex, bindingPoint);
}

void ShaderProgram::Destroy()
{
    if (m_fragmentShader) {
        glDeleteShader(m_fragmentShader);
        m_fragmentShader = 0;
    }
    
    if (m_vertexShader) {
        glDeleteShader(m_vertexShader);
        m_vertexShader = 0;
    }
    
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

} // namespace gl
} // namespace ului
