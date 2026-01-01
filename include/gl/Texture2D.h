#pragma once

#include <GLES3/gl3.h>
#include "object.h"

namespace ului {
namespace gl {

/**
 * Texture2D class for managing 2D textures
 */
class Texture2D : public Object {
public:
    Texture2D();
    ~Texture2D() override;

    // Disable copy
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    /**
     * Create and initialize the texture
     * @return true if successful, false otherwise
     */
    bool Create();

    /**
     * Bind this texture to the specified texture unit
     * @param unit Texture unit (0-7 typically)
     */
    void Bind(GLuint unit = 0) const;

    /**
     * Unbind the current texture
     */
    void Unbind() const;

    /**
     * Set texture image data
     * @param width Texture width
     * @param height Texture height
     * @param internalFormat Internal format (GL_RGBA8, GL_RGB8, etc.)
     * @param format Pixel format (GL_RGBA, GL_RGB, etc.)
     * @param type Data type (GL_UNSIGNED_BYTE, etc.)
     * @param data Pointer to pixel data (can be nullptr)
     */
    void SetImage(GLsizei width, GLsizei height, GLint internalFormat,
                  GLenum format, GLenum type, const void* data = nullptr);

    /**
     * Update part of the texture
     * @param xoffset X offset
     * @param yoffset Y offset
     * @param width Width of subregion
     * @param height Height of subregion
     * @param format Pixel format
     * @param type Data type
     * @param data Pointer to pixel data
     */
    void SetSubImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                     GLenum format, GLenum type, const void* data);

    /**
     * Set texture filtering mode
     * @param minFilter Minification filter (GL_NEAREST, GL_LINEAR, etc.)
     * @param magFilter Magnification filter (GL_NEAREST, GL_LINEAR)
     */
    void SetFilter(GLint minFilter, GLint magFilter);

    /**
     * Set texture wrap mode
     * @param wrapS Wrap mode for S coordinate (GL_REPEAT, GL_CLAMP_TO_EDGE, etc.)
     * @param wrapT Wrap mode for T coordinate
     */
    void SetWrap(GLint wrapS, GLint wrapT);

    /**
     * Generate mipmaps for this texture
     */
    void GenerateMipmap();

    /**
     * Get the OpenGL texture handle
     */
    GLuint GetHandle() const { return m_texture; }

    /**
     * Check if the texture is valid
     */
    bool IsValid() const { return m_texture != 0; }

    /**
     * Get texture width
     */
    GLsizei GetWidth() const { return m_width; }

    /**
     * Get texture height
     */
    GLsizei GetHeight() const { return m_height; }

    /**
     * Destroy the texture and free resources
     */
    void Destroy();

private:
    GLuint m_texture;
    GLsizei m_width;
    GLsizei m_height;
    GLint m_internalFormat;
};

} // namespace gl
} // namespace ului
