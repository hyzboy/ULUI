// OpenGL Base Classes Example
// This example demonstrates the usage of ShaderProgram, VBO, VAO, UBO, and SSBO classes

#include "gl/ShaderProgram.h"
#include "gl/VBO.h"
#include "gl/VAO.h"
#include "gl/UBO.h"
#include "gl/SSBO.h"
#include "logger.h"
#include <iostream>

using namespace ului;
using namespace ului::gl;

// Example uniform buffer structure
struct MaterialUniforms {
    float color[4];      // RGBA color
    float roughness;
    float metallic;
    float padding[2];    // Align to 16 bytes
};

// Example SSBO data structure
struct ParticleData {
    float position[3];
    float velocity[3];
    float life;
    float padding;       // Align to 16 bytes
};

void exampleUsage() {
    Logger::Log(Logger::LogLevel::Info, "GLExample", "OpenGL Base Classes Example");
    
    // ====== ShaderProgram Example ======
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Creating shader program...");
    
    ShaderProgram shader;
    
    std::string vertexShader = R"(
        #version 300 es
        precision highp float;
        
        in vec3 aPosition;
        in vec3 aColor;
        
        out vec3 vColor;
        
        // Uniform block for transformation matrices
        layout(std140) uniform Matrices {
            mat4 projection;
            mat4 view;
            mat4 model;
        };
        
        void main() {
            vColor = aColor;
            gl_Position = projection * view * model * vec4(aPosition, 1.0);
        }
    )";
    
    std::string fragmentShader = R"(
        #version 300 es
        precision highp float;
        
        in vec3 vColor;
        out vec4 fragColor;
        
        // Uniform block for material properties
        layout(std140) uniform Material {
            vec4 color;
            float roughness;
            float metallic;
        };
        
        void main() {
            fragColor = vec4(vColor * color.rgb, 1.0);
        }
    )";
    
    if (!shader.CreateFromSource(vertexShader, fragmentShader)) {
        Logger::Log(Logger::LogLevel::Error, "GLExample", "Failed to create shader program");
        return;
    }
    
    // ====== VBO Example ======
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Creating VBO...");
    
    VBO vbo;
    if (!vbo.Create()) {
        Logger::Log(Logger::LogLevel::Error, "GLExample", "Failed to create VBO");
        return;
    }
    
    // Vertex data (position + color)
    float vertices[] = {
        // Position (x, y, z)    Color (r, g, b)
         0.0f,  0.5f, 0.0f,      1.0f, 0.0f, 0.0f,  // Top (red)
        -0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f,  // Bottom-left (green)
         0.5f, -0.5f, 0.0f,      0.0f, 0.0f, 1.0f   // Bottom-right (blue)
    };
    
    vbo.SetData(vertices, sizeof(vertices), GL_STATIC_DRAW);
    
    // ====== VAO Example ======
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Creating VAO...");
    
    VAO vao;
    if (!vao.Create()) {
        Logger::Log(Logger::LogLevel::Error, "GLExample", "Failed to create VAO");
        return;
    }
    
    vao.Bind();
    vbo.Bind();
    
    // Get attribute locations
    GLint posAttrib = shader.GetAttribLocation("aPosition");
    GLint colorAttrib = shader.GetAttribLocation("aColor");
    
    if (posAttrib >= 0) {
        vao.EnableAttrib(posAttrib);
        vao.SetAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    }
    
    if (colorAttrib >= 0) {
        vao.EnableAttrib(colorAttrib);
        vao.SetAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
                            (void*)(3 * sizeof(float)));
    }
    
    vao.Unbind();
    
    // ====== UBO Example ======
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Creating UBO...");
    
    UBO materialUBO;
    if (!materialUBO.Create()) {
        Logger::Log(Logger::LogLevel::Error, "GLExample", "Failed to create UBO");
        return;
    }
    
    // Set material data
    MaterialUniforms material;
    material.color[0] = 1.0f;
    material.color[1] = 1.0f;
    material.color[2] = 1.0f;
    material.color[3] = 1.0f;
    material.roughness = 0.5f;
    material.metallic = 0.0f;
    
    materialUBO.SetData(sizeof(MaterialUniforms), &material, GL_DYNAMIC_DRAW);
    
    // Bind UBO to binding point
    GLuint materialBlockIndex = shader.GetUniformBlockIndex("Material");
    if (materialBlockIndex != GL_INVALID_INDEX) {
        shader.BindUniformBlock(materialBlockIndex, 0);  // Binding point 0
        materialUBO.BindToPoint(0);
    }
    
    // ====== SSBO Example ======
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Creating SSBO...");
    
    SSBO particleSSBO;
    if (!particleSSBO.Create()) {
        Logger::Log(Logger::LogLevel::Error, "GLExample", "Failed to create SSBO");
        return;
    }
    
    // Allocate storage for 1000 particles
    const int numParticles = 1000;
    std::vector<ParticleData> particles(numParticles);
    
    // Initialize particle data
    for (int i = 0; i < numParticles; ++i) {
        particles[i].position[0] = 0.0f;
        particles[i].position[1] = 0.0f;
        particles[i].position[2] = 0.0f;
        particles[i].velocity[0] = 0.0f;
        particles[i].velocity[1] = 0.0f;
        particles[i].velocity[2] = 0.0f;
        particles[i].life = 1.0f;
    }
    
    particleSSBO.SetData(particles.size() * sizeof(ParticleData), 
                        particles.data(), GL_DYNAMIC_DRAW);
    
    // Bind SSBO to binding point
    particleSSBO.BindToPoint(0);
    
    // ====== Rendering Example ======
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Ready to render!");
    
    // In a real rendering loop:
    // 1. shader.Use();
    // 2. vao.Bind();
    // 3. glDrawArrays(GL_TRIANGLES, 0, 3);
    // 4. vao.Unbind();
    // 5. shader.Unuse();
    
    Logger::Log(Logger::LogLevel::Info, "GLExample", "Example completed successfully!");
}

int main() {
    // This example is for demonstration purposes only
    // It shows how to use the OpenGL base classes
    // In a real application, you would need an OpenGL context first
    
    std::cout << "OpenGL Base Classes Example\n";
    std::cout << "============================\n\n";
    std::cout << "This example demonstrates:\n";
    std::cout << "1. ShaderProgram - Creating and managing shaders (NO individual uniforms)\n";
    std::cout << "2. VBO - Vertex buffer object for vertex data\n";
    std::cout << "3. VAO - Vertex array object for vertex attribute configuration\n";
    std::cout << "4. UBO - Uniform buffer object for uniform data blocks\n";
    std::cout << "5. SSBO - Shader storage buffer object for large data\n\n";
    std::cout << "Note: All uniform data must use UBO - individual uniform access is not supported!\n\n";
    
    // Uncomment the following line to run the example with an OpenGL context
    // exampleUsage();
    
    return 0;
}
