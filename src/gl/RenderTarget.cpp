#include "gl/RenderTarget.h"

namespace ului {
namespace gl {

RenderTarget::RenderTarget(GLsizei width, GLsizei height)
    : Object("RenderTarget")
    , m_type(Type::Screen)
    , m_width(width)
    , m_height(height)
    , m_depthRenderbuffer(0)
    , m_createDepthBuffer(false)
#ifdef __ANDROID__
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_eglContext(EGL_NO_CONTEXT)
    , m_previousContext(EGL_NO_CONTEXT)
    , m_previousReadSurface(EGL_NO_SURFACE)
    , m_previousDrawSurface(EGL_NO_SURFACE)
#endif
{
    LogD("RenderTarget constructed (screen): %dx%d", width, height);
}

RenderTarget::RenderTarget(std::shared_ptr<Texture2D> texture, bool createDepthBuffer)
    : Object("RenderTarget")
    , m_type(Type::Texture)
    , m_width(0)
    , m_height(0)
    , m_texture(texture)
    , m_depthRenderbuffer(0)
    , m_createDepthBuffer(createDepthBuffer)
#ifdef __ANDROID__
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_eglContext(EGL_NO_CONTEXT)
    , m_previousContext(EGL_NO_CONTEXT)
    , m_previousReadSurface(EGL_NO_SURFACE)
    , m_previousDrawSurface(EGL_NO_SURFACE)
#endif
{
    if (texture && texture->IsValid()) {
        m_width = texture->GetWidth();
        m_height = texture->GetHeight();
    }
    LogD("RenderTarget constructed (texture): %dx%d, depth=%d", m_width, m_height, createDepthBuffer);
}

#ifdef __ANDROID__
RenderTarget::RenderTarget(EGLDisplay eglDisplay, EGLSurface eglSurface, GLsizei width, GLsizei height)
    : Object("RenderTarget")
    , m_type(Type::EGLSurface)
    , m_width(width)
    , m_height(height)
    , m_depthRenderbuffer(0)
    , m_createDepthBuffer(false)
    , m_eglDisplay(eglDisplay)
    , m_eglSurface(eglSurface)
    , m_eglContext(EGL_NO_CONTEXT)
    , m_previousContext(EGL_NO_CONTEXT)
    , m_previousReadSurface(EGL_NO_SURFACE)
    , m_previousDrawSurface(EGL_NO_SURFACE)
{
    LogD("RenderTarget constructed (EGLSurface): %dx%d", width, height);
}
#endif

RenderTarget::~RenderTarget()
{
    Destroy();
}

bool RenderTarget::Initialize()
{
    if (m_type == Type::Screen) {
        // Screen render target doesn't need initialization
        LogI("Screen render target initialized: %dx%d", m_width, m_height);
        return true;
    }
    
#ifdef __ANDROID__
    if (m_type == Type::EGLSurface) {
        // EGLSurface render target initialization
        if (m_eglDisplay == EGL_NO_DISPLAY || m_eglSurface == EGL_NO_SURFACE) {
            LogE("Invalid EGL display or surface");
            return false;
        }
        
        // Get current EGL context to create a shared context
        EGLContext currentContext = eglGetCurrentContext();
        if (currentContext == EGL_NO_CONTEXT) {
            LogE("No current EGL context available");
            return false;
        }
        
        // Query context attributes
        EGLint configId;
        if (!eglQueryContext(m_eglDisplay, currentContext, EGL_CONFIG_ID, &configId)) {
            LogE("Failed to query EGL context config");
            return false;
        }
        
        // Get the EGL config
        EGLConfig config;
        EGLint numConfigs;
        EGLint configAttribs[] = {
            EGL_CONFIG_ID, configId,
            EGL_NONE
        };
        if (!eglChooseConfig(m_eglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
            LogE("Failed to get EGL config");
            return false;
        }
        
        // Create context attributes for OpenGL ES 3.0
        EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
        };
        
        // Create a shared context for this surface
        m_eglContext = eglCreateContext(m_eglDisplay, config, currentContext, contextAttribs);
        if (m_eglContext == EGL_NO_CONTEXT) {
            EGLint error = eglGetError();
            LogE("Failed to create EGL context: 0x%x", error);
            return false;
        }
        
        LogI("EGLSurface render target initialized: %dx%d", m_width, m_height);
        return true;
    }
#endif
    
    return InitializeTextureTarget();
}

bool RenderTarget::InitializeTextureTarget()
{
    if (!m_texture || !m_texture->IsValid()) {
        LogE("Invalid texture for texture render target");
        return false;
    }

    // Update size from texture
    m_width = m_texture->GetWidth();
    m_height = m_texture->GetHeight();

    if (m_width <= 0 || m_height <= 0) {
        LogE("Invalid texture dimensions: %dx%d", m_width, m_height);
        return false;
    }

    // Create FBO
    m_fbo = std::make_shared<FBO>();
    if (!m_fbo->Create()) {
        LogE("Failed to create FBO for texture render target");
        return false;
    }

    // Attach texture to FBO
    m_fbo->Bind();
    m_fbo->AttachTexture2D(m_texture->GetHandle(), GL_COLOR_ATTACHMENT0);

    // Create depth buffer if requested
    if (m_createDepthBuffer) {
        glGenRenderbuffers(1, &m_depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
        m_fbo->AttachDepthRenderbuffer(m_depthRenderbuffer);
        LogD("Depth renderbuffer created: %u", m_depthRenderbuffer);
    }

    // Check framebuffer completeness
    if (!m_fbo->IsComplete()) {
        GLenum status = m_fbo->GetStatus();
        LogE("Framebuffer incomplete, status: 0x%x", status);
        Destroy();
        return false;
    }

    m_fbo->Unbind();
    LogI("Texture render target initialized: %dx%d", m_width, m_height);
    return true;
}

void RenderTarget::Bind() const
{
    if (m_type == Type::Screen) {
        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
    }
#ifdef __ANDROID__
    else if (m_type == Type::EGLSurface) {
        // Save current EGL state
        EGLDisplay currentDisplay = eglGetCurrentDisplay();
        m_previousContext = eglGetCurrentContext();
        m_previousReadSurface = eglGetCurrentSurface(EGL_READ);
        m_previousDrawSurface = eglGetCurrentSurface(EGL_DRAW);
        
        // Make the EGLSurface context current
        if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
            EGLint error = eglGetError();
            LogE("Failed to make EGL surface current: 0x%x", error);
            return;
        }
        
        glViewport(0, 0, m_width, m_height);
    }
#endif
    else if (m_fbo) {
        m_fbo->Bind();
        glViewport(0, 0, m_width, m_height);
    } else {
        LogW("Cannot bind uninitialized texture render target");
    }
}

void RenderTarget::Unbind() const
{
#ifdef __ANDROID__
    if (m_type == Type::EGLSurface) {
        // Restore previous EGL context
        if (m_previousContext != EGL_NO_CONTEXT) {
            if (!eglMakeCurrent(m_eglDisplay, m_previousDrawSurface, m_previousReadSurface, m_previousContext)) {
                EGLint error = eglGetError();
                LogE("Failed to restore previous EGL context: 0x%x", error);
            }
        }
        return;
    }
#endif
    
    if (m_type == Type::Texture && m_fbo) {
        m_fbo->Unbind();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void RenderTarget::Clear(bool clearColor, bool clearDepth, bool clearStencil) const
{
    GLbitfield mask = 0;
    if (clearColor) mask |= GL_COLOR_BUFFER_BIT;
    if (clearDepth) mask |= GL_DEPTH_BUFFER_BIT;
    if (clearStencil) mask |= GL_STENCIL_BUFFER_BIT;
    
    if (mask) {
        glClear(mask);
    }
}

bool RenderTarget::IsValid() const
{
    if (m_type == Type::Screen) {
        return m_width > 0 && m_height > 0;
    }
#ifdef __ANDROID__
    else if (m_type == Type::EGLSurface) {
        return m_eglDisplay != EGL_NO_DISPLAY && 
               m_eglSurface != EGL_NO_SURFACE && 
               m_eglContext != EGL_NO_CONTEXT &&
               m_width > 0 && m_height > 0;
    }
#endif
    else {
        return m_fbo && m_fbo->IsValid() && m_texture && m_texture->IsValid();
    }
}

void RenderTarget::Resize(GLsizei width, GLsizei height)
{
    if (m_type == Type::Texture) {
        LogW("Cannot resize texture render target directly. Resize the texture instead.");
        return;
    }

    m_width = width;
    m_height = height;
    LogD("Screen render target resized to %dx%d", width, height);
}

void RenderTarget::Destroy()
{
    if (m_depthRenderbuffer) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        LogD("Depth renderbuffer destroyed: %u", m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }

#ifdef __ANDROID__
    // Destroy EGL context for EGLSurface targets
    if (m_eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(m_eglDisplay, m_eglContext);
        LogD("EGL context destroyed");
        m_eglContext = EGL_NO_CONTEXT;
    }
    // Note: We don't destroy m_eglSurface as it's owned by MediaCodec
#endif

    // Don't destroy texture or FBO as they may be shared
    // Just clear our references
    m_fbo.reset();
    m_texture.reset();
}

#ifdef __ANDROID__
bool RenderTarget::SwapBuffers()
{
    if (m_type != Type::EGLSurface) {
        LogW("SwapBuffers only valid for EGLSurface render targets");
        return false;
    }
    
    if (m_eglDisplay == EGL_NO_DISPLAY || m_eglSurface == EGL_NO_SURFACE) {
        LogE("Invalid EGL display or surface");
        return false;
    }
    
    if (!eglSwapBuffers(m_eglDisplay, m_eglSurface)) {
        EGLint error = eglGetError();
        LogE("Failed to swap buffers: 0x%x", error);
        return false;
    }
    
    return true;
}
#endif

} // namespace gl
} // namespace ului
