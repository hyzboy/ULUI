#pragma once

#include <GLES3/gl3.h>
#include "object.h"

namespace ului {
namespace gl {

/**
 * UBO (Uniform Buffer Object) class for managing uniform data blocks
 * This is the preferred way to pass uniform data to shaders in ULUI
 */
class UBO : public Object {
public:
    UBO();
    ~UBO() override;

    // Disable copy
    UBO(const UBO&) = delete;
    UBO& operator=(const UBO&) = delete;

    /**
     * Create and initialize the UBO
     * @return true if successful, false otherwise
     */
    bool Create();

    /**
     * Bind this UBO to a binding point
     * @param bindingPoint Binding point index
     */
    void BindToPoint(GLuint bindingPoint) const;

    /**
     * Bind this UBO as a uniform buffer (for direct manipulation)
     */
    void Bind() const;

    /**
     * Unbind the current UBO
     */
    void Unbind() const;

    /**
     * Allocate storage for the UBO
     * @param size Size in bytes
     * @param data Initial data (can be nullptr)
     * @param usage Usage hint (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc.)
     */
    void SetData(GLsizeiptr size, const void* data = nullptr, 
                 GLenum usage = GL_DYNAMIC_DRAW);

    /**
     * Update part of the UBO data
     * @param offset Offset in bytes
     * @param size Size of data in bytes
     * @param data Pointer to data
     */
    void SetSubData(GLintptr offset, GLsizeiptr size, const void* data);

    /**
     * Map the UBO buffer for writing
     * @return Pointer to mapped memory or nullptr on failure
     */
    void* MapBuffer(GLenum access = GL_WRITE_ONLY);

    /**
     * Unmap the UBO buffer
     */
    void UnmapBuffer();

    /**
     * Get the OpenGL buffer handle
     */
    GLuint GetHandle() const { return m_ubo; }

    /**
     * Check if the UBO is valid
     */
    bool IsValid() const { return m_ubo != 0; }

    /**
     * Destroy the UBO and free resources
     */
    void Destroy();

private:
    GLuint m_ubo;
};

} // namespace gl
} // namespace ului
