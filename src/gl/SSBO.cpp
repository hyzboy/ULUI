#include "gl/SSBO.h"

namespace ului {
namespace gl {

SSBO::SSBO()
    : Object("SSBO")
    , m_ssbo(0)
{
    LogD("SSBO constructed");
}

SSBO::~SSBO()
{
    Destroy();
}

bool SSBO::Create()
{
    if (m_ssbo) {
        LogW("SSBO already created");
        return true;
    }
    
    glGenBuffers(1, &m_ssbo);
    if (m_ssbo == 0) {
        LogE("Failed to create SSBO");
        return false;
    }
    
    LogD("SSBO created successfully: %u", m_ssbo);
    return true;
}

void SSBO::BindToPoint(GLuint bindingPoint) const
{
    if (!m_ssbo) {
        LogW("Trying to bind invalid SSBO to point");
        return;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_ssbo);
    LogD("SSBO bound to binding point %u", bindingPoint);
}

void SSBO::Bind() const
{
    if (!m_ssbo) {
        LogW("Trying to bind invalid SSBO");
        return;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
}

void SSBO::Unbind() const
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::SetData(GLsizeiptr size, const void* data, GLenum usage)
{
    if (!m_ssbo) {
        LogE("SSBO not created");
        return;
    }
    
    Bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    LogD("SSBO data set: %ld bytes", (long)size);
}

void SSBO::SetSubData(GLintptr offset, GLsizeiptr size, const void* data)
{
    if (!m_ssbo) {
        LogE("SSBO not created");
        return;
    }
    
    Bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    LogD("SSBO subdata updated: %ld bytes at offset %ld", (long)size, (long)offset);
}

void* SSBO::MapBuffer(GLenum access)
{
    if (!m_ssbo) {
        LogE("SSBO not created");
        return nullptr;
    }
    
    Bind();
    void* ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 0, access);
    if (!ptr) {
        LogE("Failed to map SSBO buffer");
    }
    return ptr;
}

void SSBO::UnmapBuffer()
{
    if (!m_ssbo) {
        LogE("SSBO not created");
        return;
    }
    
    Bind();
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void SSBO::Destroy()
{
    if (m_ssbo) {
        glDeleteBuffers(1, &m_ssbo);
        LogD("SSBO destroyed: %u", m_ssbo);
        m_ssbo = 0;
    }
}

} // namespace gl
} // namespace ului
