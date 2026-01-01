# Texture2D OES EXTERNAL 支持文档

## 概述

Texture2D 类现在支持 `GL_TEXTURE_EXTERNAL_OES` 纹理类型，这是 Android 平台用于摄像头预览和视频解码器输出的特殊纹理类型。

## 什么是 OES EXTERNAL 纹理？

`GL_TEXTURE_EXTERNAL_OES` 是一种特殊的纹理类型，用于：
- **Android 摄像头预览**：通过 `SurfaceTexture` 直接接收摄像头数据
- **视频解码器输出**：通过 `MediaCodec` 的 `Surface` 输出模式
- **零拷贝渲染**：数据直接在GPU内存中，无需CPU拷贝

### 优势
1. **零拷贝**：数据不经过CPU，直接在GPU内存
2. **高性能**：适合实时视频处理
3. **低延迟**：减少内存拷贝延迟
4. **格式自动处理**：YUV到RGB转换由硬件自动完成

### 限制
1. **不能使用 `glTexImage2D`**：数据来自外部源（SurfaceTexture）
2. **不能生成 mipmap**
3. **必须使用特殊着色器**：需要 `#extension GL_OES_EGL_image_external : require`
4. **Android 专有**：主要用于 Android 平台

## 使用方法

### 1. 创建 OES EXTERNAL 纹理

```cpp
#include "gl/Texture2D.h"
using namespace ului::gl;

// 创建外部纹理
auto externalTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
externalTexture->Create();

// 或者使用 Create 方法指定类型
auto externalTexture2 = std::make_shared<Texture2D>();
externalTexture2->Create(TextureType::TEXTURE_EXTERNAL_OES);
```

### 2. Android 摄像头集成

```cpp
// Android Java/Kotlin 代码
// 创建 SurfaceTexture
int textureId = texture->GetHandle();  // 从C++获取纹理ID
SurfaceTexture surfaceTexture = new SurfaceTexture(textureId);

// 设置给Camera
camera.setPreviewTexture(surfaceTexture);

// C++ 代码
// 设置纹理尺寸（用于渲染）
externalTexture->SetExternalTextureSize(1920, 1080);

// 绑定并渲染
externalTexture->Bind(0);
// ... OpenGL 渲染代码 ...
```

### 3. Android MediaCodec 视频解码

```cpp
// Android Java/Kotlin 代码
// 创建 Surface 用于解码输出
int textureId = texture->GetHandle();
SurfaceTexture surfaceTexture = new SurfaceTexture(textureId);
Surface surface = new Surface(surfaceTexture);

// 配置 MediaCodec
MediaCodec codec = MediaCodec.createDecoderByType("video/avc");
codec.configure(format, surface, null, 0);
codec.start();

// C++ 代码
// 每帧更新纹理
surfaceTexture.updateTexImage();  // Java 端调用

// 绑定并渲染
externalTexture->Bind(0);
// ... 渲染视频帧 ...
```

### 4. 特殊着色器要求

OES EXTERNAL 纹理需要特殊的着色器：

#### 顶点着色器（无特殊要求）
```glsl
#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}
```

#### 片段着色器（需要扩展声明）
```glsl
#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require

precision mediump float;

// 注意：使用 samplerExternalOES 而不是 sampler2D
uniform samplerExternalOES uTexture;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = texture(uTexture, vTexCoord);
}
```

## API 参考

### TextureType 枚举

```cpp
enum class TextureType {
    TEXTURE_2D,              // 标准 2D 纹理
    TEXTURE_EXTERNAL_OES     // 外部 OES 纹理（Android）
};
```

### 新增/修改的方法

#### `Create(TextureType type)`
创建指定类型的纹理
```cpp
bool Create(TextureType type = TextureType::TEXTURE_2D);
```

#### `SetExternalTextureSize(width, height)`
设置外部纹理的尺寸（用于渲染计算）
```cpp
void SetExternalTextureSize(GLsizei width, GLsizei height);
```

#### `IsExternalOES()`
检查是否为外部纹理
```cpp
bool IsExternalOES() const;
```

#### `GetTarget()`
获取 OpenGL 纹理目标
```cpp
GLenum GetTarget() const;  // 返回 GL_TEXTURE_2D 或 GL_TEXTURE_EXTERNAL_OES
```

### 行为差异

| 操作 | TEXTURE_2D | TEXTURE_EXTERNAL_OES |
|------|------------|----------------------|
| `SetImage()` | ✅ 支持 | ❌ 不支持（数据来自外部）|
| `SetSubImage()` | ✅ 支持 | ❌ 不支持 |
| `GenerateMipmap()` | ✅ 支持 | ❌ 不支持 |
| `SetFilter()` | ✅ 支持 | ✅ 支持（但限制较多）|
| `SetWrap()` | ✅ 支持 | ✅ 支持（通常只能CLAMP_TO_EDGE）|
| `Bind()` | ✅ 支持 | ✅ 支持 |

## 完整示例：Android 摄像头渲染

### C++ 代码

```cpp
#include "gl/Texture2D.h"
#include "gl/ShaderProgram.h"
#include "gl/VAO.h"
#include "gl/VBO.h"

using namespace ului::gl;

// 1. 创建外部纹理
auto cameraTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
cameraTexture->Create();
cameraTexture->SetExternalTextureSize(1920, 1080);

// 2. 创建着色器（使用 samplerExternalOES）
std::string vertShader = R"(
    #version 300 es
    in vec3 aPosition;
    in vec2 aTexCoord;
    out vec2 vTexCoord;
    void main() {
        vTexCoord = aTexCoord;
        gl_Position = vec4(aPosition, 1.0);
    }
)";

std::string fragShader = R"(
    #version 300 es
    #extension GL_OES_EGL_image_external_essl3 : require
    precision mediump float;
    uniform samplerExternalOES uTexture;
    in vec2 vTexCoord;
    out vec4 fragColor;
    void main() {
        fragColor = texture(uTexture, vTexCoord);
    }
)";

ShaderProgram shader;
shader.CreateFromSource(vertShader, fragShader);

// 3. 渲染循环
while (running) {
    // Java 端更新纹理：surfaceTexture.updateTexImage()
    
    shader.Use();
    cameraTexture->Bind(0);
    
    // 渲染全屏四边形
    DrawFullscreenQuad();
    
    shader.Unuse();
}
```

### Android Java 集成代码

```java
// 获取纹理ID
int textureId = getNativeTextureId();  // 从 C++ 获取

// 创建 SurfaceTexture
SurfaceTexture surfaceTexture = new SurfaceTexture(textureId);

// 设置给摄像头
Camera camera = Camera.open();
camera.setPreviewTexture(surfaceTexture);
camera.startPreview();

// 在渲染循环中更新纹理
surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
    @Override
    public void onFrameAvailable(SurfaceTexture st) {
        // 通知 C++ 有新帧
        nativeOnCameraFrameAvailable();
    }
});

// 更新纹理（在 GL 线程中调用）
surfaceTexture.updateTexImage();
```

## 注意事项

1. **线程同步**：`updateTexImage()` 必须在 OpenGL 渲染线程中调用
2. **纹理矩阵**：可能需要使用 `getTransformMatrix()` 获取纹理坐标变换矩阵
3. **格式限制**：只能使用 `GL_LINEAR` 过滤（某些设备不支持 `GL_NEAREST`）
4. **包裹模式**：通常只支持 `GL_CLAMP_TO_EDGE`
5. **平台限制**：OES EXTERNAL 主要用于 Android，其他平台可能不支持

## 性能建议

1. **零拷贝链路**：Camera → SurfaceTexture → GL_TEXTURE_EXTERNAL_OES → GPU
2. **避免 `glReadPixels`**：如果需要 CPU 访问，考虑使用 `ImageReader`
3. **使用硬件编码**：渲染结果可以直接输出到 MediaCodec 的 Surface
4. **异步处理**：使用 `setOnFrameAvailableListener` 实现异步帧处理

## 参考资料

- [Android SurfaceTexture 文档](https://developer.android.com/reference/android/graphics/SurfaceTexture)
- [GL_OES_EGL_image_external 扩展](https://www.khronos.org/registry/OpenGL/extensions/OES/OES_EGL_image_external.txt)
- [Android MediaCodec 文档](https://developer.android.com/reference/android/media/MediaCodec)
