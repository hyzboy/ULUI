#include "gl/FBO.h"

namespace ului {
namespace gl {

FBO::FBO()
    : Object("FBO")
    , m_fbo(0)
{
    LogD("FBO constructed");
}

FBO::~FBO()
{
    Destroy();
}

bool FBO::Create()
{
    if (m_fbo) {
        LogW("FBO already created");
        return true;
    }
    
    glGenFramebuffers(1, &m_fbo);
    if (m_fbo == 0) {
        LogE("Failed to create FBO");
        return false;
    }
    
    LogD("FBO created successfully: %u", m_fbo);
    return true;
}

void FBO::Bind() const
{
    if (!m_fbo) {
        LogW("Trying to bind invalid FBO");
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FBO::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::AttachTexture2D(GLuint texture, GLenum attachment)
{
    if (!m_fbo) {
        LogE("FBO not created");
        return;
    }
    
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    LogD("Texture %u attached to FBO at attachment 0x%x", texture, attachment);
}

void FBO::AttachDepthRenderbuffer(GLuint renderbuffer)
{
    if (!m_fbo) {
        LogE("FBO not created");
        return;
    }
    
    Bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    LogD("Depth renderbuffer %u attached to FBO", renderbuffer);
}

void FBO::AttachStencilRenderbuffer(GLuint renderbuffer)
{
    if (!m_fbo) {
        LogE("FBO not created");
        return;
    }
    
    Bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    LogD("Stencil renderbuffer %u attached to FBO", renderbuffer);
}

void FBO::AttachDepthStencilRenderbuffer(GLuint renderbuffer)
{
    if (!m_fbo) {
        LogE("FBO not created");
        return;
    }
    
    Bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    LogD("Depth-stencil renderbuffer %u attached to FBO", renderbuffer);
}

bool FBO::IsComplete() const
{
    if (!m_fbo) {
        LogW("FBO not created");
        return false;
    }
    
    Bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

GLenum FBO::GetStatus() const
{
    if (!m_fbo) {
        LogW("FBO not created");
        return 0;
    }
    
    Bind();
    return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

void FBO::Destroy()
{
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        LogD("FBO destroyed: %u", m_fbo);
        m_fbo = 0;
    }
}

} // namespace gl
} // namespace ului
