#include "gl/Texture2D.h"

namespace ului {
namespace gl {

Texture2D::Texture2D()
    : Object("Texture2D")
    , m_texture(0)
    , m_width(0)
    , m_height(0)
    , m_internalFormat(GL_RGBA8)
    , m_textureType(TextureType::TEXTURE_2D)
{
    LogD("Texture2D constructed");
}

Texture2D::Texture2D(TextureType type)
    : Object("Texture2D")
    , m_texture(0)
    , m_width(0)
    , m_height(0)
    , m_internalFormat(GL_RGBA8)
    , m_textureType(type)
{
    LogD("Texture2D constructed with type %d", static_cast<int>(type));
}

Texture2D::~Texture2D()
{
    Destroy();
}

bool Texture2D::Create(TextureType type)
{
    if (m_texture) {
        LogW("Texture2D already created");
        return true;
    }
    
    m_textureType = type;
    
    glGenTextures(1, &m_texture);
    if (m_texture == 0) {
        LogE("Failed to create Texture2D");
        return false;
    }
    
    if (IsExternalOES()) {
        LogD("External OES Texture2D created successfully: %u", m_texture);
        // External textures have specific requirements
        // Set default filter to LINEAR (NEAREST may not work on some devices)
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_texture);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    } else {
        LogD("Texture2D created successfully: %u", m_texture);
    }
    
    return true;
}

void Texture2D::Bind(GLuint unit) const
{
    if (!m_texture) {
        LogW("Trying to bind invalid Texture2D");
        return;
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GetTarget(), m_texture);
}

void Texture2D::Unbind() const
{
    glBindTexture(GetTarget(), 0);
}

void Texture2D::SetImage(GLsizei width, GLsizei height, GLint internalFormat,
                         GLenum format, GLenum type, const void* data)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    if (IsExternalOES()) {
        LogE("Cannot call SetImage on external OES texture - data comes from SurfaceTexture");
        return;
    }
    
    m_width = width;
    m_height = height;
    m_internalFormat = internalFormat;
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    LogD("Texture2D image set: %dx%d, format=%d", width, height, internalFormat);
}

void Texture2D::SetSubImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                            GLenum format, GLenum type, const void* data)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    if (IsExternalOES()) {
        LogE("Cannot call SetSubImage on external OES texture");
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, format, type, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    LogD("Texture2D subimage updated: %dx%d at (%d,%d)", width, height, xoffset, yoffset);
}

void Texture2D::SetFilter(GLint minFilter, GLint magFilter)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    GLenum target = GetTarget();
    glBindTexture(target, m_texture);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(target, 0);
    
    LogD("Texture2D filter set: min=%d, mag=%d", minFilter, magFilter);
}

void Texture2D::SetWrap(GLint wrapS, GLint wrapT)
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    GLenum target = GetTarget();
    glBindTexture(target, m_texture);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
    glBindTexture(target, 0);
    
    LogD("Texture2D wrap set: S=%d, T=%d", wrapS, wrapT);
}

void Texture2D::GenerateMipmap()
{
    if (!m_texture) {
        LogE("Texture2D not created");
        return;
    }
    
    if (IsExternalOES()) {
        LogE("Cannot generate mipmaps for external OES texture");
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    LogD("Texture2D mipmap generated");
}

void Texture2D::SetExternalTextureSize(GLsizei width, GLsizei height)
{
    if (!IsExternalOES()) {
        LogW("SetExternalTextureSize should only be called on external OES textures");
    }
    
    m_width = width;
    m_height = height;
    LogD("External texture size set: %dx%d", width, height);
}

GLenum Texture2D::GetTarget() const
{
    return IsExternalOES() ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D;
}

void Texture2D::Destroy()
{
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        LogD("Texture2D destroyed: %u (type=%d)", m_texture, static_cast<int>(m_textureType));
        m_texture = 0;
        m_width = 0;
        m_height = 0;
    }
}

} // namespace gl
} // namespace ului
