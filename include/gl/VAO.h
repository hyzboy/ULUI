#pragma once

#include <GLES3/gl3.h>
#include "object.h"

namespace ului {
namespace gl {

/**
 * VAO (Vertex Array Object) class for managing vertex attribute configuration
 */
class VAO : public Object {
public:
    VAO();
    ~VAO() override;

    // Disable copy
    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;

    /**
     * Create and initialize the VAO
     * @return true if successful, false otherwise
     */
    bool Create();

    /**
     * Bind this VAO
     */
    void Bind() const;

    /**
     * Unbind the current VAO
     */
    void Unbind() const;

    /**
     * Enable a vertex attribute array
     * @param index Attribute index
     */
    void EnableAttrib(GLuint index) const;

    /**
     * Disable a vertex attribute array
     * @param index Attribute index
     */
    void DisableAttrib(GLuint index) const;

    /**
     * Set vertex attribute pointer
     * @param index Attribute index
     * @param size Number of components per attribute (1, 2, 3, or 4)
     * @param type Data type (GL_FLOAT, GL_INT, etc.)
     * @param normalized Should be normalized
     * @param stride Byte offset between consecutive attributes
     * @param pointer Offset of first component
     */
    void SetAttribPointer(GLuint index, GLint size, GLenum type, 
                         GLboolean normalized, GLsizei stride, 
                         const void* pointer) const;

    /**
     * Set integer vertex attribute pointer (for integer types)
     * @param index Attribute index
     * @param size Number of components per attribute (1, 2, 3, or 4)
     * @param type Data type (GL_BYTE, GL_INT, etc.)
     * @param stride Byte offset between consecutive attributes
     * @param pointer Offset of first component
     */
    void SetAttribIPointer(GLuint index, GLint size, GLenum type,
                          GLsizei stride, const void* pointer) const;

    /**
     * Get the OpenGL VAO handle
     */
    GLuint GetHandle() const { return m_vao; }

    /**
     * Check if the VAO is valid
     */
    bool IsValid() const { return m_vao != 0; }

    /**
     * Destroy the VAO and free resources
     */
    void Destroy();

private:
    GLuint m_vao;
};

} // namespace gl
} // namespace ului
