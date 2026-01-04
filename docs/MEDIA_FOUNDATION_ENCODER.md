# Windows Media Foundation Encoder Integration Guide

This document describes the Windows Media Foundation encoder wrapper class in the ULUI framework for hardware video encoding on Windows.

## Overview

Media Foundation is Microsoft's modern media API that provides:
- **Hardware acceleration**: Uses device-specific encoding hardware
- **High performance**: Suitable for real-time video processing
- **Wide codec support**: H.264, H.265, WMV, and more

ULUI provides the `MediaFoundationEncoder` class for video encoding on Windows.

## MediaFoundationEncoder

### Creating and Configuring

```cpp
#include "windows/MediaFoundationEncoder.h"

// Create H.264 encoder (BUFFER mode)
auto encoder = std::make_shared<MediaFoundationEncoder>();
bool success = encoder->Create(
    "video/avc",                           // MIME type: H.264
    1920, 1080,                            // Resolution
    5000000,                               // Bitrate: 5 Mbps
    30,                                    // Frame rate: 30 fps
    MediaFoundationEncoder::Mode::BUFFER   // Mode: CPU buffer
);

// Start the encoder
encoder->Start();
```

### Encoding Modes

#### 1. BUFFER Mode (Recommended)

Encodes video frames from CPU-accessible Bitmap objects.

```cpp
auto encoder = std::make_shared<MediaFoundationEncoder>();
encoder->Create("video/avc", 1920, 1080, 5000000, 30, 
                MediaFoundationEncoder::Mode::BUFFER);
encoder->Start();

// Create bitmap with video frame data
auto bitmap = std::make_shared<Bitmap>();
// ... fill bitmap with frame data (e.g., from camera, screen capture, etc.)

// Ensure bitmap is in NV12 format (or encoder will convert automatically)
if (bitmap->GetPixelFormat() != PixelFormat::NV12) {
    bitmap->ConvertTo(PixelFormat::NV12);
}

// Encode the frame
int64_t timestampUs = GetCurrentTimestampMicroseconds();
bool forceKeyFrame = (frameCount % (30 * keyframeInterval) == 0);
encoder->EncodeFrame(bitmap, timestampUs, forceKeyFrame);

// Get encoded output
uint8_t* encodedData;
size_t encodedSize;
int64_t pts;
bool isKeyFrame;

while (encoder->HasEncodedData()) {
    if (encoder->GetEncodedData(&encodedData, &encodedSize, &pts, &isKeyFrame)) {
        // Write encoded data to file or stream
        WriteToFile(encodedData, encodedSize);
        
        // Check if it's a keyframe
        if (isKeyFrame) {
            LogInfo("Encoded keyframe at timestamp %lld", pts);
        }
    }
}
```

#### 2. TEXTURE Mode (Future)

Direct GPU texture input (not yet implemented).

### Complete Example: Video Capture and Encoding

```cpp
#include "windows/MediaFoundationEncoder.h"
#include "Bitmap.h"
#include "file_system.h"

bool CaptureAndEncodeVideo(int durationSeconds) {
    // Create encoder
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    if (!encoder->Create("video/avc", 1920, 1080, 5000000, 30, 
                         MediaFoundationEncoder::Mode::BUFFER)) {
        LogError("Failed to create encoder");
        return false;
    }
    
    // Set keyframe interval (every 2 seconds)
    encoder->SetKeyframeInterval(2);
    
    // Start encoder
    if (!encoder->Start()) {
        LogError("Failed to start encoder");
        return false;
    }
    
    // Open output file
    std::string outputPath = FileSystem::GetExternalDataPath() + "output.h264";
    FILE* outputFile = fopen(outputPath.c_str(), "wb");
    if (!outputFile) {
        LogError("Failed to open output file");
        return false;
    }
    
    // Get codec config data (SPS/PPS for H.264)
    uint8_t* configData;
    size_t configSize;
    
    // Create bitmap for frames
    auto bitmap = std::make_shared<Bitmap>();
    BitmapFormat format(PixelFormat::NV12, 1920, 1080);
    bitmap->Create(format);
    
    int frameCount = 0;
    int totalFrames = durationSeconds * 30; // 30 fps
    int64_t frameIntervalUs = 1000000 / 30; // Microseconds per frame
    
    for (int i = 0; i < totalFrames; i++) {
        // Capture frame (from camera, screen, etc.)
        CaptureFrame(bitmap);
        
        // Calculate timestamp
        int64_t timestamp = i * frameIntervalUs;
        
        // Force keyframe every 2 seconds
        bool forceKeyFrame = (i % (30 * 2) == 0);
        
        // Encode frame
        if (!encoder->EncodeFrame(bitmap, timestamp, forceKeyFrame)) {
            LogWarning("Failed to encode frame %d, need to drain output", i);
        }
        
        // Get encoded output
        uint8_t* encodedData;
        size_t encodedSize;
        int64_t pts;
        bool isKeyFrame;
        
        while (encoder->HasEncodedData()) {
            if (encoder->GetEncodedData(&encodedData, &encodedSize, &pts, &isKeyFrame)) {
                // Write to file
                fwrite(encodedData, 1, encodedSize, outputFile);
                frameCount++;
                
                if (isKeyFrame) {
                    LogInfo("Encoded keyframe %d/%d", frameCount, totalFrames);
                }
            }
        }
        
        // Get config data from first keyframe
        if (i == 0 && encoder->GetConfigData(&configData, &configSize)) {
            LogInfo("Got codec config data: %zu bytes", configSize);
            // You may want to save SPS/PPS separately for MP4 muxing
        }
    }
    
    // Signal end of stream
    encoder->SignalEndOfStream();
    
    // Drain remaining encoded frames
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
    
    // Cleanup
    fclose(outputFile);
    encoder->Stop();
    encoder->Release();
    
    LogInfo("Encoded %d frames to %s", frameCount, outputPath.c_str());
    return true;
}
```

### Supported Video Codecs

The encoder supports various video codecs through MIME types:

| MIME Type | Codec | Description |
|-----------|-------|-------------|
| `video/avc` | H.264 | Most common, widely supported |
| `video/x-h264` | H.264 | Alternative MIME type |
| `video/hevc` | H.265/HEVC | High efficiency, newer codec |
| `video/x-h265` | H.265/HEVC | Alternative MIME type |
| `video/x-ms-wmv` | WMV | Windows Media Video |

### Error Handling

```cpp
auto encoder = std::make_shared<MediaFoundationEncoder>();

if (!encoder->Create("video/avc", 1920, 1080, 5000000, 30)) {
    LogError("Failed to create encoder - check codec support and parameters");
    return false;
}

if (!encoder->Start()) {
    LogError("Failed to start encoder");
    encoder->Release();
    return false;
}

// Encode with error checking
if (!encoder->EncodeFrame(bitmap, timestamp, false)) {
    if (encoder->HasEncodedData()) {
        // Need to drain output first
        encoder->GetEncodedData(&buffer, &size, &pts, &isKey);
    } else {
        LogError("Failed to encode frame");
    }
}
```

### Resource Management

Always properly release resources:

```cpp
// Option 1: Explicit cleanup
encoder->Stop();
encoder->Release();

// Option 2: Automatic cleanup via destructor
{
    auto encoder = std::make_shared<MediaFoundationEncoder>();
    // ... use encoder ...
} // Automatically released when going out of scope
```

### Performance Tips

1. **Use NV12 Format**: Keep bitmaps in NV12 format to avoid conversion overhead
2. **Batch Encoding**: Encode multiple frames before draining output
3. **Check Output Availability**: Use `HasEncodedData()` to avoid unnecessary calls
4. **Reuse Bitmaps**: Create one bitmap and reuse it for multiple frames
5. **Appropriate Bitrate**: Use 5-10 Mbps for 1080p, 1-3 Mbps for 720p

```cpp
// Efficient encoding loop
auto bitmap = std::make_shared<Bitmap>();
BitmapFormat format(PixelFormat::NV12, 1920, 1080);
bitmap->Create(format);

for (auto& frame : videoFrames) {
    // Reuse same bitmap, just update data
    UpdateBitmapData(bitmap, frame);
    
    encoder->EncodeFrame(bitmap, frame.timestamp, frame.isKey);
    
    // Drain output periodically, not every frame
    if (frameCount % 10 == 0) {
        while (encoder->HasEncodedData()) {
            encoder->GetEncodedData(&buffer, &size, &pts, &isKey);
            WriteOutput(buffer, size);
        }
    }
}
```

### Troubleshooting

**Problem**: Encoder fails to create
- **Solution**: Ensure the codec is supported on your Windows version. H.264 is universally supported.

**Problem**: No encoded output
- **Solution**: Check that you're calling `GetEncodedData()` regularly to drain the encoder output buffer.

**Problem**: Poor video quality
- **Solution**: Increase bitrate or adjust quality settings. For 1080p, use at least 5 Mbps.

**Problem**: Large output file size
- **Solution**: Reduce bitrate, increase keyframe interval, or use a more efficient codec like H.265.

**Problem**: Encoder not accepting frames
- **Solution**: The output buffer is full. Call `GetEncodedData()` to drain output before encoding more frames.

## API Reference

### Methods

- `Create(mimeType, width, height, bitrate, frameRate, mode)` - Initialize encoder
- `Start()` - Start the encoder
- `Stop()` - Stop the encoder
- `Release()` - Release all resources
- `EncodeFrame(bitmap, timestamp, forceKeyFrame)` - Encode a frame from Bitmap
- `SignalEndOfStream()` - Signal no more input
- `GetEncodedData(buffer, size, timestamp, isKeyFrame)` - Get encoded output
- `GetConfigData(buffer, size)` - Get codec configuration (SPS/PPS)
- `HasEncodedData()` - Check if encoded data is available
- `Flush()` - Clear encoder buffers
- `SetInputFormat(format)` - Set input pixel format
- `SetKeyframeInterval(seconds)` - Set keyframe interval

### Properties

- `GetWidth()` - Get configured width
- `GetHeight()` - Get configured height
- `GetBitrate()` - Get configured bitrate
- `GetFrameRate()` - Get configured frame rate
- `GetMode()` - Get encoding mode
- `IsRunning()` - Check if encoder is running

## Integration with ULUI Components

### With Bitmap and Textures

```cpp
auto encoder = std::make_shared<MediaFoundationEncoder>();
auto bitmap = std::make_shared<Bitmap>();
auto texture = std::make_shared<gl::Texture2D>();

// Download texture to bitmap
bitmap->DownloadFromTexture(texture.get());

// Encode frame
encoder->EncodeFrame(bitmap, timestamp, false);
```

### With File System

```cpp
// Save encoded video
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

### Screen Recording Example

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
        
        // Capture screen to bitmap
        CaptureScreenToBitmap(bitmap);
        
        // Encode frame
        int64_t timestamp = frameCount * (1000000 / 60);
        encoder->EncodeFrame(bitmap, timestamp, false);
        
        // Write encoded data
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
    
    // Drain remaining frames
    uint8_t* buffer;
    size_t size;
    while (encoder->HasEncodedData()) {
        if (encoder->GetEncodedData(&buffer, &size, nullptr, nullptr)) {
            fwrite(buffer, 1, size, output);
        }
    }
    
    fclose(output);
    encoder->Stop();
    
    LogInfo("Recorded %d frames", frameCount);
    return true;
}
```

## See Also

- [Windows Media Foundation Decoder Documentation](MEDIA_FOUNDATION_DECODER.md)
- [Bitmap Documentation](BITMAP_CN.md)
- [Android MediaCodec Integration](MEDIACODEC_CN.md)
- [OpenGL Classes Documentation](OPENGL_CLASSES.md)
