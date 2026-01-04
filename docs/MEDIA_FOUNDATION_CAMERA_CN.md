# Windows Media Foundation 摄像头集成文档

本文档介绍 ULUI 框架中的 Windows Media Foundation 摄像头捕获封装类。

## 概述

Media Foundation 在 Windows 上提供全面的摄像头访问：
- **硬件加速**：直接访问摄像头硬件
- **高性能**：高效帧捕获，最小开销
- **广泛设备支持**：支持内置摄像头、USB 摄像头和采集设备

ULUI 提供 `MediaFoundationCamera` 类用于 Windows 平台的摄像头捕获。

## MediaFoundationCamera

### 初始化和摄像头枚举

```cpp
#include "windows/MediaFoundationCamera.h"

// 创建并初始化摄像头管理器
auto camera = std::make_shared<MediaFoundationCamera>();
if (!camera->Initialize()) {
    LogError("初始化摄像头管理器失败");
    return false;
}

// 枚举可用摄像头
if (!camera->EnumerateCameras()) {
    LogError("未找到摄像头");
    return false;
}

// 列出所有摄像头
const auto& cameras = camera->GetCameras();
for (const auto& camInfo : cameras) {
    LogInfo("摄像头：%s", camInfo.name.c_str());
    LogInfo("  ID：%s", camInfo.id.c_str());
    LogInfo("  朝向：%d", (int)camInfo.facing);
    LogInfo("  格式数：%zu", camInfo.formats.size());
    
    for (const auto& fmt : camInfo.formats) {
        LogInfo("    %dx%d @ %.1f fps", fmt.width, fmt.height, fmt.GetFPS());
    }
}
```

### 打开摄像头并开始捕获

```cpp
// 打开第一个摄像头
if (!cameras.empty()) {
    const std::string& cameraId = cameras[0].id;
    
    if (!camera->OpenCamera(cameraId)) {
        LogError("打开摄像头失败");
        return false;
    }
    
    // 以 1920x1080、30 fps 开始捕获
    if (!camera->StartCapture(1920, 1080, 30.0f)) {
        LogError("启动捕获失败");
        return false;
    }
    
    LogInfo("摄像头捕获已启动");
}
```

### 捕获帧

#### 方法 1：轮询帧

```cpp
auto bitmap = std::make_shared<Bitmap>();

while (capturing) {
    int64_t timestamp;
    if (camera->GetFrame(bitmap, &timestamp)) {
        // 有新帧可用
        LogInfo("获得帧，时间戳 %lld", timestamp);
        
        // 处理帧
        // - 上传到 OpenGL 纹理
        // - 编码为视频
        // - 保存到文件
        // - 应用特效等
        
        texture->UploadFromBitmap(bitmap);
    }
    
    // 小延迟避免忙等
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

#### 方法 2：使用帧回调

```cpp
// 设置帧回调
camera->SetFrameCallback([](std::shared_ptr<Bitmap> frame, int64_t timestampUs) {
    LogInfo("新帧：%dx%d，时间戳 %lld", 
            frame->GetWidth(), frame->GetHeight(), timestampUs);
    
    // 立即处理帧
    ProcessFrame(frame);
});

// 启动捕获
camera->StartCapture(1920, 1080, 30.0f);

// 帧将通过回调传递
// 无需轮询
```

### 完整示例：从摄像头录制视频

```cpp
#include "windows/MediaFoundationCamera.h"
#include "windows/MediaFoundationEncoder.h"
#include "Bitmap.h"

bool RecordFromCamera(int durationSeconds) {
    // 初始化摄像头
    auto camera = std::make_shared<MediaFoundationCamera>();
    if (!camera->Initialize()) {
        return false;
    }
    
    // 查找摄像头
    camera->EnumerateCameras();
    const auto& cameras = camera->GetCameras();
    if (cameras.empty()) {
        LogError("未找到摄像头");
        return false;
    }
    
    // 打开第一个摄像头
    if (!camera->OpenCamera(cameras[0].id)) {
        return false;
    }
    
    // 启动捕获
    if (!camera->StartCapture(1920, 1080, 30.0f)) {
        return false;
    }
    
    // 创建编码器
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    if (!encoder->Create("video/avc", 1920, 1080, 5000000, 30)) {
        return false;
    }
    encoder->Start();
    
    // 打开输出文件
    FILE* outputFile = fopen("camera_recording.h264", "wb");
    if (!outputFile) {
        return false;
    }
    
    // 录制帧
    auto bitmap = std::make_shared<Bitmap>();
    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;
    
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
        
        if (elapsed.count() >= durationSeconds) break;
        
        // 获取摄像头帧
        int64_t timestamp;
        if (camera->GetFrame(bitmap, &timestamp)) {
            // 编码帧
            bool forceKeyFrame = (frameCount % 60 == 0);
            encoder->EncodeFrame(bitmap, timestamp, forceKeyFrame);
            frameCount++;
            
            // 写入编码数据
            uint8_t* encodedData;
            size_t encodedSize;
            int64_t pts;
            bool isKeyFrame;
            
            while (encoder->HasEncodedData()) {
                if (encoder->GetEncodedData(&encodedData, &encodedSize, &pts, &isKeyFrame)) {
                    fwrite(encodedData, 1, encodedSize, outputFile);
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 发送结束信号并排空编码器
    encoder->SignalEndOfStream();
    
    uint8_t* encodedData;
    size_t encodedSize;
    while (encoder->HasEncodedData()) {
        if (encoder->GetEncodedData(&encodedData, &encodedSize, nullptr, nullptr)) {
            fwrite(encodedData, 1, encodedSize, outputFile);
        }
    }
    
    // 清理
    fclose(outputFile);
    camera->StopCapture();
    camera->CloseCamera();
    encoder->Stop();
    
    LogInfo("录制了 %d 帧", frameCount);
    return true;
}
```

### 摄像头参数控制

```cpp
// 设置摄像头参数
CameraParams params;
params.autoExposure = true;
params.autoFocus = true;
params.autoWhiteBalance = true;
params.zoomRatio = 1.5;  // 1.5x 变焦

if (camera->SetCameraParams(params)) {
    LogInfo("摄像头参数设置成功");
}

// 获取当前参数
CameraParams current = camera->GetCameraParams();
LogInfo("当前变焦：%.1f", current.zoomRatio);
```

### 错误处理

```cpp
auto camera = std::make_shared<MediaFoundationCamera>();

if (!camera->Initialize()) {
    LogError("初始化摄像头失败 - 检查权限");
    return false;
}

if (!camera->EnumerateCameras()) {
    LogError("枚举摄像头失败 - 未找到设备");
    return false;
}

const std::string& cameraId = camera->GetCameras()[0].id;

if (!camera->OpenCamera(cameraId)) {
    LogError("打开摄像头失败 - 设备可能正在使用");
    camera->Cleanup();
    return false;
}

if (!camera->StartCapture(1920, 1080, 30.0f)) {
    LogError("启动捕获失败 - 可能不支持该格式");
    camera->CloseCamera();
    return false;
}
```

### 资源管理

```cpp
// 选项 1：显式清理
camera->StopCapture();
camera->CloseCamera();
camera->Cleanup();

// 选项 2：通过析构函数自动清理
{
    auto camera = std::make_shared<MediaFoundationCamera>();
    // ... 使用摄像头 ...
} // 超出作用域时自动清理
```

### 性能提示

1. **选择适当分辨率**：更高分辨率需要更多处理
2. **使用帧回调**：比轮询更高效
3. **重用 Bitmap**：创建一个 bitmap 并在 GetFrame() 中重用
4. **匹配编码器格式**：摄像头和编码器使用相同分辨率
5. **硬件加速**：确保 GPU 驱动程序是最新的

```cpp
// 高效捕获循环
auto bitmap = std::make_shared<Bitmap>();

// 预创建具有预期格式的 bitmap
BitmapFormat format(PixelFormat::BGRA8, 1920, 1080);
bitmap->Create(format);

while (capturing) {
    // 重用同一个 bitmap
    if (camera->GetFrame(bitmap, nullptr)) {
        ProcessFrame(bitmap);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

### 故障排除

**问题**：未找到摄像头
- **解决方案**：检查摄像头是否已连接且驱动程序已安装。验证摄像头在其他应用程序中是否工作。

**问题**：打开摄像头失败
- **解决方案**：摄像头可能被另一个应用程序使用。关闭其他使用摄像头的应用程序。

**问题**：未接收到帧
- **解决方案**：检查捕获是否成功启动。验证摄像头是否支持该分辨率。

**问题**：帧率差
- **解决方案**：降低分辨率或帧率。检查 CPU 使用率。确保启用硬件加速。

**问题**：帧损坏或黑屏
- **解决方案**：验证摄像头是否接收充足光照。检查摄像头格式设置。

## API 参考

### 方法

- `Initialize()` - 初始化摄像头管理器
- `Cleanup()` - 释放所有资源
- `EnumerateCameras()` - 枚举可用摄像头
- `GetCameras()` - 获取摄像头信息列表
- `GetCameraInfo(id)` - 获取特定摄像头信息
- `OpenCamera(id)` - 打开摄像头
- `CloseCamera()` - 关闭当前摄像头
- `IsOpen()` - 检查摄像头是否打开
- `StartCapture(width, height, frameRate)` - 开始捕获
- `StopCapture()` - 停止捕获
- `IsCapturing()` - 检查是否正在捕获
- `GetFrame(bitmap, timestamp)` - 获取最新捕获帧
- `SetFrameCallback(callback)` - 设置帧回调
- `SetCameraParams(params)` - 设置摄像头控制参数
- `GetCameraParams()` - 获取当前参数
- `SetCameraFormat(width, height, frameRate)` - 设置捕获格式

### 属性

- `GetWidth()` - 获取捕获宽度
- `GetHeight()` - 获取捕获高度
- `GetFrameRate()` - 获取捕获帧率
- `GetCurrentCameraInfo()` - 获取当前摄像头信息

## 与 ULUI 组件集成

### 与编码器

```cpp
// 捕获和编码
auto camera = std::make_shared<MediaFoundationCamera>();
auto encoder = std::make_shared<MediaFoundationEncoder>();
auto bitmap = std::make_shared<Bitmap>();

camera->OpenCamera(cameraId);
camera->StartCapture(1920, 1080, 30);

encoder->Create("video/avc", 1920, 1080, 5000000, 30);
encoder->Start();

while (recording) {
    if (camera->GetFrame(bitmap, &timestamp)) {
        encoder->EncodeFrame(bitmap, timestamp, false);
        // ... 排空编码器 ...
    }
}
```

### 与 OpenGL 纹理

```cpp
// 显示摄像头预览
auto camera = std::make_shared<MediaFoundationCamera>();
auto bitmap = std::make_shared<Bitmap>();
auto texture = std::make_shared<gl::Texture2D>();

camera->StartCapture(1280, 720, 30);

// 渲染循环
while (rendering) {
    if (camera->GetFrame(bitmap, nullptr)) {
        // 上传到纹理
        bitmap->UploadToTexture(texture.get());
        
        // 渲染纹理
        texture->Bind(0);
        DrawFullscreenQuad();
    }
}
```

### 实时预览示例

```cpp
bool ShowCameraPreview() {
    auto camera = std::make_shared<MediaFoundationCamera>();
    camera->Initialize();
    camera->EnumerateCameras();
    
    if (camera->GetCameras().empty()) {
        return false;
    }
    
    camera->OpenCamera(camera->GetCameras()[0].id);
    camera->StartCapture(1280, 720, 30);
    
    auto bitmap = std::make_shared<Bitmap>();
    auto texture = std::make_shared<gl::Texture2D>();
    
    // 设置帧回调以自动更新
    camera->SetFrameCallback([&](std::shared_ptr<Bitmap> frame, int64_t ts) {
        // 在回调中上传到纹理
        frame->UploadToTexture(texture.get());
    });
    
    // 渲染循环
    while (!ShouldClose()) {
        // 纹理由回调自动更新
        RenderTexture(texture);
        SwapBuffers();
    }
    
    camera->StopCapture();
    camera->CloseCamera();
    return true;
}
```

## 另见

- [Windows Media Foundation 编码器文档](MEDIA_FOUNDATION_ENCODER_CN.md)
- [Windows Media Foundation 解码器文档](MEDIA_FOUNDATION_DECODER_CN.md)
- [Android Camera2 集成](CAMERA2_CN.md)
- [Bitmap 文档](BITMAP_CN.md)
