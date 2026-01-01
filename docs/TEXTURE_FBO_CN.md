# Texture2D, FBO 和 RenderTarget 类

ULUI 提供了完整的纹理和帧缓冲管理类，用于离屏渲染和渲染目标管理。

## 类概述

### Texture2D (2D 纹理)

管理 OpenGL 2D 纹理对象。

**特性：**
- 创建和管理 2D 纹理
- 设置纹理数据和子区域更新
- 配置过滤和包裹模式
- 自动生成 mipmap
- 查询纹理尺寸

**用法示例：**

```cpp
#include "gl/Texture2D.h"
using namespace ului::gl;

// 创建纹理
Texture2D texture;
texture.Create();

// 设置纹理数据
texture.SetImage(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

// 设置过滤模式
texture.SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

// 设置包裹模式
texture.SetWrap(GL_REPEAT, GL_REPEAT);

// 生成 mipmap
texture.GenerateMipmap();

// 绑定到纹理单元
texture.Bind(0);  // 绑定到 GL_TEXTURE0
```

### FBO (帧缓冲对象)

管理 OpenGL 帧缓冲对象，用于离屏渲染。

**特性：**
- 创建和管理帧缓冲对象
- 附加纹理作为颜色附件
- 附加深度/模板渲染缓冲
- 检查帧缓冲完整性

**用法示例：**

```cpp
#include "gl/FBO.h"
using namespace ului::gl;

// 创建 FBO
FBO fbo;
fbo.Create();

// 附加纹理作为颜色附件
fbo.Bind();
fbo.AttachTexture2D(texture.GetHandle(), GL_COLOR_ATTACHMENT0);

// 创建并附加深度缓冲
GLuint depthRB;
glGenRenderbuffers(1, &depthRB);
glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 512, 512);
fbo.AttachDepthRenderbuffer(depthRB);

// 检查完整性
if (fbo.IsComplete()) {
    // 渲染到 FBO
    fbo.Bind();
    // ... 渲染代码 ...
    fbo.Unbind();
}
```

### RenderTarget (渲染目标)

统一的渲染目标接口，支持渲染到纹理和屏幕。

**特性：**
- 统一的渲染目标接口
- 支持渲染到纹理（通过 FBO）
- 支持渲染到屏幕（默认帧缓冲）
- 提供统一的尺寸、宽高比等属性
- 自动管理深度缓冲
- 清除操作

**用法示例：**

#### 1. 屏幕渲染目标

```cpp
#include "gl/RenderTarget.h"
using namespace ului::gl;

// 创建屏幕渲染目标
auto screenTarget = std::make_shared<RenderTarget>(800, 600);
screenTarget->Initialize();

// 使用
screenTarget->Bind();
screenTarget->Clear(true, true, false);
// ... 渲染代码 ...
screenTarget->Unbind();

// 窗口大小改变时
screenTarget->Resize(1024, 768);
```

#### 2. 纹理渲染目标

```cpp
#include "gl/RenderTarget.h"
#include "gl/Texture2D.h"
using namespace ului::gl;

// 创建纹理
auto texture = std::make_shared<Texture2D>();
texture->Create();
texture->SetImage(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
texture->SetFilter(GL_LINEAR, GL_LINEAR);

// 创建纹理渲染目标（带深度缓冲）
auto textureTarget = std::make_shared<RenderTarget>(texture, true);
if (textureTarget->Initialize()) {
    // 渲染到纹理
    textureTarget->Bind();
    textureTarget->Clear(true, true, false);
    // ... 渲染代码 ...
    textureTarget->Unbind();
    
    // 现在可以在后续渲染中使用纹理
    texture->Bind(0);
}
```

#### 3. 统一接口示例

```cpp
void RenderScene(std::shared_ptr<RenderTarget> target) {
    target->Bind();
    
    // 设置视口（自动完成）
    GLsizei width = target->GetWidth();
    GLsizei height = target->GetHeight();
    float aspect = target->GetAspectRatio();
    
    // 清除缓冲
    target->Clear(true, true, false);
    
    // 渲染场景
    // ...
    
    target->Unbind();
}

// 可以用于屏幕和纹理
RenderScene(screenTarget);
RenderScene(textureTarget);
```

## 完整示例：后处理管线

```cpp
#include "gl/Texture2D.h"
#include "gl/RenderTarget.h"
#include "gl/ShaderProgram.h"

// 1. 创建离屏渲染纹理
auto offscreenTexture = std::make_shared<Texture2D>();
offscreenTexture->Create();
offscreenTexture->SetImage(1024, 768, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
offscreenTexture->SetFilter(GL_LINEAR, GL_LINEAR);
offscreenTexture->SetWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

// 2. 创建离屏渲染目标
auto offscreenTarget = std::make_shared<RenderTarget>(offscreenTexture, true);
offscreenTarget->Initialize();

// 3. 创建屏幕渲染目标
auto screenTarget = std::make_shared<RenderTarget>(1024, 768);

// 4. 第一遍：渲染场景到纹理
offscreenTarget->Bind();
offscreenTarget->Clear(true, true, false);
// 渲染 3D 场景
RenderScene();
offscreenTarget->Unbind();

// 5. 第二遍：使用纹理渲染到屏幕（后处理）
screenTarget->Bind();
screenTarget->Clear(true, false, false);
postProcessShader.Use();
offscreenTexture->Bind(0);
// 渲染全屏四边形应用后处理效果
RenderFullscreenQuad();
screenTarget->Unbind();
```

## 设计理念

1. **统一接口：** RenderTarget 提供统一的接口，无论渲染到屏幕还是纹理
2. **资源管理：** 自动管理 FBO 和深度缓冲的生命周期
3. **灵活性：** 支持共享纹理和 FBO 对象
4. **类型安全：** 使用 shared_ptr 管理对象生命周期
5. **易用性：** 简化常见的离屏渲染场景

## 使用场景

### 1. 阴影贴图

```cpp
// 创建阴影贴图纹理
auto shadowMap = std::make_shared<Texture2D>();
shadowMap->Create();
shadowMap->SetImage(2048, 2048, GL_DEPTH_COMPONENT16, 
                    GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);

// 创建阴影贴图渲染目标
auto shadowTarget = std::make_shared<RenderTarget>(shadowMap, false);
shadowTarget->Initialize();

// 渲染阴影贴图
shadowTarget->Bind();
shadowTarget->Clear(false, true, false);
RenderSceneFromLightPerspective();
shadowTarget->Unbind();
```

### 2. 反射/折射

```cpp
// 创建反射纹理
auto reflectionTexture = std::make_shared<Texture2D>();
reflectionTexture->Create();
reflectionTexture->SetImage(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

auto reflectionTarget = std::make_shared<RenderTarget>(reflectionTexture, true);
reflectionTarget->Initialize();

// 渲染反射
reflectionTarget->Bind();
RenderSceneReflected();
reflectionTarget->Unbind();

// 在主渲染中使用反射纹理
reflectionTexture->Bind(0);
```

### 3. 多渲染目标（MRT）

虽然单个 RenderTarget 支持一个颜色附件，但可以扩展 FBO 类来支持多个附件：

```cpp
// 附加多个纹理
fbo.AttachTexture2D(texture1->GetHandle(), GL_COLOR_ATTACHMENT0);
fbo.AttachTexture2D(texture2->GetHandle(), GL_COLOR_ATTACHMENT0 + 1);
fbo.AttachTexture2D(texture3->GetHandle(), GL_COLOR_ATTACHMENT0 + 2);
```

## 注意事项

- **纹理尺寸：** 确保纹理尺寸为 2 的幂次方以获得最佳性能（如果使用 mipmap）
- **格式兼容性：** 检查设备对特定内部格式的支持
- **帧缓冲完整性：** 始终检查 `IsComplete()` 或 `GetStatus()`
- **性能：** 避免频繁切换渲染目标
- **内存：** 大纹理会消耗大量 GPU 内存
- **生命周期：** 使用 shared_ptr 确保纹理在 RenderTarget 使用期间保持有效

## 参考资料

- [OpenGL ES 3.0 帧缓冲对象](https://www.khronos.org/opengl/wiki/Framebuffer_Object)
- [OpenGL ES 3.0 纹理](https://www.khronos.org/opengl/wiki/Texture)
