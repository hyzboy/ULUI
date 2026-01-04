# Windows Media Foundation Camera Integration Guide

This document describes the Windows Media Foundation camera capture wrapper class in the ULUI framework.

## Overview

Media Foundation provides comprehensive camera access on Windows with:
- **Hardware acceleration**: Direct access to camera hardware
- **High performance**: Efficient frame capture with minimal overhead
- **Wide device support**: Works with built-in webcams, USB cameras, and capture devices

ULUI provides the `MediaFoundationCamera` class for camera capture on Windows.

## MediaFoundationCamera

### Initialization and Camera Enumeration

```cpp
#include "windows/MediaFoundationCamera.h"

// Create and initialize camera manager
auto camera = std::make_shared<MediaFoundationCamera>();
if (!camera->Initialize()) {
    LogError("Failed to initialize camera manager");
    return false;
}

// Enumerate available cameras
if (!camera->EnumerateCameras()) {
    LogError("No cameras found");
    return false;
}

// List all cameras
const auto& cameras = camera->GetCameras();
for (const auto& camInfo : cameras) {
    LogInfo("Camera: %s", camInfo.name.c_str());
    LogInfo("  ID: %s", camInfo.id.c_str());
    LogInfo("  Facing: %d", (int)camInfo.facing);
    LogInfo("  Formats: %zu", camInfo.formats.size());
    
    for (const auto& fmt : camInfo.formats) {
        LogInfo("    %dx%d @ %.1f fps", fmt.width, fmt.height, fmt.GetFPS());
    }
}
```

### Opening Camera and Starting Capture

```cpp
// Open the first camera
if (!cameras.empty()) {
    const std::string& cameraId = cameras[0].id;
    
    if (!camera->OpenCamera(cameraId)) {
        LogError("Failed to open camera");
        return false;
    }
    
    // Start capture at 1920x1080, 30 fps
    if (!camera->StartCapture(1920, 1080, 30.0f)) {
        LogError("Failed to start capture");
        return false;
    }
    
    LogInfo("Camera capture started");
}
```

### Capturing Frames

#### Method 1: Polling for Frames

```cpp
auto bitmap = std::make_shared<Bitmap>();

while (capturing) {
    int64_t timestamp;
    if (camera->GetFrame(bitmap, &timestamp)) {
        // New frame available
        LogInfo("Got frame at timestamp %lld", timestamp);
        
        // Process the frame
        // - Upload to OpenGL texture
        // - Encode to video
        // - Save to file
        // - Apply effects, etc.
        
        texture->UploadFromBitmap(bitmap);
    }
    
    // Small delay to avoid busy-wait
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

#### Method 2: Using Frame Callback

```cpp
// Set frame callback
camera->SetFrameCallback([](std::shared_ptr<Bitmap> frame, int64_t timestampUs) {
    LogInfo("New frame: %dx%d at %lld", 
            frame->GetWidth(), frame->GetHeight(), timestampUs);
    
    // Process frame immediately
    ProcessFrame(frame);
});

// Start capture
camera->StartCapture(1920, 1080, 30.0f);

// Frames will be delivered via callback
// No need to poll
```

### Complete Example: Video Recording from Camera

```cpp
#include "windows/MediaFoundationCamera.h"
#include "windows/MediaFoundationEncoder.h"
#include "Bitmap.h"

bool RecordFromCamera(int durationSeconds) {
    // Initialize camera
    auto camera = std::make_shared<MediaFoundationCamera>();
    if (!camera->Initialize()) {
        return false;
    }
    
    // Find a camera
    camera->EnumerateCameras();
    const auto& cameras = camera->GetCameras();
    if (cameras.empty()) {
        LogError("No cameras found");
        return false;
    }
    
    // Open first camera
    if (!camera->OpenCamera(cameras[0].id)) {
        return false;
    }
    
    // Start capture
    if (!camera->StartCapture(1920, 1080, 30.0f)) {
        return false;
    }
    
    // Create encoder
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    if (!encoder->Create("video/avc", 1920, 1080, 5000000, 30)) {
        return false;
    }
    encoder->Start();
    
    // Open output file
    FILE* outputFile = fopen("camera_recording.h264", "wb");
    if (!outputFile) {
        return false;
    }
    
    // Record frames
    auto bitmap = std::make_shared<Bitmap>();
    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;
    
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
        
        if (elapsed.count() >= durationSeconds) break;
        
        // Get camera frame
        int64_t timestamp;
        if (camera->GetFrame(bitmap, &timestamp)) {
            // Encode frame
            bool forceKeyFrame = (frameCount % 60 == 0);
            encoder->EncodeFrame(bitmap, timestamp, forceKeyFrame);
            frameCount++;
            
            // Write encoded data
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
    
    // Signal end and drain encoder
    encoder->SignalEndOfStream();
    
    uint8_t* encodedData;
    size_t encodedSize;
    while (encoder->HasEncodedData()) {
        if (encoder->GetEncodedData(&encodedData, &encodedSize, nullptr, nullptr)) {
            fwrite(encodedData, 1, encodedSize, outputFile);
        }
    }
    
    // Cleanup
    fclose(outputFile);
    camera->StopCapture();
    camera->CloseCamera();
    encoder->Stop();
    
    LogInfo("Recorded %d frames", frameCount);
    return true;
}
```

### Camera Parameter Control

```cpp
// Set camera parameters
CameraParams params;
params.autoExposure = true;
params.autoFocus = true;
params.autoWhiteBalance = true;
params.zoomRatio = 1.5;  // 1.5x zoom

if (camera->SetCameraParams(params)) {
    LogInfo("Camera parameters set successfully");
}

// Get current parameters
CameraParams current = camera->GetCameraParams();
LogInfo("Current zoom: %.1f", current.zoomRatio);
```

### Error Handling

```cpp
auto camera = std::make_shared<MediaFoundationCamera>();

if (!camera->Initialize()) {
    LogError("Failed to initialize camera - check permissions");
    return false;
}

if (!camera->EnumerateCameras()) {
    LogError("Failed to enumerate cameras - no devices found");
    return false;
}

const std::string& cameraId = camera->GetCameras()[0].id;

if (!camera->OpenCamera(cameraId)) {
    LogError("Failed to open camera - device may be in use");
    camera->Cleanup();
    return false;
}

if (!camera->StartCapture(1920, 1080, 30.0f)) {
    LogError("Failed to start capture - format may not be supported");
    camera->CloseCamera();
    return false;
}
```

### Resource Management

```cpp
// Option 1: Explicit cleanup
camera->StopCapture();
camera->CloseCamera();
camera->Cleanup();

// Option 2: Automatic cleanup via destructor
{
    auto camera = std::make_shared<MediaFoundationCamera>();
    // ... use camera ...
} // Automatically cleaned up when going out of scope
```

### Performance Tips

1. **Choose appropriate resolution**: Higher resolutions require more processing
2. **Use frame callbacks**: More efficient than polling
3. **Reuse bitmaps**: Create one bitmap and reuse it for GetFrame()
4. **Match encoder format**: Use same resolution for camera and encoder
5. **Hardware acceleration**: Ensure GPU drivers are up to date

```cpp
// Efficient capture loop
auto bitmap = std::make_shared<Bitmap>();

// Pre-create bitmap with expected format
BitmapFormat format(PixelFormat::BGRA8, 1920, 1080);
bitmap->Create(format);

while (capturing) {
    // Reuse same bitmap
    if (camera->GetFrame(bitmap, nullptr)) {
        ProcessFrame(bitmap);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

### Troubleshooting

**Problem**: No cameras found
- **Solution**: Check that camera is connected and drivers are installed. Verify camera works in other applications.

**Problem**: Failed to open camera
- **Solution**: Camera may be in use by another application. Close other apps using the camera.

**Problem**: No frames received
- **Solution**: Check that capture was started successfully. Verify resolution is supported by camera.

**Problem**: Poor frame rate
- **Solution**: Reduce resolution or frame rate. Check CPU usage. Ensure hardware acceleration is enabled.

**Problem**: Frames are corrupted or black
- **Solution**: Verify camera is receiving adequate lighting. Check camera format settings.

## API Reference

### Methods

- `Initialize()` - Initialize camera manager
- `Cleanup()` - Release all resources
- `EnumerateCameras()` - Enumerate available cameras
- `GetCameras()` - Get list of camera information
- `GetCameraInfo(id)` - Get specific camera information
- `OpenCamera(id)` - Open a camera
- `CloseCamera()` - Close current camera
- `IsOpen()` - Check if camera is open
- `StartCapture(width, height, frameRate)` - Start capturing
- `StopCapture()` - Stop capturing
- `IsCapturing()` - Check if capturing
- `GetFrame(bitmap, timestamp)` - Get latest captured frame
- `SetFrameCallback(callback)` - Set frame callback
- `SetCameraParams(params)` - Set camera control parameters
- `GetCameraParams()` - Get current parameters
- `SetCameraFormat(width, height, frameRate)` - Set capture format

### Properties

- `GetWidth()` - Get capture width
- `GetHeight()` - Get capture height
- `GetFrameRate()` - Get capture frame rate
- `GetCurrentCameraInfo()` - Get info for current camera

## Integration with ULUI Components

### With Encoder

```cpp
// Capture and encode
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
        // ... drain encoder ...
    }
}
```

### With OpenGL Textures

```cpp
// Display camera preview
auto camera = std::make_shared<MediaFoundationCamera>();
auto bitmap = std::make_shared<Bitmap>();
auto texture = std::make_shared<gl::Texture2D>();

camera->StartCapture(1280, 720, 30);

// Render loop
while (rendering) {
    if (camera->GetFrame(bitmap, nullptr)) {
        // Upload to texture
        bitmap->UploadToTexture(texture.get());
        
        // Render texture
        texture->Bind(0);
        DrawFullscreenQuad();
    }
}
```

### Live Preview Example

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
    
    // Set frame callback for automatic updates
    camera->SetFrameCallback([&](std::shared_ptr<Bitmap> frame, int64_t ts) {
        // Upload to texture on callback
        frame->UploadToTexture(texture.get());
    });
    
    // Render loop
    while (!ShouldClose()) {
        // Texture is automatically updated by callback
        RenderTexture(texture);
        SwapBuffers();
    }
    
    camera->StopCapture();
    camera->CloseCamera();
    return true;
}
```

## See Also

- [Windows Media Foundation Encoder Documentation](MEDIA_FOUNDATION_ENCODER.md)
- [Windows Media Foundation Decoder Documentation](MEDIA_FOUNDATION_DECODER.md)
- [Android Camera2 Integration](CAMERA2_CN.md)
- [Bitmap Documentation](BITMAP_CN.md)
