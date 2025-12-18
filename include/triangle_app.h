#ifndef TRIANGLE_APP_H
#define TRIANGLE_APP_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <string>
#include <memory>
#include "object.h"

class TriangleApp : public ului::Object {
public:
    TriangleApp();
    ~TriangleApp() override;

    bool initialize(int width, int height);
    void render();
    void cleanup();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    bool compileShader(GLuint shader, const char* source);
    bool linkProgram(GLuint program);
    std::string readShaderFile(const char* filename);

    int m_width;
    int m_height;
    
    GLuint m_shaderProgram;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_vbo;
    
    GLint m_positionAttrib;
    GLint m_colorAttrib;
};

#endif // TRIANGLE_APP_H
