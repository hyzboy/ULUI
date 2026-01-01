#pragma once

#include <GLES3/gl3.h>
#include <memory>
#include "object.h"
#include "gl/Texture2D.h"
#include "gl/FBO.h"

#ifdef __ANDROID__
#include <EGL/egl.h>
#endif

namespace ului {
namespace gl {

/**
 * RenderTarget class for unified rendering interface
 * Supports rendering to Texture2D, default screen framebuffer, and EGLSurface (Android)
 * Provides unified access to render target properties like size and orientation
 */
class RenderTarget : public Object {
public:
    enum class Type {
        Screen,      // Render to default framebuffer (screen)
        Texture,     // Render to texture via FBO
        EGLSurface   // Render to EGLSurface (Android MediaCodec)
    };

    /**
     * Create a screen render target (default framebuffer)
     * @param width Screen width
     * @param height Screen height
     */
    RenderTarget(GLsizei width, GLsizei height);

    /**
     * Create a texture render target
     * @param texture Shared pointer to texture to render to
     * @param createDepthBuffer Whether to create a depth buffer
     */
    RenderTarget(std::shared_ptr<Texture2D> texture, bool createDepthBuffer = true);

#ifdef __ANDROID__
    /**
     * Create an EGLSurface render target (Android MediaCodec)
     * @param eglDisplay EGL display
     * @param eglSurface EGL surface (from AMediaCodec_createInputSurface)
     * @param width Surface width
     * @param height Surface height
     */
    RenderTarget(EGLDisplay eglDisplay, EGLSurface eglSurface, GLsizei width, GLsizei height);
#endif

    ~RenderTarget() override;

    // Disable copy
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    /**
     * Initialize the render target
     * @return true if successful, false otherwise
     */
    bool Initialize();

    /**
     * Bind this render target for rendering
     */
    void Bind() const;

    /**
     * Unbind this render target
     */
    void Unbind() const;

    /**
     * Clear the render target
     * @param clearColor Clear color buffer
     * @param clearDepth Clear depth buffer
     * @param clearStencil Clear stencil buffer
     */
    void Clear(bool clearColor = true, bool clearDepth = true, bool clearStencil = false) const;

    /**
     * Get the render target type
     */
    Type GetType() const { return m_type; }

    /**
     * Check if this is a screen render target
     */
    bool IsScreen() const { return m_type == Type::Screen; }

    /**
     * Check if this is a texture render target
     */
    bool IsTexture() const { return m_type == Type::Texture; }

    /**
     * Check if this is an EGLSurface render target (Android)
     */
    bool IsEGLSurface() const { return m_type == Type::EGLSurface; }

    /**
     * Get render target width
     */
    GLsizei GetWidth() const { return m_width; }

    /**
     * Get render target height
     */
    GLsizei GetHeight() const { return m_height; }

    /**
     * Get aspect ratio (width / height)
     */
    float GetAspectRatio() const { 
        return m_height > 0 ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.0f; 
    }

    /**
     * Get the texture (only for texture render targets)
     * @return Shared pointer to texture, or nullptr for screen targets
     */
    std::shared_ptr<Texture2D> GetTexture() const { return m_texture; }

    /**
     * Get the FBO (only for texture render targets)
     * @return Shared pointer to FBO, or nullptr for screen targets
     */
    std::shared_ptr<FBO> GetFBO() const { return m_fbo; }

    /**
     * Check if the render target is valid
     */
    bool IsValid() const;

    /**
     * Update render target size (for screen targets)
     * @param width New width
     * @param height New height
     */
    void Resize(GLsizei width, GLsizei height);

    /**
     * Destroy the render target and free resources
     */
    void Destroy();

#ifdef __ANDROID__
    /**
     * Swap buffers for EGLSurface render target
     * Must be called after rendering to EGLSurface to present the frame
     * @return true if successful, false otherwise
     */
    bool SwapBuffers();
    
    /**
     * Get the EGLSurface (only for EGLSurface render targets)
     */
    EGLSurface GetEGLSurface() const { return m_eglSurface; }
#endif

private:
    Type m_type;
    GLsizei m_width;
    GLsizei m_height;
    
    // For texture render targets
    std::shared_ptr<Texture2D> m_texture;
    std::shared_ptr<FBO> m_fbo;
    GLuint m_depthRenderbuffer;
    bool m_createDepthBuffer;

#ifdef __ANDROID__
    // For EGLSurface render targets (Android MediaCodec)
    EGLDisplay m_eglDisplay;
    EGLSurface m_eglSurface;
    EGLContext m_eglContext;
    EGLContext m_previousContext;
    EGLSurface m_previousReadSurface;
    EGLSurface m_previousDrawSurface;
#endif

    bool InitializeTextureTarget();
};

} // namespace gl
} // namespace ului
