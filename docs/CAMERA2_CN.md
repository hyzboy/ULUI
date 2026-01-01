# Android Camera2 集成文档

## 概述

`Camera2Manager` 类提供了Android Camera2 API的完整封装，支持：
- 摄像头枚举和选择
- 分辨率和格式配置
- 摄像头参数调节（曝光、对焦、白平衡等）
- HDR、OIS、EIS等高级功能
- 零拷贝集成OpenGL纹理（OES EXTERNAL）

## 类结构

### Camera2Manager
主管理类，负责摄像头枚举、打开、配置和预览。

### CameraInfo
摄像头信息结构，包含：
- 摄像头ID和朝向（前置/后置/外置）
- 支持的分辨率和格式
- 传感器信息（焦距、光圈等）
- 功能支持（HDR、OIS、AF、AE、AWB等）

### CameraParams
摄像头控制参数：
- 自动/手动曝光控制
- 自动/手动对焦控制
- 自动/手动白平衡控制
- 缩放比例
- HDR开关
- 防抖开关（OIS/EIS）
- 闪光灯控制
- FPS范围设置

## 使用示例

### 1. 枚举摄像头

```cpp
#include "android/Camera2Manager.h"

using namespace ului::android;

// 创建并初始化
auto cameraManager = std::make_shared<Camera2Manager>();
cameraManager->Initialize();

// 枚举所有摄像头
cameraManager->EnumerateCameras();

// 获取摄像头列表
const auto& cameras = cameraManager->GetCameras();

for (const auto& info : cameras) {
    printf("Camera ID: %s\n", info.id.c_str());
    printf("Facing: %s\n", 
           info.facing == CameraFacing::BACK ? "Back" : 
           info.facing == CameraFacing::FRONT ? "Front" : "External");
    printf("Sensor Orientation: %d\n", info.sensorOrientation);
    printf("HDR Support: %s\n", info.supportHDR ? "Yes" : "No");
    printf("OIS Support: %s\n", info.supportOIS ? "Yes" : "No");
    
    // 打印所有支持的分辨率
    printf("Supported Formats:\n");
    for (const auto& fmt : info.formats) {
        printf("  %dx%d @ %.1f FPS\n", 
               fmt.width, fmt.height, fmt.GetFPS());
    }
}
```

### 2. 选择摄像头和分辨率

```cpp
// 选择后置摄像头
std::string backCameraId;
for (const auto& info : cameras) {
    if (info.facing == CameraFacing::BACK) {
        backCameraId = info.id;
        break;
    }
}

// 打开摄像头
if (!cameraManager->OpenCamera(backCameraId)) {
    printf("Failed to open camera\n");
    return;
}

// 选择1920x1080分辨率
const CameraInfo* info = cameraManager->GetCurrentCameraInfo();
int32_t targetWidth = 1920;
int32_t targetHeight = 1080;

for (const auto& fmt : info->formats) {
    if (fmt.width == targetWidth && fmt.height == targetHeight) {
        printf("Found format: %dx%d @ %.1f FPS\n",
               fmt.width, fmt.height, fmt.GetFPS());
        break;
    }
}
```

### 3. 创建OpenGL纹理并启动预览

```cpp
// 创建OES EXTERNAL纹理
auto cameraTexture = cameraManager->CreateCameraTexture();
if (!cameraTexture) {
    printf("Failed to create camera texture\n");
    return;
}

// 启动预览（注意：需要JNI桥接）
if (!cameraManager->StartPreview(cameraTexture, 1920, 1080)) {
    printf("Failed to start preview\n");
    return;
}

// 在OpenGL中使用纹理
cameraTexture->Bind(0);
// ... 渲染摄像头画面 ...
```

### 4. 调节摄像头参数

```cpp
CameraParams params;

// 自动模式（默认）
params.autoExposure = true;
params.autoFocus = true;
params.autoWhiteBalance = true;

// 或手动模式
params.autoExposure = false;
params.exposureTime = 10000000;  // 10ms in nanoseconds
params.sensitivity = 800;  // ISO 800

params.autoFocus = false;
params.focusDistance = 0.5f;  // 0.5 diopters

// 启用HDR
params.hdrEnabled = true;

// 启用光学防抖
params.oisEnabled = true;

// 设置缩放
params.zoomRatio = 2.0f;  // 2x zoom

// 设置FPS范围（用于视频录制）
params.minFPS = 30;
params.maxFPS = 30;

// 应用参数
cameraManager->SetCameraParams(params);
```

### 5. 帧回调

```cpp
class MyCameraFrameCallback : public ICameraFrameCallback {
public:
    void OnFrameAvailable(int64_t timestamp) override {
        // 新帧到达，更新纹理
        // 在这里调用 SurfaceTexture.updateTexImage() (通过JNI)
        printf("New frame at timestamp: %lld\n", timestamp);
    }
};

auto callback = std::make_shared<MyCameraFrameCallback>();
cameraManager->SetFrameCallback(callback.get());
```

### 6. 完整视频管线示例

```cpp
// 摄像头 → OpenGL处理 → MediaCodec编码

// 1. 创建摄像头纹理
auto cameraTexture = cameraManager->CreateCameraTexture();
cameraManager->StartPreview(cameraTexture, 1920, 1080);

// 2. 创建编码器
MediaCodecEncoder encoder;
encoder.Configure("video/avc", 1920, 1080, 5000000, 30);
encoder.Start();
auto encoderTarget = encoder.CreateInputRenderTarget();

// 3. 渲染循环
while (recording) {
    // 等待新帧回调，然后更新纹理
    // updateTexImage(); // 通过JNI调用
    
    // 渲染到编码器（零拷贝）
    encoderTarget->Bind();
    encoderTarget->Clear(true, false, false);
    
    // 绑定摄像头纹理并渲染
    cameraTexture->Bind(0);
    RenderFullscreenQuad();  // 可以应用滤镜/特效
    
    encoderTarget->Unbind();
    encoderTarget->SwapBuffers();
    
    // 获取编码输出
    MediaCodecBuffer outputBuffer;
    if (encoder.DequeueOutputBuffer(outputBuffer, 0)) {
        // 发送到网络或保存到文件
        SendH264Packet(outputBuffer.data, outputBuffer.size);
        encoder.ReleaseOutputBuffer(outputBuffer.index);
    }
}

// 4. 清理
encoder.Stop();
cameraManager->StopPreview();
cameraManager->CloseCamera();
```

## 着色器要求

使用OES EXTERNAL纹理需要特殊的着色器：

```glsl
#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require

precision mediump float;

in vec2 vTexCoord;
out vec4 fragColor;

uniform samplerExternalOES uCameraTexture;  // 注意类型

void main() {
    fragColor = texture(uCameraTexture, vTexCoord);
}
```

## JNI桥接

`Camera2Manager::StartPreview()` 需要JNI支持来创建`ANativeWindow`：

```java
// Java侧代码
public class CameraBridge {
    public static Surface createCameraSurface(int textureId) {
        SurfaceTexture surfaceTexture = new SurfaceTexture(textureId);
        surfaceTexture.setDefaultBufferSize(1920, 1080);
        return new Surface(surfaceTexture);
    }
}
```

```cpp
// C++侧JNI调用
JNIEnv* env = GetJNIEnv();
jclass bridgeClass = env->FindClass("com/example/CameraBridge");
jmethodID createSurfaceMethod = env->GetStaticMethodID(
    bridgeClass, "createCameraSurface", "(I)Landroid/view/Surface;");

jobject surface = env->CallStaticObjectMethod(
    bridgeClass, createSurfaceMethod, texture->GetHandle());

// 获取ANativeWindow
ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
```

## 权限要求

在`AndroidManifest.xml`中添加：

```xml
<uses-permission android:name="android.permission.CAMERA" />
<uses-feature android:name="android.hardware.camera" />
<uses-feature android:name="android.hardware.camera.autofocus" />
```

并在运行时请求权限（Android 6.0+）。

## 性能优化

### 零拷贝管线
- 摄像头 → SurfaceTexture → OES EXTERNAL纹理（GPU直接访问）
- OpenGL渲染 → EGLSurface → MediaCodec（GPU直接编码）
- 全程无CPU内存拷贝

### 最佳实践
1. **选择合适的分辨率**：根据应用场景选择（预览vs录制vs拍照）
2. **使用自动模式**：除非特殊需求，自动AE/AF/AWB效果最好
3. **固定FPS**：视频录制时设置固定FPS范围（如30-30）
4. **启用防抖**：OIS优于EIS，优先使用OIS
5. **HDR谨慎使用**：HDR会增加延迟，不适合实时场景

## 功能支持检测

```cpp
const CameraInfo* info = cameraManager->GetCurrentCameraInfo();

if (info->supportHDR) {
    // 支持HDR
    params.hdrEnabled = true;
}

if (info->supportOIS) {
    // 支持光学防抖
    params.oisEnabled = true;
} else if (info->supportEIS) {
    // 降级到电子防抖
    params.eisEnabled = true;
}

if (!info->supportAF) {
    // 不支持自动对焦，使用固定焦距
    params.autoFocus = false;
    params.focusDistance = 0.0f;  // 无限远
}
```

## 常见问题

### Q: StartPreview返回false？
A: 需要实现JNI桥接来创建SurfaceTexture和ANativeWindow。

### Q: 如何更新纹理内容？
A: 在帧回调中调用`SurfaceTexture.updateTexImage()`（通过JNI）。

### Q: 如何选择最佳分辨率？
A: 根据帧率和应用需求选择。高分辨率需要更多GPU/编码器资源。

### Q: HDR和手动曝光可以同时使用吗？
A: 不可以，HDR是自动场景模式，会覆盖手动设置。

### Q: 如何实现实时美颜/滤镜？
A: 在OpenGL渲染阶段应用着色器效果：
```cpp
cameraTexture->Bind(0);
beautyShader->Use();
beautyShader->SetUniform("smoothLevel", 0.8f);
RenderFullscreenQuad();
```

## 参考链接

- [Android Camera2 API](https://developer.android.com/training/camera2)
- [NDK Camera API](https://developer.android.com/ndk/reference/group/camera)
- [SurfaceTexture](https://developer.android.com/reference/android/graphics/SurfaceTexture)
- [GL_OES_EGL_image_external](https://www.khronos.org/registry/OpenGL/extensions/OES/OES_EGL_image_external.txt)
