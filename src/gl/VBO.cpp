#include "gl/VBO.h"

namespace ului {
namespace gl {

VBO::VBO()
    : Object("VBO")
    , m_vbo(0)
{
    LogD("VBO constructed");
}

VBO::~VBO()
{
    Destroy();
}

bool VBO::Create()
{
    if (m_vbo) {
        LogW("VBO already created");
        return true;
    }
    
    glGenBuffers(1, &m_vbo);
    if (m_vbo == 0) {
        LogE("Failed to create VBO");
        return false;
    }
    
    LogD("VBO created successfully: %u", m_vbo);
    return true;
}

void VBO::Bind() const
{
    if (!m_vbo) {
        LogW("Trying to bind invalid VBO");
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
}

void VBO::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::SetData(const void* data, GLsizeiptr size, GLenum usage)
{
    if (!m_vbo) {
        LogE("VBO not created");
        return;
    }
    
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    LogD("VBO data set: %ld bytes", (long)size);
}

void VBO::SetSubData(GLintptr offset, const void* data, GLsizeiptr size)
{
    if (!m_vbo) {
        LogE("VBO not created");
        return;
    }
    
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    LogD("VBO subdata updated: %ld bytes at offset %ld", (long)size, (long)offset);
}

void VBO::Destroy()
{
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        LogD("VBO destroyed: %u", m_vbo);
        m_vbo = 0;
    }
}

} // namespace gl
} // namespace ului
