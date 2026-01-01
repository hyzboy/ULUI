#include "gl/VAO.h"

namespace ului {
namespace gl {

VAO::VAO()
    : Object("VAO")
    , m_vao(0)
{
    LogD("VAO constructed");
}

VAO::~VAO()
{
    Destroy();
}

bool VAO::Create()
{
    if (m_vao) {
        LogW("VAO already created");
        return true;
    }
    
    glGenVertexArrays(1, &m_vao);
    if (m_vao == 0) {
        LogE("Failed to create VAO");
        return false;
    }
    
    LogD("VAO created successfully: %u", m_vao);
    return true;
}

void VAO::Bind() const
{
    if (!m_vao) {
        LogW("Trying to bind invalid VAO");
        return;
    }
    glBindVertexArray(m_vao);
}

void VAO::Unbind() const
{
    glBindVertexArray(0);
}

void VAO::EnableAttrib(GLuint index) const
{
    if (!m_vao) {
        LogW("VAO not bound");
        return;
    }
    glEnableVertexAttribArray(index);
}

void VAO::DisableAttrib(GLuint index) const
{
    if (!m_vao) {
        LogW("VAO not bound");
        return;
    }
    glDisableVertexAttribArray(index);
}

void VAO::SetAttribPointer(GLuint index, GLint size, GLenum type,
                          GLboolean normalized, GLsizei stride,
                          const void* pointer) const
{
    if (!m_vao) {
        LogW("VAO not bound");
        return;
    }
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void VAO::SetAttribIPointer(GLuint index, GLint size, GLenum type,
                           GLsizei stride, const void* pointer) const
{
    if (!m_vao) {
        LogW("VAO not bound");
        return;
    }
    glVertexAttribIPointer(index, size, type, stride, pointer);
}

void VAO::Destroy()
{
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        LogD("VAO destroyed: %u", m_vao);
        m_vao = 0;
    }
}

} // namespace gl
} // namespace ului
