# OpenGL 基础类

ULUI 提供了一套完整的 OpenGL ES 3.0 基础类，用于管理着色器程序、顶点缓冲、顶点数组和统一缓冲。

## 重要说明

**本项目不支持单个 uniform 访问，全部使用 UBO (Uniform Buffer Objects) 技术来管理 uniform 数据块。**

## 类概述

### ShaderProgram (着色器程序)

管理 OpenGL 着色器程序的创建、编译和链接。

**特性：**
- 自动编译顶点和片段着色器
- 错误日志报告
- 获取属性位置
- UBO 支持（通过 `GetUniformBlockIndex` 和 `BindUniformBlock`）
- **不支持单个 uniform 访问**

**用法示例：**

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
    // 渲染代码
    shader.Unuse();
}
```

### VBO (顶点缓冲对象)

管理 OpenGL 顶点缓冲对象，用于存储顶点数据。

**用法示例：**

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

### VAO (顶点数组对象)

管理顶点属性配置，简化顶点属性设置。

**用法示例：**

```cpp
#include "gl/VAO.h"
using namespace ului::gl;

VAO vao;
vao.Create();
vao.Bind();

// 配置顶点属性
vao.EnableAttrib(0);
vao.SetAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

vao.Unbind();
```

### UBO (统一缓冲对象)

**这是 ULUI 中管理 uniform 数据的首选方式。**

**用法示例：**

```cpp
#include "gl/UBO.h"
using namespace ului::gl;

// 定义 uniform 结构（需要匹配着色器中的布局）
struct MaterialUniforms {
    float color[4];
    float roughness;
    float metallic;
    float padding[2];  // 对齐到 16 字节
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

// 绑定到着色器
GLuint blockIndex = shader.GetUniformBlockIndex("Material");
shader.BindUniformBlock(blockIndex, 0);
ubo.BindToPoint(0);
```

### SSBO (着色器存储缓冲对象)

用于大量数据存储，比 UBO 有更大的大小限制，并且可以在着色器中写入。

**用法示例：**

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
// 初始化粒子数据...

ssbo.SetData(particles.size() * sizeof(ParticleData), 
            particles.data(), GL_DYNAMIC_DRAW);
ssbo.BindToPoint(0);
```

## UBO 内存布局规则

使用 UBO 时，必须遵循 `std140` 布局规则：

1. **标量和向量对齐：**
   - `float`, `int`: 4 字节对齐
   - `vec2`: 8 字节对齐
   - `vec3`, `vec4`: 16 字节对齐

2. **数组：**
   - 数组元素对齐到 16 字节

3. **结构体：**
   - 结构体对齐到最大成员的对齐值

**示例：**

```cpp
// C++ 结构
struct MyUniforms {
    float value1;        // offset 0, size 4
    float padding1[3];   // 填充到 16 字节
    float values[4];     // offset 16, 每个元素 16 字节
    float value2;        // offset 80
    float padding2[3];   // 填充
};

// GLSL 着色器
layout(std140) uniform MyBlock {
    float value1;        // offset 0
    float values[4];     // offset 16
    float value2;        // offset 80
};
```

## 完整示例

请参阅 `examples/gl_classes_example.cpp` 获取完整的使用示例。

## 设计理念

1. **面向对象：** 所有类都继承自 `ului::Object`，提供自动日志记录
2. **资源管理：** 析构函数自动清理 OpenGL 资源
3. **错误处理：** 失败操作返回 false 并记录错误
4. **UBO 优先：** 不支持单个 uniform 访问，强制使用 UBO
5. **类型安全：** 使用 C++ 类型系统避免 OpenGL 句柄错误

## 为什么使用 UBO？

1. **性能：** UBO 比单个 uniform 更高效
2. **组织：** 将相关数据分组
3. **共享：** 多个着色器可以共享同一个 UBO
4. **大小：** UBO 可以存储更多数据（至少 16KB）
5. **标准化：** 跨平台一致的内存布局

## 注意事项

- OpenGL ES 3.0 不支持几何着色器
- SSBO 在某些 OpenGL ES 3.0 实现中可能不可用（需要 ES 3.1+）
- 使用 `std140` 布局确保跨平台兼容性
- 始终检查 `Create()` 返回值
- 使用前绑定 VAO 和 VBO
- UBO 大小限制至少为 16KB（查询 `GL_MAX_UNIFORM_BLOCK_SIZE`）

## 参考资料

- [OpenGL ES 3.0 规范](https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf)
- [GLSL ES 3.00 规范](https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf)
- [UBO 布局规则](https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Memory_layout)
