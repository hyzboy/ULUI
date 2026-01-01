#pragma once

#include <GLES3/gl3.h>
#include "object.h"

namespace ului {
namespace gl {

/**
 * VBO (Vertex Buffer Object) class for managing vertex data
 */
class VBO : public Object {
public:
    VBO();
    ~VBO() override;

    // Disable copy
    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;

    /**
     * Create and initialize the VBO
     * @return true if successful, false otherwise
     */
    bool Create();

    /**
     * Bind this VBO
     */
    void Bind() const;

    /**
     * Unbind the current VBO
     */
    void Unbind() const;

    /**
     * Upload data to the VBO
     * @param data Pointer to vertex data
     * @param size Size of data in bytes
     * @param usage Usage hint (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc.)
     */
    void SetData(const void* data, GLsizeiptr size, GLenum usage = GL_STATIC_DRAW);

    /**
     * Update part of the VBO data
     * @param offset Offset in bytes
     * @param data Pointer to data
     * @param size Size of data in bytes
     */
    void SetSubData(GLintptr offset, const void* data, GLsizeiptr size);

    /**
     * Get the OpenGL buffer handle
     */
    GLuint GetHandle() const { return m_vbo; }

    /**
     * Check if the VBO is valid
     */
    bool IsValid() const { return m_vbo != 0; }

    /**
     * Destroy the VBO and free resources
     */
    void Destroy();

private:
    GLuint m_vbo;
};

} // namespace gl
} // namespace ului
