#pragma once

#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include "object.h"

namespace ului {
namespace gl {

/**
 * ShaderProgram class for managing OpenGL shader programs.
 * Does NOT support individual uniform access - use UBO (Uniform Buffer Objects) instead.
 */
class ShaderProgram : public Object {
public:
    ShaderProgram();
    ~ShaderProgram() override;

    // Disable copy
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    /**
     * Create shader program from vertex and fragment shader sources
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @return true if successful, false otherwise
     */
    bool CreateFromSource(const std::string& vertexSource, const std::string& fragmentSource);



    /**
     * Use this shader program for rendering
     */
    void Use() const;

    /**
     * Stop using this shader program
     */
    void Unuse() const;

    /**
     * Get the OpenGL program handle
     */
    GLuint GetHandle() const { return m_program; }

    /**
     * Check if the program is valid
     */
    bool IsValid() const { return m_program != 0; }

    /**
     * Get attribute location
     * @param name Attribute name
     * @return Attribute location or -1 if not found
     */
    GLint GetAttribLocation(const char* name) const;

    /**
     * Get uniform block index for UBO
     * @param name Uniform block name
     * @return Uniform block index
     */
    GLuint GetUniformBlockIndex(const char* name) const;

    /**
     * Bind uniform block to binding point
     * @param blockIndex Uniform block index
     * @param bindingPoint Binding point
     */
    void BindUniformBlock(GLuint blockIndex, GLuint bindingPoint) const;

    /**
     * Destroy the shader program and free resources
     */
    void Destroy();

private:
    GLuint m_program;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;

    bool CompileShader(GLuint shader, const std::string& source);
    bool LinkProgram();
};

} // namespace gl
} // namespace ului
