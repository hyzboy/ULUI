# Windows Media Foundation 解码器集成文档

本文档介绍 ULUI 框架中的 Windows Media Foundation 视频解码器封装类。

## 概述

Media Foundation 是微软的现代媒体 API，提供：
- **硬件加速**：使用设备的专用解码硬件
- **高性能**：适合实时视频处理场景
- **广泛编解码器支持**：H.264、H.265、VP9 等

ULUI 提供 `MediaFoundationDecoder` 类用于 Windows 平台的视频解码。

## MediaFoundationDecoder（解码器）

### 创建和配置

```cpp
#include "windows/MediaFoundationDecoder.h"

// 创建 H.264 解码器（BUFFER 模式）
auto decoder = std::make_shared<MediaFoundationDecoder>();
bool success = decoder->Create(
    "video/avc",                           // MIME 类型：H.264
    MediaFoundationDecoder::Mode::BUFFER   // 模式：CPU 缓冲区
);

// 配置解码器尺寸
decoder->Configure(
    1920, 1080,  // 分辨率：1920x1080
    30.0f,       // 帧率：30 fps（可选）
    0            // 码率：自动（可选）
);

// 启动解码器
decoder->Start();
```

### 解码模式

#### 1. BUFFER 模式（推荐）

将视频帧解码到 CPU 可访问的 Bitmap 对象。

```cpp
auto decoder = std::make_shared<MediaFoundationDecoder>();
decoder->Create("video/avc", MediaFoundationDecoder::Mode::BUFFER);
decoder->Configure(1920, 1080);
decoder->Start();

// 创建用于接收解码帧的 bitmap
auto bitmap = std::make_shared<Bitmap>();

// 队列化编码的视频数据
const uint8_t* h264Data = ...; // 你的 H.264 编码数据
size_t dataSize = ...;         // 编码数据大小
int64_t timestampUs = ...;     // 呈现时间戳（微秒）
bool isKeyFrame = true;        // 是否为关键帧？

decoder->QueueInputBuffer(h264Data, dataSize, timestampUs, isKeyFrame);

// 获取解码帧
int64_t presentationTime;
if (decoder->GetDecodedBitmap(bitmap, &presentationTime)) {
    // 成功解码一帧
    // bitmap 现在包含 NV12 格式的解码视频帧
    
    // 如果需要，可以转换为其他格式
    bitmap->ConvertTo(PixelFormat::RGBA8);
    
    // 或上传到 OpenGL 纹理
    texture->UploadFromBitmap(bitmap);
    
    // 或保存到文件
    bitmap->SaveToFile("frame.png");
}
```

#### 2. TEXTURE 模式（未来功能）

直接 GPU 纹理输出（尚未实现）。

### 完整示例：视频文件解码

```cpp
#include "windows/MediaFoundationDecoder.h"
#include "Bitmap.h"

bool DecodeVideoFile(const char* videoPath) {
    // 创建解码器
    auto decoder = std::make_shared<MediaFoundationDecoder>();
    if (!decoder->Create("video/avc", MediaFoundationDecoder::Mode::BUFFER)) {
        LogError("创建解码器失败");
        return false;
    }
    
    // 配置解码器
    if (!decoder->Configure(1920, 1080, 30.0f)) {
        LogError("配置解码器失败");
        return false;
    }
    
    // 启动解码器
    if (!decoder->Start()) {
        LogError("启动解码器失败");
        return false;
    }
    
    // 创建输出 bitmap
    auto bitmap = std::make_shared<Bitmap>();
    
    // 读取和解码视频数据
    // （通常你会从文件或网络流读取）
    std::vector<uint8_t> encodedData = ReadVideoPacket(videoPath);
    int64_t timestamp = 0;
    int frameCount = 0;
    
    while (!encodedData.empty()) {
        // 队列化输入数据
        bool isKeyFrame = (frameCount % 30 == 0); // 假设每30帧一个关键帧
        if (!decoder->QueueInputBuffer(encodedData.data(), encodedData.size(), 
                                       timestamp, isKeyFrame)) {
            // 解码器可能需要先输出帧
            // 尝试获取解码帧
        }
        
        // 获取解码帧
        int64_t pts;
        while (decoder->GetDecodedBitmap(bitmap, &pts)) {
            LogInfo("解码第 %d 帧，时间戳 %lld", frameCount, pts);
            
            // 处理解码帧
            // - 如果需要转换格式
            // - 上传到纹理
            // - 保存到文件
            // - 应用特效等
            
            frameCount++;
        }
        
        // 读取下一个数据包
        encodedData = ReadVideoPacket(videoPath);
        timestamp += 33333; // ~30 fps（每帧约33.333毫秒，以微秒为单位）
    }
    
    // 发送流结束信号
    decoder->SignalEndOfStream();
    
    // 排空剩余帧
    int64_t pts;
    while (decoder->GetDecodedBitmap(bitmap, &pts)) {
        LogInfo("排空帧，时间戳 %lld", pts);
        frameCount++;
    }
    
    // 停止和清理
    decoder->Stop();
    decoder->Release();
    
    LogInfo("总共解码 %d 帧", frameCount);
    return true;
}
```

### 支持的视频编解码器

解码器通过 MIME 类型支持各种视频编解码器：

| MIME 类型 | 编解码器 | 描述 |
|-----------|---------|------|
| `video/avc` | H.264 | 最常见，广泛支持 |
| `video/x-h264` | H.264 | 替代 MIME 类型 |
| `video/hevc` | H.265/HEVC | 高效率，较新编解码器 |
| `video/x-h265` | H.265/HEVC | 替代 MIME 类型 |
| `video/mp4` | MPEG-4 | 传统 MPEG-4 视频 |
| `video/x-ms-wmv` | WMV | Windows Media Video |
| `video/mpeg` | MPEG-1 | 传统 MPEG-1 视频 |

### 错误处理

```cpp
auto decoder = std::make_shared<MediaFoundationDecoder>();

if (!decoder->Create("video/avc", MediaFoundationDecoder::Mode::BUFFER)) {
    LogError("创建解码器失败 - 检查编解码器支持");
    return false;
}

if (!decoder->Configure(1920, 1080)) {
    LogError("配置解码器失败 - 检查分辨率");
    decoder->Release();
    return false;
}

if (!decoder->Start()) {
    LogError("启动解码器失败");
    decoder->Release();
    return false;
}

// 带错误检查的队列输入
if (!decoder->QueueInputBuffer(data, size, timestamp, isKeyFrame)) {
    if (decoder->HasDecodedFrame()) {
        // 需要先排空输出
        decoder->GetDecodedBitmap(bitmap);
    } else {
        LogError("队列输入失败");
    }
}
```

### 资源管理

始终正确释放资源：

```cpp
// 选项 1：显式清理
decoder->Stop();
decoder->Release();

// 选项 2：通过析构函数自动清理
{
    auto decoder = std::make_shared<MediaFoundationDecoder>();
    // ... 使用解码器 ...
} // 超出作用域时自动释放
```

### 性能提示

1. **重用 Bitmap**：创建一个 bitmap 并在多帧中重用以避免分配
2. **批量处理**：在读取输出之前队列化多个输入缓冲区
3. **检查帧可用性**：在调用 `GetDecodedBitmap()` 前使用 `HasDecodedFrame()` 检查
4. **寻址时刷新**：寻址时调用 `Flush()` 清除解码器状态

```cpp
// 高效解码循环
auto bitmap = std::make_shared<Bitmap>(); // 重用同一个 bitmap

for (auto& packet : videoPackets) {
    decoder->QueueInputBuffer(packet.data, packet.size, packet.pts, packet.isKey);
    
    // 检查帧是否可用
    while (decoder->HasDecodedFrame()) {
        if (decoder->GetDecodedBitmap(bitmap, nullptr)) {
            ProcessFrame(bitmap); // 处理而不重新分配
        }
    }
}
```

### 故障排除

**问题**：解码器创建失败
- **解决方案**：确保你的 Windows 版本支持该编解码器。H.264 普遍支持。

**问题**：没有输出帧
- **解决方案**：检查你是否队列化了完整的视频数据包，并具有正确的时间戳和关键帧标志。

**问题**：性能差
- **解决方案**：验证硬件加速是否启用。检查 Windows 硬件加速设置。

**问题**：帧损坏
- **解决方案**：确保输入数据是有效的 H.264/H.265 基本流数据，而不是容器格式（MP4、MKV）。

## API 参考

### 方法

- `Create(mimeType, mode)` - 使用编解码器类型和模式初始化解码器
- `Configure(width, height, frameRate, bitrate)` - 设置视频参数
- `Start()` - 启动解码器
- `Stop()` - 停止解码器
- `Release()` - 释放所有资源
- `QueueInputBuffer(data, size, timestamp, isKeyFrame)` - 队列化编码数据
- `SignalEndOfStream()` - 发送无更多输入信号
- `GetDecodedBitmap(bitmap, timestamp)` - 获取解码帧
- `HasDecodedFrame()` - 检查帧是否可用
- `Flush()` - 清除解码器缓冲区
- `SetOutputFormat(format)` - 设置输出像素格式

### 属性

- `GetWidth()` - 获取配置的宽度
- `GetHeight()` - 获取配置的高度
- `GetMode()` - 获取解码模式
- `IsRunning()` - 检查解码器是否运行

## 与 ULUI 组件集成

### 与 OpenGL 纹理

```cpp
auto decoder = std::make_shared<MediaFoundationDecoder>();
auto bitmap = std::make_shared<Bitmap>();
auto texture = std::make_shared<gl::Texture2D>();

// 解码帧
decoder->GetDecodedBitmap(bitmap);

// 上传到纹理
bitmap->UploadToTexture(texture.get());

// 在渲染中使用纹理
texture->Bind(0);
// ... 渲染 ...
```

### 与文件系统

```cpp
// 保存解码帧
if (decoder->GetDecodedBitmap(bitmap)) {
    std::string path = FileSystem::GetExternalDataPath() + "frame_" + 
                       std::to_string(frameNum) + ".png";
    bitmap->SaveToFile(path.c_str());
}
```

## 另见

- [Bitmap 文档](BITMAP_CN.md)
- [Android MediaCodec 集成](MEDIACODEC_CN.md)
- [OpenGL 类文档](OPENGL_CLASSES_CN.md)
