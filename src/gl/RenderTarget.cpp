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
{
    if (texture && texture->IsValid()) {
        m_width = texture->GetWidth();
        m_height = texture->GetHeight();
    }
    LogD("RenderTarget constructed (texture): %dx%d, depth=%d", m_width, m_height, createDepthBuffer);
}

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
    } else if (m_fbo) {
        m_fbo->Bind();
        glViewport(0, 0, m_width, m_height);
    } else {
        LogW("Cannot bind uninitialized texture render target");
    }
}

void RenderTarget::Unbind() const
{
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
    } else {
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

    // Don't destroy texture or FBO as they may be shared
    // Just clear our references
    m_fbo.reset();
    m_texture.reset();
}

} // namespace gl
} // namespace ului
