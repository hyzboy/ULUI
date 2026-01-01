#pragma once

#include <GLES3/gl3.h>
#include "object.h"

namespace ului {
namespace gl {

/**
 * FBO (Framebuffer Object) class for off-screen rendering
 */
class FBO : public Object {
public:
    FBO();
    ~FBO() override;

    // Disable copy
    FBO(const FBO&) = delete;
    FBO& operator=(const FBO&) = delete;

    /**
     * Create and initialize the framebuffer
     * @return true if successful, false otherwise
     */
    bool Create();

    /**
     * Bind this framebuffer for rendering
     */
    void Bind() const;

    /**
     * Unbind the framebuffer (bind to default framebuffer 0)
     */
    void Unbind() const;

    /**
     * Attach a texture as a color attachment
     * @param texture Texture handle
     * @param attachment Attachment point (GL_COLOR_ATTACHMENT0, etc.)
     */
    void AttachTexture2D(GLuint texture, GLenum attachment = GL_COLOR_ATTACHMENT0);

    /**
     * Attach a renderbuffer as a depth attachment
     * @param renderbuffer Renderbuffer handle
     */
    void AttachDepthRenderbuffer(GLuint renderbuffer);

    /**
     * Attach a renderbuffer as a stencil attachment
     * @param renderbuffer Renderbuffer handle
     */
    void AttachStencilRenderbuffer(GLuint renderbuffer);

    /**
     * Attach a renderbuffer as a depth-stencil attachment
     * @param renderbuffer Renderbuffer handle
     */
    void AttachDepthStencilRenderbuffer(GLuint renderbuffer);

    /**
     * Check if the framebuffer is complete
     * @return true if complete, false otherwise
     */
    bool IsComplete() const;

    /**
     * Get the framebuffer status
     * @return OpenGL framebuffer status
     */
    GLenum GetStatus() const;

    /**
     * Get the OpenGL framebuffer handle
     */
    GLuint GetHandle() const { return m_fbo; }

    /**
     * Check if the FBO is valid
     */
    bool IsValid() const { return m_fbo != 0; }

    /**
     * Destroy the framebuffer and free resources
     */
    void Destroy();

private:
    GLuint m_fbo;
};

} // namespace gl
} // namespace ului
