# Windows Media Foundation 编码器集成文档

本文档介绍 ULUI 框架中的 Windows Media Foundation 视频编码器封装类。

## 概述

Media Foundation 是微软的现代媒体 API，提供：
- **硬件加速**：使用设备的专用编码硬件
- **高性能**：适合实时视频处理场景
- **广泛编解码器支持**：H.264、H.265、WMV 等

ULUI 提供 `MediaFoundationEncoder` 类用于 Windows 平台的视频编码。

## MediaFoundationEncoder（编码器）

### 创建和配置

```cpp
#include "windows/MediaFoundationEncoder.h"

// 创建 H.264 编码器（BUFFER 模式）
auto encoder = std::make_shared<MediaFoundationEncoder>();
bool success = encoder->Create(
    "video/avc",                           // MIME 类型：H.264
    1920, 1080,                            // 分辨率
    5000000,                               // 码率：5 Mbps
    30,                                    // 帧率：30 fps
    MediaFoundationEncoder::Mode::BUFFER   // 模式：CPU 缓冲区
);

// 启动编码器
encoder->Start();
```

### 编码模式

#### 1. BUFFER 模式（推荐）

从 CPU 可访问的 Bitmap 对象编码视频帧。

```cpp
auto encoder = std::make_shared<MediaFoundationEncoder>();
encoder->Create("video/avc", 1920, 1080, 5000000, 30, 
                MediaFoundationEncoder::Mode::BUFFER);
encoder->Start();

// 创建包含视频帧数据的 bitmap
auto bitmap = std::make_shared<Bitmap>();
// ... 填充 bitmap 帧数据（例如从相机、屏幕捕获等）

// 确保 bitmap 是 NV12 格式（或编码器将自动转换）
if (bitmap->GetPixelFormat() != PixelFormat::NV12) {
    bitmap->ConvertTo(PixelFormat::NV12);
}

// 编码帧
int64_t timestampUs = GetCurrentTimestampMicroseconds();
bool forceKeyFrame = (frameCount % (30 * keyframeInterval) == 0);
encoder->EncodeFrame(bitmap, timestampUs, forceKeyFrame);

// 获取编码输出
uint8_t* encodedData;
size_t encodedSize;
int64_t pts;
bool isKeyFrame;

while (encoder->HasEncodedData()) {
    if (encoder->GetEncodedData(&encodedData, &encodedSize, &pts, &isKeyFrame)) {
        // 将编码数据写入文件或流
        WriteToFile(encodedData, encodedSize);
        
        // 检查是否为关键帧
        if (isKeyFrame) {
            LogInfo("编码关键帧，时间戳 %lld", pts);
        }
    }
}
```

#### 2. TEXTURE 模式（未来功能）

直接 GPU 纹理输入（尚未实现）。

### 完整示例：视频捕获和编码

```cpp
#include "windows/MediaFoundationEncoder.h"
#include "Bitmap.h"
#include "file_system.h"

bool CaptureAndEncodeVideo(int durationSeconds) {
    // 创建编码器
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    if (!encoder->Create("video/avc", 1920, 1080, 5000000, 30, 
                         MediaFoundationEncoder::Mode::BUFFER)) {
        LogError("创建编码器失败");
        return false;
    }
    
    // 设置关键帧间隔（每2秒）
    encoder->SetKeyframeInterval(2);
    
    // 启动编码器
    if (!encoder->Start()) {
        LogError("启动编码器失败");
        return false;
    }
    
    // 打开输出文件
    std::string outputPath = FileSystem::GetExternalDataPath() + "output.h264";
    FILE* outputFile = fopen(outputPath.c_str(), "wb");
    if (!outputFile) {
        LogError("打开输出文件失败");
        return false;
    }
    
    // 获取编解码器配置数据（H.264 的 SPS/PPS）
    uint8_t* configData;
    size_t configSize;
    
    // 为帧创建 bitmap
    auto bitmap = std::make_shared<Bitmap>();
    BitmapFormat format(PixelFormat::NV12, 1920, 1080);
    bitmap->Create(format);
    
    int frameCount = 0;
    int totalFrames = durationSeconds * 30; // 30 fps
    int64_t frameIntervalUs = 1000000 / 30; // 每帧微秒数
    
    for (int i = 0; i < totalFrames; i++) {
        // 捕获帧（从相机、屏幕等）
        CaptureFrame(bitmap);
        
        // 计算时间戳
        int64_t timestamp = i * frameIntervalUs;
        
        // 每2秒强制关键帧
        bool forceKeyFrame = (i % (30 * 2) == 0);
        
        // 编码帧
        if (!encoder->EncodeFrame(bitmap, timestamp, forceKeyFrame)) {
            LogWarning("编码第 %d 帧失败，需要排空输出", i);
        }
        
        // 获取编码输出
        uint8_t* encodedData;
        size_t encodedSize;
        int64_t pts;
        bool isKeyFrame;
        
        while (encoder->HasEncodedData()) {
            if (encoder->GetEncodedData(&encodedData, &encodedSize, &pts, &isKeyFrame)) {
                // 写入文件
                fwrite(encodedData, 1, encodedSize, outputFile);
                frameCount++;
                
                if (isKeyFrame) {
                    LogInfo("编码关键帧 %d/%d", frameCount, totalFrames);
                }
            }
        }
        
        // 从第一个关键帧获取配置数据
        if (i == 0 && encoder->GetConfigData(&configData, &configSize)) {
            LogInfo("获得编解码器配置数据：%zu 字节", configSize);
            // 你可能想要单独保存 SPS/PPS 以便 MP4 封装
        }
    }
    
    // 发送流结束信号
    encoder->SignalEndOfStream();
    
    // 排空剩余编码帧
    uint8_t* encodedData;
    size_t encodedSize;
    int64_t pts;
    bool isKeyFrame;
    
    while (encoder->HasEncodedData()) {
        if (encoder->GetEncodedData(&encodedData, &encodedSize, &pts, &isKeyFrame)) {
            fwrite(encodedData, 1, encodedSize, outputFile);
            frameCount++;
        }
    }
    
    // 清理
    fclose(outputFile);
    encoder->Stop();
    encoder->Release();
    
    LogInfo("编码 %d 帧到 %s", frameCount, outputPath.c_str());
    return true;
}
```

### 支持的视频编解码器

编码器通过 MIME 类型支持各种视频编解码器：

| MIME 类型 | 编解码器 | 描述 |
|-----------|---------|------|
| `video/avc` | H.264 | 最常见，广泛支持 |
| `video/x-h264` | H.264 | 替代 MIME 类型 |
| `video/hevc` | H.265/HEVC | 高效率，较新编解码器 |
| `video/x-h265` | H.265/HEVC | 替代 MIME 类型 |
| `video/x-ms-wmv` | WMV | Windows Media Video |

### 错误处理

```cpp
auto encoder = std::make_shared<MediaFoundationEncoder>();

if (!encoder->Create("video/avc", 1920, 1080, 5000000, 30)) {
    LogError("创建编码器失败 - 检查编解码器支持和参数");
    return false;
}

if (!encoder->Start()) {
    LogError("启动编码器失败");
    encoder->Release();
    return false;
}

// 带错误检查的编码
if (!encoder->EncodeFrame(bitmap, timestamp, false)) {
    if (encoder->HasEncodedData()) {
        // 需要先排空输出
        encoder->GetEncodedData(&buffer, &size, &pts, &isKey);
    } else {
        LogError("编码帧失败");
    }
}
```

### 资源管理

始终正确释放资源：

```cpp
// 选项 1：显式清理
encoder->Stop();
encoder->Release();

// 选项 2：通过析构函数自动清理
{
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    // ... 使用编码器 ...
} // 超出作用域时自动释放
```

### 性能提示

1. **使用 NV12 格式**：保持 bitmap 为 NV12 格式以避免转换开销
2. **批量编码**：在排空输出之前编码多帧
3. **检查输出可用性**：使用 `HasEncodedData()` 避免不必要的调用
4. **重用 Bitmap**：创建一个 bitmap 并在多帧中重用
5. **适当的码率**：1080p 使用 5-10 Mbps，720p 使用 1-3 Mbps

```cpp
// 高效编码循环
auto bitmap = std::make_shared<Bitmap>();
BitmapFormat format(PixelFormat::NV12, 1920, 1080);
bitmap->Create(format);

for (auto& frame : videoFrames) {
    // 重用同一个 bitmap，仅更新数据
    UpdateBitmapData(bitmap, frame);
    
    encoder->EncodeFrame(bitmap, frame.timestamp, frame.isKey);
    
    // 定期排空输出，而不是每帧都排空
    if (frameCount % 10 == 0) {
        while (encoder->HasEncodedData()) {
            encoder->GetEncodedData(&buffer, &size, &pts, &isKey);
            WriteOutput(buffer, size);
        }
    }
}
```

### 故障排除

**问题**：编码器创建失败
- **解决方案**：确保你的 Windows 版本支持该编解码器。H.264 普遍支持。

**问题**：没有编码输出
- **解决方案**：检查你是否定期调用 `GetEncodedData()` 来排空编码器输出缓冲区。

**问题**：视频质量差
- **解决方案**：增加码率或调整质量设置。对于 1080p，至少使用 5 Mbps。

**问题**：输出文件很大
- **解决方案**：降低码率，增加关键帧间隔，或使用更高效的编解码器如 H.265。

**问题**：编码器不接受帧
- **解决方案**：输出缓冲区已满。在编码更多帧之前调用 `GetEncodedData()` 排空输出。

## API 参考

### 方法

- `Create(mimeType, width, height, bitrate, frameRate, mode)` - 初始化编码器
- `Start()` - 启动编码器
- `Stop()` - 停止编码器
- `Release()` - 释放所有资源
- `EncodeFrame(bitmap, timestamp, forceKeyFrame)` - 从 Bitmap 编码帧
- `SignalEndOfStream()` - 发送无更多输入信号
- `GetEncodedData(buffer, size, timestamp, isKeyFrame)` - 获取编码输出
- `GetConfigData(buffer, size)` - 获取编解码器配置（SPS/PPS）
- `HasEncodedData()` - 检查编码数据是否可用
- `Flush()` - 清除编码器缓冲区
- `SetInputFormat(format)` - 设置输入像素格式
- `SetKeyframeInterval(seconds)` - 设置关键帧间隔

### 属性

- `GetWidth()` - 获取配置的宽度
- `GetHeight()` - 获取配置的高度
- `GetBitrate()` - 获取配置的码率
- `GetFrameRate()` - 获取配置的帧率
- `GetMode()` - 获取编码模式
- `IsRunning()` - 检查编码器是否运行

## 与 ULUI 组件集成

### 与 Bitmap 和纹理

```cpp
auto encoder = std::make_shared<MediaFoundationEncoder>();
auto bitmap = std::make_shared<Bitmap>();
auto texture = std::make_shared<gl::Texture2D>();

// 从纹理下载到 bitmap
bitmap->DownloadFromTexture(texture.get());

// 编码帧
encoder->EncodeFrame(bitmap, timestamp, false);
```

### 与文件系统

```cpp
// 保存编码视频
std::string path = FileSystem::GetExternalDataPath() + "video.h264";
FILE* file = fopen(path.c_str(), "wb");

uint8_t* buffer;
size_t size;
while (encoder->HasEncodedData()) {
    if (encoder->GetEncodedData(&buffer, &size, nullptr, nullptr)) {
        fwrite(buffer, 1, size, file);
    }
}
fclose(file);
```

### 屏幕录制示例

```cpp
bool RecordScreen(int durationSeconds) {
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    encoder->Create("video/avc", 1920, 1080, 8000000, 60);
    encoder->Start();
    
    auto bitmap = std::make_shared<Bitmap>();
    
    FILE* output = fopen("screen_recording.h264", "wb");
    
    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;
    
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
        
        if (elapsed.count() >= durationSeconds) break;
        
        // 捕获屏幕到 bitmap
        CaptureScreenToBitmap(bitmap);
        
        // 编码帧
        int64_t timestamp = frameCount * (1000000 / 60);
        encoder->EncodeFrame(bitmap, timestamp, false);
        
        // 写入编码数据
        uint8_t* buffer;
        size_t size;
        while (encoder->HasEncodedData()) {
            if (encoder->GetEncodedData(&buffer, &size, nullptr, nullptr)) {
                fwrite(buffer, 1, size, output);
            }
        }
        
        frameCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 fps
    }
    
    encoder->SignalEndOfStream();
    
    // 排空剩余帧
    uint8_t* buffer;
    size_t size;
    while (encoder->HasEncodedData()) {
        if (encoder->GetEncodedData(&buffer, &size, nullptr, nullptr)) {
            fwrite(buffer, 1, size, output);
        }
    }
    
    fclose(output);
    encoder->Stop();
    
    LogInfo("录制 %d 帧", frameCount);
    return true;
}
```

## 另见

- [Windows Media Foundation 解码器文档](MEDIA_FOUNDATION_DECODER_CN.md)
- [Bitmap 文档](BITMAP_CN.md)
- [Android MediaCodec 集成](MEDIACODEC_CN.md)
- [OpenGL 类文档](OPENGL_CLASSES_CN.md)
