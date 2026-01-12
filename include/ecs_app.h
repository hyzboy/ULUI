#ifndef ECS_APP_H
#define ECS_APP_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include "object.h"
#include "ecs/ECS.h"
#include "ecs/components/RoundedRect2D.h"
#include <memory>

/**
 * ECSApp uses the Entity Component System to render multiple rounded rectangles.
 * This replaces the triangle rendering with an ECS-based architecture.
 */
class ECSApp : public ului::Object {
public:
    ECSApp();
    ~ECSApp() override;

    bool initialize(int width, int height);
    void render();
    void cleanup();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    bool compileShader(GLuint shader, const char* source);
    bool linkProgram(GLuint program);
    void createShaders();
    void createRoundedRectEntities();

    int m_width;
    int m_height;
    
    // OpenGL resources
    GLuint m_shaderProgram;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_vbo;
    GLuint m_vao;
    
    // Shader uniforms
    GLint m_positionUniform;
    GLint m_sizeUniform;
    GLint m_cornerRadiusUniform;
    GLint m_colorUniform;
    GLint m_resolutionUniform;
    
    // ECS Scene
    std::unique_ptr<ului::ecs::Scene> m_scene;
};

#endif // ECS_APP_H
