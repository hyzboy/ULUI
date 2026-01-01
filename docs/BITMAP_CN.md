# Bitmap 类文档

Bitmap 类是 ULUI 中用于图像和视频帧数据交换的核心类。它不是用于绘图，而是作为各种图像/视频数据源（摄像头、解码器、文件）与 OpenGL 纹理之间的桥梁。

## 设计理念

1. **零拷贝优先**：支持包装外部数据，避免不必要的内存复制
2. **多格式支持**：RGB、YUV、平台特定格式
3. **统一接口**：跨平台的上层API
4. **双向转换**：支持上传到GPU和从GPU下载

## 核心功能

### 1. 数据管理模式

- **内部数据模式** (`BitmapOwnership::INTERNAL`)：Bitmap 拥有并管理内存
- **外部数据模式** (`BitmapOwnership::EXTERNAL`)：Bitmap 只包装外部数据指针

### 2. 支持的像素格式

#### RGB 格式（8位/通道）
- `RGB8` - 24位 RGB
- `RGBA8` - 32位 RGBA
- `BGR8` - 24位 BGR
- `BGRA8` - 32位 BGRA

#### RGB 格式（16位/通道）
- `RGB16` - 48位 RGB
- `RGBA16` - 64位 RGBA

#### 灰度格式
- `GRAY8` - 8位灰度
- `GRAY16` - 16位灰度

#### YUV 平面格式（视频常用）
- `YUV420P` - YUV 4:2:0 平面 (I420)
- `YUV422P` - YUV 4:2:2 平面
- `YUV444P` - YUV 4:4:4 平面

#### YUV 打包/半平面格式
- `NV12` - YUV 4:2:0 半平面（Y平面 + 交错UV）
- `NV21` - YUV 4:2:0 半平面（Y平面 + 交错VU）- Android常用
- `YUYV` - YUV 4:2:2 打包
- `UYVY` - YUV 4:2:2 打包

#### 平台特定格式
- `ANDROID_HARDWARE_BUFFER` - Android硬件缓冲
- `IOS_CVPIXELBUFFER` - iOS CVPixelBuffer
- `MEDIACODEC_BUFFER` - Android MediaCodec缓冲

## 使用示例

### 1. 创建内部数据Bitmap

```cpp
#include "Bitmap.h"
using namespace ului;

// 创建RGBA格式的Bitmap
BitmapFormat format(PixelFormat::RGBA8, 512, 512);
Bitmap bitmap;
if (bitmap.Create(format)) {
    // Bitmap创建成功，内存已分配
    void* data = bitmap.GetData();
    // ... 填充像素数据 ...
}
```

### 2. 包装外部数据（零拷贝）

```cpp
// 假设有一个来自摄像头的缓冲区
void* cameraBuffer = GetCameraBuffer();
int width = 1920, height = 1080;

BitmapFormat format(PixelFormat::NV21, width, height);
Bitmap bitmap;
bitmap.WrapExternalData(cameraBuffer, format);

// 现在bitmap包装了cameraBuffer，没有内存复制
```

### 3. 包装平面YUV数据

```cpp
// YUV420P有三个平面
void* yPlane = GetYPlane();
void* uPlane = GetUPlane();
void* vPlane = GetVPlane();

void* planes[3] = { yPlane, uPlane, vPlane };
BitmapFormat format(PixelFormat::YUV420P, 1920, 1080);

Bitmap bitmap;
bitmap.WrapExternalDataPlanes(planes, 3, format);
```

### 4. 上传到OpenGL纹理

```cpp
#include "gl/Texture2D.h"

// 创建纹理
auto texture = std::make_shared<gl::Texture2D>();
texture->Create();

// 上传Bitmap数据到纹理
bitmap.UploadToTexture(texture.get());

// 现在可以使用纹理进行渲染
texture->Bind(0);
```

### 5. 从文件加载（未来实现）

```cpp
Bitmap bitmap;
if (bitmap.LoadFromFile("image.png")) {
    // 图片加载成功
}
```

## 视频处理管线示例

### 完整的摄像头到编码器管线

```cpp
#include "Bitmap.h"
#include "gl/Texture2D.h"
#include "gl/RenderTarget.h"

// 1. 从摄像头获取帧
void* cameraBuffer = camera->GetNextFrameBuffer();
BitmapFormat cameraFormat(PixelFormat::NV21, 1920, 1080);

Bitmap inputFrame;
inputFrame.WrapExternalData(cameraBuffer, cameraFormat);

// 2. 上传到纹理
auto texture = std::make_shared<gl::Texture2D>();
texture->Create();
inputFrame.UploadToTexture(texture.get());

// 3. OpenGL渲染
auto renderTarget = std::make_shared<gl::RenderTarget>(1920, 1080);
renderTarget->Initialize();
renderTarget->Bind();

// ... 使用texture进行OpenGL渲染 ...

renderTarget->Unbind();

// 4. 从渲染目标下载（未来实现）
Bitmap outputFrame;
outputFrame.DownloadFromTexture(texture.get());

// 5. 发送到编码器
encoder->EncodeFrame(outputFrame.GetData());
```

## BitmapFormat 结构

```cpp
struct BitmapFormat {
    PixelFormat pixelFormat;  // 像素格式
    int width;                // 宽度
    int height;               // 高度
    int stride;               // 每行字节数（0=自动计算）
    ColorSpace colorSpace;    // 色彩空间
    bool premultipliedAlpha;  // Alpha是否预乘
    
    // 辅助方法
    int GetBytesPerPixel() const;     // 获取每像素字节数
    size_t GetDataSize() const;       // 获取总数据大小
    bool IsYUV() const;               // 是否YUV格式
    bool IsRGB() const;               // 是否RGB格式
    bool IsPlanar() const;            // 是否平面格式
    bool IsCompressed() const;        // 是否压缩格式
};
```

## 平台特定功能

### Android

```cpp
#ifdef ANDROID
// 包装Android Hardware Buffer
AHardwareBuffer* hwBuffer = GetHardwareBuffer();
bitmap.WrapHardwareBuffer(hwBuffer);
#endif
```

### iOS

```cpp
#ifdef __APPLE__
// 包装CVPixelBuffer
CVPixelBufferRef pixelBuffer = GetCVPixelBuffer();
bitmap.WrapCVPixelBuffer(pixelBuffer);
#endif
```

### FFmpeg

```cpp
// 包装FFmpeg AVFrame（需要FFmpeg头文件）
AVFrame* frame = GetDecodedFrame();
bitmap.WrapFFmpegFrame(frame);
```

## 注意事项

1. **外部数据生命周期**：使用 `WrapExternalData` 时，确保原始数据在Bitmap使用期间保持有效
2. **格式转换**：某些格式之间的转换尚未实现（如YUV到RGB）
3. **线程安全**：Bitmap本身不是线程安全的，多线程访问需要外部同步
4. **平面格式**：使用 `GetPlaneData(plane)` 访问YUV平面数据
5. **内存对齐**：某些平台对stride有特殊要求，注意设置正确的stride值

## 未来扩展

- [ ] 完整的格式转换支持（RGB ↔ YUV）
- [ ] 文件加载/保存（使用stb_image）
- [ ] 从纹理下载到Bitmap
- [ ] 完整的FFmpeg集成
- [ ] 完整的平台特定缓冲支持
- [ ] 内存池管理避免频繁分配
- [ ] SIMD优化的格式转换

## 参考

- [OpenGL ES 3.0 纹理格式](https://www.khronos.org/registry/OpenGL-Refpages/es3.0/)
- [YUV格式说明](https://wiki.videolan.org/YUV/)
- [FFmpeg AVFrame文档](https://ffmpeg.org/doxygen/trunk/structAVFrame.html)
