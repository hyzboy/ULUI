# Texture2D, FBO and RenderTarget Classes

ULUI provides comprehensive texture and framebuffer management classes for offscreen rendering and render target management.

## Class Overview

### Texture2D

Manages OpenGL 2D texture objects.

**Features:**
- Create and manage 2D textures
- Set texture data and sub-region updates
- Configure filtering and wrapping modes
- Automatic mipmap generation
- Query texture dimensions

**Usage Example:**

```cpp
#include "gl/Texture2D.h"
using namespace ului::gl;

// Create texture
Texture2D texture;
texture.Create();

// Set texture data
texture.SetImage(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

// Set filter mode
texture.SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

// Set wrap mode
texture.SetWrap(GL_REPEAT, GL_REPEAT);

// Generate mipmaps
texture.GenerateMipmap();

// Bind to texture unit
texture.Bind(0);  // Bind to GL_TEXTURE0
```

### FBO (Framebuffer Object)

Manages OpenGL framebuffer objects for offscreen rendering.

**Features:**
- Create and manage framebuffer objects
- Attach textures as color attachments
- Attach depth/stencil renderbuffers
- Check framebuffer completeness

**Usage Example:**

```cpp
#include "gl/FBO.h"
using namespace ului::gl;

// Create FBO
FBO fbo;
fbo.Create();

// Attach texture as color attachment
fbo.Bind();
fbo.AttachTexture2D(texture.GetHandle(), GL_COLOR_ATTACHMENT0);

// Create and attach depth buffer
GLuint depthRB;
glGenRenderbuffers(1, &depthRB);
glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 512, 512);
fbo.AttachDepthRenderbuffer(depthRB);

// Check completeness
if (fbo.IsComplete()) {
    // Render to FBO
    fbo.Bind();
    // ... rendering code ...
    fbo.Unbind();
}
```

### RenderTarget

Unified rendering target interface supporting both texture and screen rendering.

**Features:**
- Unified render target interface
- Support rendering to texture (via FBO)
- Support rendering to screen (default framebuffer)
- Provide unified properties like size and aspect ratio
- Automatic depth buffer management
- Clear operations

**Usage Examples:**

#### 1. Screen Render Target

```cpp
#include "gl/RenderTarget.h"
using namespace ului::gl;

// Create screen render target
auto screenTarget = std::make_shared<RenderTarget>(800, 600);
screenTarget->Initialize();

// Usage
screenTarget->Bind();
screenTarget->Clear(true, true, false);
// ... rendering code ...
screenTarget->Unbind();

// On window resize
screenTarget->Resize(1024, 768);
```

#### 2. Texture Render Target

```cpp
#include "gl/RenderTarget.h"
#include "gl/Texture2D.h"
using namespace ului::gl;

// Create texture
auto texture = std::make_shared<Texture2D>();
texture->Create();
texture->SetImage(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
texture->SetFilter(GL_LINEAR, GL_LINEAR);

// Create texture render target (with depth buffer)
auto textureTarget = std::make_shared<RenderTarget>(texture, true);
if (textureTarget->Initialize()) {
    // Render to texture
    textureTarget->Bind();
    textureTarget->Clear(true, true, false);
    // ... rendering code ...
    textureTarget->Unbind();
    
    // Now use the texture in subsequent rendering
    texture->Bind(0);
}
```

#### 3. Unified Interface Example

```cpp
void RenderScene(std::shared_ptr<RenderTarget> target) {
    target->Bind();
    
    // Set viewport (automatic)
    GLsizei width = target->GetWidth();
    GLsizei height = target->GetHeight();
    float aspect = target->GetAspectRatio();
    
    // Clear buffers
    target->Clear(true, true, false);
    
    // Render scene
    // ...
    
    target->Unbind();
}

// Works for both screen and texture
RenderScene(screenTarget);
RenderScene(textureTarget);
```

## Complete Example: Post-Processing Pipeline

```cpp
#include "gl/Texture2D.h"
#include "gl/RenderTarget.h"
#include "gl/ShaderProgram.h"

// 1. Create offscreen render texture
auto offscreenTexture = std::make_shared<Texture2D>();
offscreenTexture->Create();
offscreenTexture->SetImage(1024, 768, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
offscreenTexture->SetFilter(GL_LINEAR, GL_LINEAR);
offscreenTexture->SetWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

// 2. Create offscreen render target
auto offscreenTarget = std::make_shared<RenderTarget>(offscreenTexture, true);
offscreenTarget->Initialize();

// 3. Create screen render target
auto screenTarget = std::make_shared<RenderTarget>(1024, 768);

// 4. First pass: Render scene to texture
offscreenTarget->Bind();
offscreenTarget->Clear(true, true, false);
// Render 3D scene
RenderScene();
offscreenTarget->Unbind();

// 5. Second pass: Render texture to screen (post-process)
screenTarget->Bind();
screenTarget->Clear(true, false, false);
postProcessShader.Use();
offscreenTexture->Bind(0);
// Render fullscreen quad with post-process effect
RenderFullscreenQuad();
screenTarget->Unbind();
```

## Design Philosophy

1. **Unified Interface:** RenderTarget provides unified interface for screen and texture rendering
2. **Resource Management:** Automatic FBO and depth buffer lifecycle management
3. **Flexibility:** Support shared texture and FBO objects
4. **Type Safety:** Use shared_ptr for object lifetime management
5. **Ease of Use:** Simplify common offscreen rendering scenarios

## Use Cases

### 1. Shadow Mapping

```cpp
// Create shadow map texture
auto shadowMap = std::make_shared<Texture2D>();
shadowMap->Create();
shadowMap->SetImage(2048, 2048, GL_DEPTH_COMPONENT16, 
                    GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);

// Create shadow map render target
auto shadowTarget = std::make_shared<RenderTarget>(shadowMap, false);
shadowTarget->Initialize();

// Render shadow map
shadowTarget->Bind();
shadowTarget->Clear(false, true, false);
RenderSceneFromLightPerspective();
shadowTarget->Unbind();
```

### 2. Reflection/Refraction

```cpp
// Create reflection texture
auto reflectionTexture = std::make_shared<Texture2D>();
reflectionTexture->Create();
reflectionTexture->SetImage(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

auto reflectionTarget = std::make_shared<RenderTarget>(reflectionTexture, true);
reflectionTarget->Initialize();

// Render reflection
reflectionTarget->Bind();
RenderSceneReflected();
reflectionTarget->Unbind();

// Use reflection texture in main render
reflectionTexture->Bind(0);
```

### 3. Multiple Render Targets (MRT)

While a single RenderTarget supports one color attachment, you can extend the FBO class for multiple attachments:

```cpp
// Attach multiple textures
fbo.AttachTexture2D(texture1->GetHandle(), GL_COLOR_ATTACHMENT0);
fbo.AttachTexture2D(texture2->GetHandle(), GL_COLOR_ATTACHMENT0 + 1);
fbo.AttachTexture2D(texture3->GetHandle(), GL_COLOR_ATTACHMENT0 + 2);
```

## Important Notes

- **Texture Size:** Ensure texture dimensions are powers of 2 for best performance (if using mipmaps)
- **Format Compatibility:** Check device support for specific internal formats
- **Framebuffer Completeness:** Always check `IsComplete()` or `GetStatus()`
- **Performance:** Avoid frequent render target switches
- **Memory:** Large textures consume significant GPU memory
- **Lifetime:** Use shared_ptr to ensure texture remains valid during RenderTarget usage

## References

- [OpenGL ES 3.0 Framebuffer Objects](https://www.khronos.org/opengl/wiki/Framebuffer_Object)
- [OpenGL ES 3.0 Textures](https://www.khronos.org/opengl/wiki/Texture)
