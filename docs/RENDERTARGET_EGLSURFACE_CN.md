# RenderTarget EGLSurface 支持文档

## 概述

RenderTarget 类现在支持 Android 下的 EGLSurface 渲染目标，专门用于对接 Android MediaCodec 的硬件编码器输入表面。

## 什么是 EGLSurface 渲染目标？

EGLSurface 是 Android 平台上的一种特殊渲染目标，用于：
- **硬件视频编码**：通过 `AMediaCodec_createInputSurface` 创建的编码器输入表面
- **零拷贝编码**：OpenGL 渲染结果直接进入编码器，无需 CPU 读回
- **高性能视频处理**：适合实时视频特效、滤镜、合成等场景

### 优势
1. **零拷贝**：GPU 渲染结果直接送入硬件编码器
2. **低延迟**：避免 `glReadPixels` 的 CPU 读回开销
3. **硬件加速**：充分利用 GPU 和编码器硬件
4. **高效能**：适合实时视频处理和直播场景

### 典型应用场景
- **实时视频滤镜**：摄像头预览 + OpenGL 特效 + 实时编码
- **视频转码**：解码 + OpenGL 处理 + 重新编码
- **屏幕录制**：屏幕内容 + OpenGL 渲染 + 编码
- **视频合成**：多路视频 + OpenGL 混合 + 编码输出

## 使用方法

### 1. Android MediaCodec 集成

#### Java/Kotlin 端代码

```java
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.view.Surface;

// 配置 MediaCodec 编码器
MediaFormat format = MediaFormat.createVideoFormat("video/avc", 1920, 1080);
format.setInteger(MediaFormat.KEY_BIT_RATE, 2000000);
format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
format.setInteger(MediaFormat.KEY_COLOR_FORMAT, 
                  MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

MediaCodec encoder = MediaCodec.createEncoderByType("video/avc");
encoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

// 创建输入 Surface
Surface inputSurface = encoder.createInputSurface();
encoder.start();

// 将 Surface 传递给 C++ 层
long nativeSurfaceHandle = getNativeSurfaceHandle(inputSurface);
nativeSetupEncoderSurface(nativeSurfaceHandle, 1920, 1080);
```

#### C++ 端代码

```cpp
#include "gl/RenderTarget.h"
#include <android/native_window_jni.h>

using namespace ului::gl;

// JNI 接口：从 Java Surface 获取 ANativeWindow
extern "C" JNIEXPORT void JNICALL
Java_com_example_VideoEncoder_nativeSetupEncoderSurface(
    JNIEnv* env, jobject obj, jobject surface, jint width, jint height)
{
    // 获取 ANativeWindow
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGE("Failed to get ANativeWindow from Surface");
        return;
    }
    
    // 获取当前 EGL 上下文
    EGLDisplay display = eglGetCurrentDisplay();
    if (display == EGL_NO_DISPLAY) {
        LOGE("No current EGL display");
        ANativeWindow_release(window);
        return;
    }
    
    // 创建 EGLSurface
    EGLint surfaceAttribs[] = { EGL_NONE };
    EGLSurface eglSurface = eglCreateWindowSurface(
        display, 
        getCurrentEGLConfig(),  // 你的 EGL config
        window, 
        surfaceAttribs
    );
    
    if (eglSurface == EGL_NO_SURFACE) {
        EGLint error = eglGetError();
        LOGE("Failed to create EGL surface: 0x%x", error);
        ANativeWindow_release(window);
        return;
    }
    
    // 创建 RenderTarget
    auto encoderTarget = std::make_shared<RenderTarget>(
        display, eglSurface, width, height
    );
    
    if (!encoderTarget->Initialize()) {
        LOGE("Failed to initialize encoder render target");
        eglDestroySurface(display, eglSurface);
        ANativeWindow_release(window);
        return;
    }
    
    // 保存 RenderTarget 供后续使用
    g_encoderRenderTarget = encoderTarget;
    
    // 注意：不要释放 window，它由 RenderTarget 管理
}
```

### 2. 渲染到编码器

```cpp
// 渲染循环
while (encoding) {
    // 1. 绑定编码器渲染目标
    g_encoderRenderTarget->Bind();
    
    // 2. 清空缓冲区
    g_encoderRenderTarget->Clear(true, true, false);
    
    // 3. OpenGL 渲染（应用滤镜、特效等）
    shader.Use();
    cameraTexture->Bind(0);
    DrawFullscreenQuad();
    shader.Unuse();
    
    // 4. 交换缓冲区，提交帧到编码器
    if (!g_encoderRenderTarget->SwapBuffers()) {
        LOGE("Failed to swap buffers");
        break;
    }
    
    // 5. 恢复之前的渲染上下文
    g_encoderRenderTarget->Unbind();
    
    // 6. 通知 MediaCodec 有新帧（Java 端）
    // notifyFrameAvailable();
}
```

### 3. 完整的视频处理管线

#### 摄像头 → OpenGL 特效 → 硬件编码

```cpp
#include "gl/Texture2D.h"
#include "gl/RenderTarget.h"
#include "gl/ShaderProgram.h"

// 1. 创建摄像头纹理（OES EXTERNAL）
auto cameraTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
cameraTexture->Create();
cameraTexture->SetExternalTextureSize(1920, 1080);

// Java: SurfaceTexture cameraSurfaceTexture = new SurfaceTexture(cameraTexture->GetHandle())
// Java: camera.setPreviewTexture(cameraSurfaceTexture)

// 2. 创建编码器渲染目标（通过 JNI 设置）
// std::shared_ptr<RenderTarget> encoderTarget = ...; // 从上面的 JNI 函数获取

// 3. 加载特效着色器
ShaderProgram filterShader;
filterShader.CreateFromSource(vertexShader, fragmentShader);

// 4. 渲染循环
while (recording) {
    // 更新摄像头纹理
    // Java: cameraSurfaceTexture.updateTexImage()
    
    // 绑定编码器目标
    encoderTarget->Bind();
    encoderTarget->Clear(true, false, false);
    
    // 应用特效渲染
    filterShader.Use();
    cameraTexture->Bind(0);
    
    // 设置特效参数
    // filterShader.SetUniform(...);
    
    // 渲染全屏四边形
    DrawFullscreenQuad();
    
    filterShader.Unuse();
    
    // 提交帧到编码器
    encoderTarget->SwapBuffers();
    encoderTarget->Unbind();
}

// 5. 清理
// Java: encoder.signalEndOfInputStream()
// Java: encoder.stop()
// Java: encoder.release()
```

## API 参考

### 构造函数（Android 专用）

```cpp
#ifdef __ANDROID__
RenderTarget(EGLDisplay eglDisplay, EGLSurface eglSurface, 
             GLsizei width, GLsizei height);
#endif
```

**参数：**
- `eglDisplay`: EGL 显示句柄
- `eglSurface`: EGL 表面句柄（从 MediaCodec 创建）
- `width`: 表面宽度
- `height`: 表面高度

### 核心方法

#### `Initialize()`
初始化 EGLSurface 渲染目标，创建共享的 EGL 上下文

```cpp
bool Initialize();
```

**返回值：** 成功返回 `true`，失败返回 `false`

#### `Bind()`
绑定 EGLSurface 进行渲染，自动切换 EGL 上下文

```cpp
void Bind() const;
```

#### `Unbind()`
解绑 EGLSurface，恢复之前的 EGL 上下文

```cpp
void Unbind() const;
```

#### `SwapBuffers()`（Android 专用）
交换缓冲区，将渲染结果提交到 MediaCodec 编码器

```cpp
#ifdef __ANDROID__
bool SwapBuffers();
#endif
```

**返回值：** 成功返回 `true`，失败返回 `false`

**重要：** 必须在每帧渲染完成后调用，否则编码器不会接收到帧数据

#### `GetEGLSurface()`（Android 专用）
获取 EGL 表面句柄

```cpp
#ifdef __ANDROID__
EGLSurface GetEGLSurface() const;
#endif
```

## 渲染目标类型对比

| 特性 | Screen | Texture | EGLSurface (Android) |
|------|--------|---------|----------------------|
| 渲染目标 | 默认帧缓冲 | FBO + Texture2D | MediaCodec Surface |
| 用途 | 屏幕显示 | 离屏渲染/后处理 | 视频编码 |
| 性能 | 高 | 高 | 极高（零拷贝） |
| 需要 `Initialize()` | ❌ | ✅ | ✅ |
| 需要 `SwapBuffers()` | ❌ | ❌ | ✅ |
| 平台限制 | 无 | 无 | Android 专用 |

## 注意事项

### 1. EGL 上下文管理
- RenderTarget 会自动创建共享的 EGL 上下文
- `Bind()` 时自动切换到 EGLSurface 上下文
- `Unbind()` 时自动恢复之前的上下文
- 不要手动调用 `eglMakeCurrent` 管理 EGLSurface 的上下文

### 2. SwapBuffers 必须调用
```cpp
// ❌ 错误：忘记调用 SwapBuffers
encoderTarget->Bind();
RenderScene();
encoderTarget->Unbind();  // 编码器不会收到帧！

// ✅ 正确：调用 SwapBuffers
encoderTarget->Bind();
RenderScene();
encoderTarget->SwapBuffers();  // 提交帧到编码器
encoderTarget->Unbind();
```

### 3. 生命周期管理
- RenderTarget 不拥有 `EGLSurface`（由 MediaCodec 管理）
- 只拥有自己创建的共享 `EGLContext`
- 销毁顺序：RenderTarget → MediaCodec → Surface

### 4. 线程安全
- EGL 上下文是线程局部的
- 确保在同一个 OpenGL 线程中操作 RenderTarget
- MediaCodec 编码通常在独立线程

### 5. 性能最佳实践
- 避免频繁创建/销毁 RenderTarget
- 使用对象池复用 RenderTarget
- 监控 `eglSwapBuffers` 的耗时
- 使用 MediaCodec 的异步模式

## 完整示例：视频录制应用

### 架构设计

```
[Camera] → [OES Texture] → [Filter Shader] → [EGLSurface RenderTarget] → [MediaCodec] → [MP4 File]
   ↓                                                                               ↓
[Preview Screen]                                                            [Encoded Stream]
```

### 代码结构

```cpp
class VideoRecorder {
private:
    // 摄像头纹理
    std::shared_ptr<Texture2D> m_cameraTexture;
    
    // 编码器渲染目标
    std::shared_ptr<RenderTarget> m_encoderTarget;
    
    // 预览渲染目标
    std::shared_ptr<RenderTarget> m_previewTarget;
    
    // 滤镜着色器
    std::shared_ptr<ShaderProgram> m_filterShader;
    
public:
    bool Initialize(JNIEnv* env, jobject encoderSurface, int width, int height);
    void RenderFrame();
    void Cleanup();
};

bool VideoRecorder::Initialize(JNIEnv* env, jobject encoderSurface, 
                                int width, int height)
{
    // 1. 创建摄像头纹理
    m_cameraTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
    if (!m_cameraTexture->Create()) {
        return false;
    }
    
    // 2. 设置编码器渲染目标（从 Surface 创建 EGLSurface）
    // ... EGL 初始化代码 ...
    m_encoderTarget = std::make_shared<RenderTarget>(display, eglSurface, width, height);
    if (!m_encoderTarget->Initialize()) {
        return false;
    }
    
    // 3. 创建预览渲染目标
    m_previewTarget = std::make_shared<RenderTarget>(width, height);
    
    // 4. 加载滤镜着色器
    m_filterShader = std::make_shared<ShaderProgram>();
    m_filterShader->CreateFromSource(vertexSrc, fragmentSrc);
    
    return true;
}

void VideoRecorder::RenderFrame()
{
    // 更新摄像头纹理
    // updateCameraTexture();
    
    // 渲染函数（预览和编码器共用）
    auto renderScene = [this]() {
        m_filterShader->Use();
        m_cameraTexture->Bind(0);
        DrawFullscreenQuad();
        m_filterShader->Unuse();
    };
    
    // 1. 渲染到编码器
    m_encoderTarget->Bind();
    m_encoderTarget->Clear(true, false, false);
    renderScene();
    m_encoderTarget->SwapBuffers();
    m_encoderTarget->Unbind();
    
    // 2. 渲染到预览屏幕
    m_previewTarget->Bind();
    m_previewTarget->Clear(true, false, false);
    renderScene();
    m_previewTarget->Unbind();
}
```

## 故障排查

### 问题：`eglCreateContext` 失败
**原因：** 没有当前的 EGL 上下文或配置不匹配  
**解决：** 确保在有效的 OpenGL 上下文中调用 `Initialize()`

### 问题：编码器收不到帧
**原因：** 忘记调用 `SwapBuffers()`  
**解决：** 每帧渲染后必须调用 `SwapBuffers()`

### 问题：画面撕裂或不完整
**原因：** `SwapBuffers()` 时间点不正确  
**解决：** 在所有渲染命令执行完毕后调用 `SwapBuffers()`

### 问题：性能不佳
**原因：** EGL 上下文切换开销  
**解决：** 批量渲染多个帧，减少上下文切换次数

## 参考资料

- [Android MediaCodec 官方文档](https://developer.android.com/reference/android/media/MediaCodec)
- [EGL 1.5 规范](https://www.khronos.org/registry/EGL/specs/eglspec.1.5.pdf)
- [Android 视频编码最佳实践](https://developer.android.com/guide/topics/media/media-formats)
