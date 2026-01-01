# Android MediaCodec 集成文档

本文档介绍 ULUI 框架中的 Android MediaCodec 封装类，用于硬件视频编码和解码。

## 概述

MediaCodec 是 Android 提供的底层硬件编解码 API，支持：
- **硬件加速**：使用设备的专用编解码硬件
- **零拷贝模式**：通过 Surface 直接与 OpenGL 交互
- **高性能**：适合实时视频处理场景

ULUI 提供两个封装类：
- `MediaCodecEncoder` - 视频编码器
- `MediaCodecDecoder` - 视频解码器

## MediaCodecEncoder（编码器）

### 创建和配置

```cpp
#include "android/MediaCodecEncoder.h"

// 创建 H.264 编码器（SURFACE 模式 - 零拷贝）
auto encoder = std::make_shared<MediaCodecEncoder>();
bool success = encoder->Create(
    "video/avc",              // MIME 类型：H.264
    1920, 1080,               // 分辨率
    5000000,                  // 码率：5 Mbps
    30,                       // 帧率：30 fps
    MediaCodecEncoder::Mode::SURFACE  // 模式：零拷贝
);

// 启动编码器
encoder->Start();
```

### 两种编码模式

#### 1. SURFACE 模式（推荐 - 零拷贝）

直接从 OpenGL 渲染到编码器输入表面，无需 CPU 内存拷贝。

```cpp
// 创建编码器输入 RenderTarget
auto encoderTarget = encoder->CreateInputRenderTarget();

// 渲染循环
while (recording) {
    // 绑定编码器 RenderTarget
    encoderTarget->Bind();
    encoderTarget->Clear(true, false, false);
    
    // OpenGL 渲染（特效、合成等）
    cameraTexture->Bind(0);
    DrawFullscreenQuad();
    
    // 提交帧到编码器
    encoderTarget->Unbind();
    encoderTarget->SwapBuffers();
    
    // 获取编码输出
    ProcessEncodedOutput(encoder);
}
```

#### 2. BUFFER 模式

通过 CPU 内存传递数据，适用于非 OpenGL 源。

```cpp
auto encoder = std::make_shared<MediaCodecEncoder>();
encoder->Create("video/avc", 1920, 1080, 5000000, 30,
                MediaCodecEncoder::Mode::BUFFER);
encoder->Start();

// 编码 YUV 帧
int64_t timestamp = GetTimestampUs();
encoder->EncodeFrame(yuvData, dataSize, timestamp);

// 获取编码输出
ProcessEncodedOutput(encoder);
```

### 获取编码输出

```cpp
void ProcessEncodedOutput(std::shared_ptr<MediaCodecEncoder> encoder) {
    uint8_t* buffer;
    size_t size;
    int64_t pts;
    uint32_t flags;
    
    // 轮询编码输出
    while (encoder->GetEncodedData(&buffer, &size, &pts, &flags)) {
        // 检查是否为关键帧
        bool isKeyFrame = (flags & AMEDIACODEC_BUFFER_FLAG_KEY_FRAME);
        
        // 检查是否为配置数据（SPS/PPS）
        bool isConfig = (flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG);
        
        if (isConfig) {
            // 保存 SPS/PPS 用于流头部
            SaveConfigData(buffer, size);
        } else {
            // 发送编码帧数据
            SendEncodedFrame(buffer, size, pts, isKeyFrame);
        }
        
        // 释放输出缓冲区
        encoder->ReleaseOutputBuffer(encoder->m_currentOutputIndex);
    }
}
```

### 结束编码

```cpp
// 通知编码器流结束
encoder->SignalEndOfStream();

// 获取剩余输出
ProcessEncodedOutput(encoder);

// 停止和释放
encoder->Stop();
encoder->Release();
```

## MediaCodecDecoder（解码器）

### 创建和配置

```cpp
#include "android/MediaCodecDecoder.h"

// 创建 H.264 解码器（SURFACE 模式 - 零拷贝）
auto decoder = std::make_shared<MediaCodecDecoder>();
decoder->Create("video/avc", MediaCodecDecoder::Mode::SURFACE);

// 设置输出 Surface（从 SurfaceTexture 获取）
// Java: SurfaceTexture surfaceTexture = new SurfaceTexture(textureId);
// Java: Surface surface = new Surface(surfaceTexture);
// C++: ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
decoder->SetOutputSurface(nativeWindow);

// 配置解码器（包含 SPS/PPS）
decoder->Configure(1920, 1080, spsData, spsSize, ppsData, ppsSize);

// 启动解码器
decoder->Start();
```

### 两种解码模式

#### 1. SURFACE 模式（推荐 - 零拷贝）

解码输出直接到 OES EXTERNAL 纹理，可立即用于 OpenGL 渲染。

```cpp
// 创建 OES EXTERNAL 纹理接收解码输出
auto videoTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
videoTexture->Create();
videoTexture->SetExternalTextureSize(1920, 1080);

// 从纹理创建 SurfaceTexture（需要 JNI 调用）
// Java: SurfaceTexture st = new SurfaceTexture(videoTexture->GetHandle());
// Java: Surface surface = new Surface(st);
ANativeWindow* window = GetWindowFromSurfaceTexture(surfaceTexture);

// 设置解码器输出
decoder->SetOutputSurface(window);
decoder->Configure(1920, 1080);
decoder->Start();

// 解码循环
while (hasData) {
    // 输入编码数据
    decoder->QueueInputBuffer(encodedData, dataSize, timestamp, 0);
    
    // 渲染解码帧（零拷贝到纹理）
    if (decoder->RenderFrame()) {
        // Java: surfaceTexture.updateTexImage();
        
        // 使用解码纹理渲染
        videoTexture->Bind(0);
        DrawVideoQuad();
    }
}
```

#### 2. BUFFER 模式

解码输出到 CPU 内存，可转换为 Bitmap。

```cpp
auto decoder = std::make_shared<MediaCodecDecoder>();
decoder->Create("video/avc", MediaCodecDecoder::Mode::BUFFER);
decoder->Configure(1920, 1080);
decoder->Start();

// 解码循环
while (hasData) {
    // 输入编码数据
    decoder->QueueInputBuffer(encodedData, dataSize, timestamp, 0);
    
    // 获取解码数据
    uint8_t* buffer;
    size_t size;
    int64_t pts;
    uint32_t flags;
    
    if (decoder->GetDecodedData(&buffer, &size, &pts, &flags)) {
        // 处理 YUV 数据
        ProcessYUVFrame(buffer, size, 1920, 1080);
        
        // 释放输出缓冲区
        decoder->ReleaseOutputBuffer(decoder->m_currentOutputIndex, false);
    }
}
```

## 完整示例：相机预览录制

结合 OES EXTERNAL 纹理、OpenGL 渲染和 MediaCodec 编码的完整零拷贝管线。

```cpp
#include "android/MediaCodecEncoder.h"
#include "gl/Texture2D.h"
#include "gl/RenderTarget.h"

class CameraRecorder {
public:
    bool Initialize(int width, int height) {
        // 1. 创建相机输入纹理（OES EXTERNAL）
        m_cameraTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
        m_cameraTexture->Create();
        m_cameraTexture->SetExternalTextureSize(width, height);
        
        // Java: 创建 SurfaceTexture 连接相机
        // SurfaceTexture st = new SurfaceTexture(m_cameraTexture->GetHandle());
        // camera.setPreviewTexture(st);
        
        // 2. 创建编码器（SURFACE 模式）
        m_encoder = std::make_shared<MediaCodecEncoder>();
        if (!m_encoder->Create("video/avc", width, height, 
                               5000000, 30, 
                               MediaCodecEncoder::Mode::SURFACE)) {
            return false;
        }
        
        if (!m_encoder->Start()) {
            return false;
        }
        
        // 3. 创建编码器输入 RenderTarget
        m_encoderTarget = m_encoder->CreateInputRenderTarget();
        if (!m_encoderTarget) {
            return false;
        }
        
        m_width = width;
        m_height = height;
        m_recording = false;
        
        return true;
    }
    
    void StartRecording() {
        m_recording = true;
        m_frameCount = 0;
    }
    
    void ProcessFrame() {
        if (!m_recording) {
            return;
        }
        
        // Java: surfaceTexture.updateTexImage(); // 更新相机纹理
        
        // 渲染相机画面到编码器（可添加特效、水印等）
        m_encoderTarget->Bind();
        m_encoderTarget->Clear(true, false, false);
        
        // 绑定相机纹理并渲染
        m_cameraTexture->Bind(0);
        RenderFullscreenQuad();  // 使用 samplerExternalOES 着色器
        
        m_encoderTarget->Unbind();
        m_encoderTarget->SwapBuffers();  // 提交帧到编码器
        
        // 获取编码输出
        ProcessEncodedOutput();
        
        m_frameCount++;
    }
    
    void StopRecording() {
        if (!m_recording) {
            return;
        }
        
        m_recording = false;
        
        // 通知编码器流结束
        m_encoder->SignalEndOfStream();
        
        // 获取剩余编码数据
        ProcessEncodedOutput();
        
        m_encoder->Stop();
    }
    
    void Release() {
        StopRecording();
        m_encoder->Release();
        m_encoderTarget = nullptr;
        m_cameraTexture = nullptr;
    }
    
private:
    void ProcessEncodedOutput() {
        uint8_t* buffer;
        size_t size;
        int64_t pts;
        uint32_t flags;
        
        while (m_encoder->GetEncodedData(&buffer, &size, &pts, &flags)) {
            bool isKeyFrame = (flags & AMEDIACODEC_BUFFER_FLAG_KEY_FRAME);
            bool isConfig = (flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG);
            bool isEOS = (flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
            
            if (isConfig) {
                // 保存 SPS/PPS
                m_configData.assign(buffer, buffer + size);
            } else if (!isEOS) {
                // 写入 H.264 帧到文件或网络
                WriteVideoFrame(buffer, size, pts, isKeyFrame);
            }
            
            m_encoder->ReleaseOutputBuffer(m_encoder->m_currentOutputIndex);
            
            if (isEOS) {
                break;
            }
        }
    }
    
    void WriteVideoFrame(uint8_t* data, size_t size, int64_t pts, bool isKeyFrame) {
        // 写入 MP4 文件或发送到 RTMP 服务器
        // ...
    }
    
    void RenderFullscreenQuad() {
        // 使用全屏四边形渲染，着色器使用 samplerExternalOES
        // ...
    }
    
private:
    std::shared_ptr<Texture2D> m_cameraTexture;
    std::shared_ptr<MediaCodecEncoder> m_encoder;
    std::shared_ptr<RenderTarget> m_encoderTarget;
    
    int m_width;
    int m_height;
    bool m_recording;
    int m_frameCount;
    std::vector<uint8_t> m_configData;
};
```

## 完整示例：视频播放

```cpp
#include "android/MediaCodecDecoder.h"
#include "gl/Texture2D.h"

class VideoPlayer {
public:
    bool Initialize(const char* videoPath) {
        // 1. 创建解码输出纹理（OES EXTERNAL）
        m_videoTexture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
        m_videoTexture->Create();
        
        // 2. 创建解码器（SURFACE 模式）
        m_decoder = std::make_shared<MediaCodecDecoder>();
        if (!m_decoder->Create("video/avc", MediaCodecDecoder::Mode::SURFACE)) {
            return false;
        }
        
        // 3. 从纹理创建 SurfaceTexture（需要 JNI）
        // Java: SurfaceTexture st = new SurfaceTexture(m_videoTexture->GetHandle());
        // Java: Surface surface = new Surface(st);
        ANativeWindow* window = CreateWindowFromTexture(m_videoTexture);
        m_decoder->SetOutputSurface(window);
        
        // 4. 从视频文件读取 SPS/PPS 和配置解码器
        uint8_t* sps; size_t spsSize;
        uint8_t* pps; size_t ppsSize;
        int width, height;
        
        if (!ParseVideoHeader(videoPath, &width, &height, &sps, &spsSize, &pps, &ppsSize)) {
            return false;
        }
        
        m_decoder->Configure(width, height, sps, spsSize, pps, ppsSize);
        m_videoTexture->SetExternalTextureSize(width, height);
        
        // 5. 启动解码器
        if (!m_decoder->Start()) {
            return false;
        }
        
        m_videoPath = videoPath;
        return true;
    }
    
    void Update() {
        // 从视频文件读取 H.264 NAL 单元
        uint8_t* nalData;
        size_t nalSize;
        int64_t timestamp;
        uint32_t flags = 0;
        
        if (ReadNextNAL(&nalData, &nalSize, &timestamp)) {
            // 输入编码数据到解码器
            m_decoder->QueueInputBuffer(nalData, nalSize, timestamp, flags);
        }
        
        // 渲染解码帧到纹理
        if (m_decoder->RenderFrame()) {
            // Java: surfaceTexture.updateTexImage();
            m_hasNewFrame = true;
        }
    }
    
    void Render() {
        if (!m_hasNewFrame) {
            return;
        }
        
        // 使用解码纹理渲染
        m_videoTexture->Bind(0);
        RenderVideoQuad();
        
        m_hasNewFrame = false;
    }
    
    void Release() {
        m_decoder->Stop();
        m_decoder->Release();
        m_videoTexture = nullptr;
    }
    
private:
    bool ParseVideoHeader(const char* path, int* width, int* height,
                         uint8_t** sps, size_t* spsSize,
                         uint8_t** pps, size_t* ppsSize) {
        // 解析 MP4 文件头获取 SPS/PPS
        // ...
        return true;
    }
    
    bool ReadNextNAL(uint8_t** data, size_t* size, int64_t* timestamp) {
        // 从 MP4 文件读取下一个 NAL 单元
        // ...
        return true;
    }
    
    void RenderVideoQuad() {
        // 渲染视频画面
        // ...
    }
    
private:
    std::shared_ptr<Texture2D> m_videoTexture;
    std::shared_ptr<MediaCodecDecoder> m_decoder;
    std::string m_videoPath;
    bool m_hasNewFrame;
};
```

## 性能优化建议

### 1. 使用 SURFACE 模式

SURFACE 模式避免 CPU 内存拷贝，性能远优于 BUFFER 模式：
- **编码器**：OpenGL → MediaCodec 零拷贝
- **解码器**：MediaCodec → OpenGL 零拷贝

### 2. 合理设置编码参数

```cpp
// 1080p 高质量：5-8 Mbps
encoder->Create("video/avc", 1920, 1080, 6000000, 30, Mode::SURFACE);

// 720p 标准质量：2-4 Mbps
encoder->Create("video/avc", 1280, 720, 3000000, 30, Mode::SURFACE);

// 480p 低质量：1-2 Mbps
encoder->Create("video/avc", 854, 480, 1500000, 30, Mode::SURFACE);
```

### 3. 异步处理编码输出

在独立线程处理编码输出，避免阻塞渲染循环：

```cpp
std::thread outputThread([this]() {
    while (m_recording) {
        ProcessEncodedOutput();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
});
```

### 4. 使用 H.265 提高压缩率

H.265 (HEVC) 比 H.264 压缩效率高约 50%：

```cpp
// H.265 编码
encoder->Create("video/hevc", 1920, 1080, 3000000, 30, Mode::SURFACE);

// 需要检查设备是否支持
// AMediaCodec* codec = AMediaCodec_createEncoderByType("video/hevc");
// if (codec == nullptr) { /* 不支持 H.265 */ }
```

## 注意事项

### 1. 线程安全

MediaCodec 不是线程安全的，所有操作应在同一线程进行。

### 2. EGL 上下文

SURFACE 模式需要有效的 EGL 上下文。RenderTarget 会自动管理上下文切换。

### 3. 格式支持

不同设备支持的编解码格式不同，使用前应检查：
- H.264 (video/avc) - 所有设备都支持
- H.265 (video/hevc) - 较新设备支持
- VP8 (video/x-vnd.on2.vp8) - 部分设备支持
- VP9 (video/x-vnd.on2.vp9) - 较新设备支持

### 4. 内存管理

及时释放输出缓冲区，避免编码器/解码器阻塞：

```cpp
// 处理完输出后立即释放
encoder->ReleaseOutputBuffer(index);
decoder->ReleaseOutputBuffer(index, render);
```

## 错误处理

```cpp
// 创建失败
if (!encoder->Create(...)) {
    // 检查 MIME 类型是否支持
    // 检查参数是否合理
}

// 启动失败
if (!encoder->Start()) {
    // 检查是否已配置
    // 检查 SURFACE 模式是否正确设置
}

// 获取输出失败
if (!encoder->GetEncodedData(...)) {
    // 正常情况，表示暂无输出
    // 继续编码更多帧
}
```

## 相关文档

- [TEXTURE_OES_EXTERNAL_CN.md](TEXTURE_OES_EXTERNAL_CN.md) - OES EXTERNAL 纹理使用
- [RENDERTARGET_EGLSURFACE_CN.md](RENDERTARGET_EGLSURFACE_CN.md) - EGLSurface RenderTarget
- [BITMAP_CN.md](BITMAP_CN.md) - Bitmap 类和视频管线
