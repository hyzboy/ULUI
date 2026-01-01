#pragma once

#include <GLES3/gl3.h>
#include "object.h"

namespace ului {
namespace gl {

/**
 * SSBO (Shader Storage Buffer Object) class for managing large data buffers
 * SSBOs provide shader-writable storage with larger size limits than UBOs
 */
class SSBO : public Object {
public:
    SSBO();
    ~SSBO() override;

    // Disable copy
    SSBO(const SSBO&) = delete;
    SSBO& operator=(const SSBO&) = delete;

    /**
     * Create and initialize the SSBO
     * @return true if successful, false otherwise
     */
    bool Create();

    /**
     * Bind this SSBO to a binding point
     * @param bindingPoint Binding point index
     */
    void BindToPoint(GLuint bindingPoint) const;

    /**
     * Bind this SSBO as a shader storage buffer (for direct manipulation)
     */
    void Bind() const;

    /**
     * Unbind the current SSBO
     */
    void Unbind() const;

    /**
     * Allocate storage for the SSBO
     * @param size Size in bytes
     * @param data Initial data (can be nullptr)
     * @param usage Usage hint (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc.)
     */
    void SetData(GLsizeiptr size, const void* data = nullptr, 
                 GLenum usage = GL_DYNAMIC_DRAW);

    /**
     * Update part of the SSBO data
     * @param offset Offset in bytes
     * @param size Size of data in bytes
     * @param data Pointer to data
     */
    void SetSubData(GLintptr offset, GLsizeiptr size, const void* data);

    /**
     * Map the SSBO buffer for reading/writing
     * @param offset Offset in bytes to start mapping
     * @param length Length in bytes to map
     * @param access Access mode flags (GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, etc.)
     * @return Pointer to mapped memory or nullptr on failure
     */
    void* MapBufferRange(GLintptr offset, GLsizeiptr length, GLbitfield access);

    /**
     * Unmap the SSBO buffer
     */
    void UnmapBuffer();

    /**
     * Get the OpenGL buffer handle
     */
    GLuint GetHandle() const { return m_ssbo; }

    /**
     * Check if the SSBO is valid
     */
    bool IsValid() const { return m_ssbo != 0; }

    /**
     * Destroy the SSBO and free resources
     */
    void Destroy();

private:
    GLuint m_ssbo;
};

} // namespace gl
} // namespace ului
