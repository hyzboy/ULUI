# OpenGL Base Classes

ULUI provides a complete set of OpenGL ES 3.0 base classes for managing shader programs, vertex buffers, vertex arrays, and uniform buffers.

## Important Note

**This project does NOT support individual uniform access. All uniform data MUST use UBO (Uniform Buffer Objects) technology.**

## Class Overview

### ShaderProgram

Manages OpenGL shader program creation, compilation, and linking.

**Features:**
- Automatic vertex and fragment shader compilation
- Error log reporting
- Attribute location queries
- UBO support (via `GetUniformBlockIndex` and `BindUniformBlock`)
- **No individual uniform access**

**Usage Example:**

```cpp
#include "gl/ShaderProgram.h"
using namespace ului::gl;

ShaderProgram shader;

std::string vertexShader = R"(
    #version 300 es
    precision highp float;
    
    in vec3 aPosition;
    
    layout(std140) uniform Matrices {
        mat4 projection;
        mat4 view;
        mat4 model;
    };
    
    void main() {
        gl_Position = projection * view * model * vec4(aPosition, 1.0);
    }
)";

std::string fragmentShader = R"(
    #version 300 es
    precision highp float;
    
    out vec4 fragColor;
    
    layout(std140) uniform Material {
        vec4 color;
    };
    
    void main() {
        fragColor = color;
    }
)";

if (shader.CreateFromSource(vertexShader, fragmentShader)) {
    shader.Use();
    // Rendering code
    shader.Unuse();
}
```

### VBO (Vertex Buffer Object)

Manages OpenGL vertex buffer objects for storing vertex data.

**Usage Example:**

```cpp
#include "gl/VBO.h"
using namespace ului::gl;

VBO vbo;
vbo.Create();

float vertices[] = {
    0.0f,  0.5f, 0.0f,
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
};

vbo.SetData(vertices, sizeof(vertices), GL_STATIC_DRAW);
vbo.Bind();
```

### VAO (Vertex Array Object)

Manages vertex attribute configuration, simplifying vertex attribute setup.

**Usage Example:**

```cpp
#include "gl/VAO.h"
using namespace ului::gl;

VAO vao;
vao.Create();
vao.Bind();

// Configure vertex attributes
vao.EnableAttrib(0);
vao.SetAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

vao.Unbind();
```

### UBO (Uniform Buffer Object)

**This is the preferred way to pass uniform data to shaders in ULUI.**

**Usage Example:**

```cpp
#include "gl/UBO.h"
using namespace ului::gl;

// Define uniform structure (must match shader layout)
struct MaterialUniforms {
    float color[4];
    float roughness;
    float metallic;
    float padding[2];  // Align to 16 bytes
};

UBO ubo;
ubo.Create();

MaterialUniforms material;
material.color[0] = 1.0f;
material.color[1] = 1.0f;
material.color[2] = 1.0f;
material.color[3] = 1.0f;
material.roughness = 0.5f;
material.metallic = 0.0f;

ubo.SetData(sizeof(MaterialUniforms), &material, GL_DYNAMIC_DRAW);

// Bind to shader
GLuint blockIndex = shader.GetUniformBlockIndex("Material");
shader.BindUniformBlock(blockIndex, 0);
ubo.BindToPoint(0);
```

### SSBO (Shader Storage Buffer Object)

For large data storage with larger size limits than UBO and shader-writable capability.

**Usage Example:**

```cpp
#include "gl/SSBO.h"
using namespace ului::gl;

struct ParticleData {
    float position[3];
    float velocity[3];
    float life;
    float padding;
};

SSBO ssbo;
ssbo.Create();

std::vector<ParticleData> particles(1000);
// Initialize particle data...

ssbo.SetData(particles.size() * sizeof(ParticleData), 
            particles.data(), GL_DYNAMIC_DRAW);
ssbo.BindToPoint(0);
```

## UBO Memory Layout Rules

When using UBOs, you must follow the `std140` layout rules:

1. **Scalar and Vector Alignment:**
   - `float`, `int`: 4-byte aligned
   - `vec2`: 8-byte aligned
   - `vec3`, `vec4`: 16-byte aligned

2. **Arrays:**
   - Array elements are aligned to 16 bytes

3. **Structs:**
   - Structs align to the largest member alignment

**Example:**

```cpp
// C++ Structure
struct MyUniforms {
    float value1;        // offset 0, size 4
    float padding1[3];   // padding to 16 bytes
    float values[4];     // offset 16, each element 16 bytes
    float value2;        // offset 80
    float padding2[3];   // padding
};

// GLSL Shader
layout(std140) uniform MyBlock {
    float value1;        // offset 0
    float values[4];     // offset 16
    float value2;        // offset 80
};
```

## Complete Example

See `examples/gl_classes_example.cpp` for a complete usage example.

## Design Philosophy

1. **Object-Oriented:** All classes inherit from `ului::Object` for automatic logging
2. **Resource Management:** Destructors automatically clean up OpenGL resources
3. **Error Handling:** Failed operations return false and log errors
4. **UBO-First:** No individual uniform access, enforcing UBO usage
5. **Type Safety:** Use C++ type system to avoid OpenGL handle errors

## Why Use UBO?

1. **Performance:** UBOs are more efficient than individual uniforms
2. **Organization:** Group related data together
3. **Sharing:** Multiple shaders can share the same UBO
4. **Size:** UBOs can store more data (minimum 16KB)
5. **Standardization:** Consistent memory layout across platforms

## Important Notes

- OpenGL ES 3.0 does not support geometry shaders
- SSBO may not be available in some OpenGL ES 3.0 implementations (requires ES 3.1+)
- Use `std140` layout for cross-platform compatibility
- Always check `Create()` return values
- Bind VAO and VBO before use
- UBO size limit is at least 16KB (query `GL_MAX_UNIFORM_BLOCK_SIZE`)

## References

- [OpenGL ES 3.0 Specification](https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf)
- [GLSL ES 3.00 Specification](https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf)
- [UBO Layout Rules](https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Memory_layout)
