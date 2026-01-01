#include "gl/Texture2D.h"

namespace ului {
namespace gl {

Texture2D::Texture2D()
    : Object("Texture2D")
    , m_texture(0)
    , m_width(0)
    , m_height(0)
    , m_internalFormat(GL_RGBA8)
{
    LogD("Texture2D constructed");
}

Texture2D::~Texture2D()
{
    Destroy();
}

bool Texture2D::Create()
{
    if (m_texture) {
        LogW("Texture2D already created");
        return true;
    }
    
    glGenTextures(1, &m_texture);
    if (m_texture == 0) {
        LogE("Failed to create Texture2D");
        return false;
    }
    
    LogD("Texture2D created successfully: %u", m_texture);
    return true;
}

void Texture2D::Bind(GLuint unit) const
{
    if (!m_texture) {
        LogW("Trying to bind invalid Texture2D");
        return;
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Texture2D::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::SetImage(GLsizei width, GLsizei height, GLint internalFormat,
                         GLenum format, GLenum type, const void* data)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    m_width = width;
    m_height = height;
    m_internalFormat = internalFormat;
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    
    LogD("Texture2D image set: %dx%d, format=%d", width, height, internalFormat);
}

void Texture2D::SetSubImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                            GLenum format, GLenum type, const void* data)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, format, type, data);
    
    LogD("Texture2D subimage updated: %dx%d at (%d,%d)", width, height, xoffset, yoffset);
}

void Texture2D::SetFilter(GLint minFilter, GLint magFilter)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    LogD("Texture2D filter set: min=%d, mag=%d", minFilter, magFilter);
}

void Texture2D::SetWrap(GLint wrapS, GLint wrapT)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    
    LogD("Texture2D wrap set: S=%d, T=%d", wrapS, wrapT);
}

void Texture2D::GenerateMipmap()
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    LogD("Texture2D mipmap generated");
}

void Texture2D::Destroy()
{
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        LogD("Texture2D destroyed: %u", m_texture);
        m_texture = 0;
        m_width = 0;
        m_height = 0;
    }
}

} // namespace gl
} // namespace ului
