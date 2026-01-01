#include "gl/UBO.h"

namespace ului {
namespace gl {

UBO::UBO()
    : Object("UBO")
    , m_ubo(0)
{
    LogD("UBO constructed");
}

UBO::~UBO()
{
    Destroy();
}

bool UBO::Create()
{
    if (m_ubo) {
        LogW("UBO already created");
        return true;
    }
    
    glGenBuffers(1, &m_ubo);
    if (m_ubo == 0) {
        LogE("Failed to create UBO");
        return false;
    }
    
    LogD("UBO created successfully: %u", m_ubo);
    return true;
}

void UBO::BindToPoint(GLuint bindingPoint) const
{
    if (!m_ubo) {
        LogW("Trying to bind invalid UBO to point");
        return;
    }
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_ubo);
    LogD("UBO bound to binding point %u", bindingPoint);
}

void UBO::Bind() const
{
    if (!m_ubo) {
        LogW("Trying to bind invalid UBO");
        return;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
}

void UBO::Unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO::SetData(GLsizeiptr size, const void* data, GLenum usage)
{
    if (!m_ubo) {
        LogE("UBO not created");
        return;
    }
    
    Bind();
    glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
    LogD("UBO data set: %ld bytes", (long)size);
}

void UBO::SetSubData(GLintptr offset, GLsizeiptr size, const void* data)
{
    if (!m_ubo) {
        LogE("UBO not created");
        return;
    }
    
    Bind();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    LogD("UBO subdata updated: %ld bytes at offset %ld", (long)size, (long)offset);
}

void* UBO::MapBufferRange(GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    if (!m_ubo) {
        LogE("UBO not created");
        return nullptr;
    }
    
    if (length <= 0) {
        LogE("Invalid map length: %ld", (long)length);
        return nullptr;
    }
    
    Bind();
    void* ptr = glMapBufferRange(GL_UNIFORM_BUFFER, offset, length, access);
    if (!ptr) {
        LogE("Failed to map UBO buffer");
    }
    return ptr;
}

void UBO::UnmapBuffer()
{
    if (!m_ubo) {
        LogE("UBO not created");
        return;
    }
    
    Bind();
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UBO::Destroy()
{
    if (m_ubo) {
        glDeleteBuffers(1, &m_ubo);
        LogD("UBO destroyed: %u", m_ubo);
        m_ubo = 0;
    }
}

} // namespace gl
} // namespace ului
