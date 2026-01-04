# Windows Media Foundation Decoder Integration Guide

This document describes the Windows Media Foundation decoder wrapper class in the ULUI framework for hardware video decoding on Windows.

## Overview

Media Foundation is Microsoft's modern media API that provides:
- **Hardware acceleration**: Uses device-specific decoding hardware
- **High performance**: Suitable for real-time video processing
- **Wide codec support**: H.264, H.265, VP9, and more

ULUI provides the `MediaFoundationDecoder` class for video decoding on Windows.

## MediaFoundationDecoder

### Creating and Configuring

```cpp
#include "windows/MediaFoundationDecoder.h"

// Create H.264 decoder (BUFFER mode)
auto decoder = std::make_shared<MediaFoundationDecoder>();
bool success = decoder->Create(
    "video/avc",                           // MIME type: H.264
    MediaFoundationDecoder::Mode::BUFFER   // Mode: CPU buffer
);

// Configure decoder dimensions
decoder->Configure(
    1920, 1080,  // Resolution: 1920x1080
    30.0f,       // Frame rate: 30 fps (optional)
    0            // Bitrate: auto (optional)
);

// Start the decoder
decoder->Start();
```

### Decoding Modes

#### 1. BUFFER Mode (Recommended)

Decodes video frames to CPU-accessible Bitmap objects.

```cpp
auto decoder = std::make_shared<MediaFoundationDecoder>();
decoder->Create("video/avc", MediaFoundationDecoder::Mode::BUFFER);
decoder->Configure(1920, 1080);
decoder->Start();

// Create bitmap for receiving decoded frames
auto bitmap = std::make_shared<Bitmap>();

// Queue encoded video data
const uint8_t* h264Data = ...; // Your encoded H.264 data
size_t dataSize = ...;         // Size of encoded data
int64_t timestampUs = ...;     // Presentation timestamp in microseconds
bool isKeyFrame = true;        // Is this a keyframe?

decoder->QueueInputBuffer(h264Data, dataSize, timestampUs, isKeyFrame);

// Get decoded frame
int64_t presentationTime;
if (decoder->GetDecodedBitmap(bitmap, &presentationTime)) {
    // Successfully decoded a frame
    // The bitmap now contains the decoded video frame in NV12 format
    
    // You can convert to other formats if needed
    bitmap->ConvertTo(PixelFormat::RGBA8);
    
    // Or upload to OpenGL texture
    texture->UploadFromBitmap(bitmap);
    
    // Or save to file
    bitmap->SaveToFile("frame.png");
}
```

#### 2. TEXTURE Mode (Future)

Direct GPU texture output (not yet implemented).

### Complete Example: Video File Decoding

```cpp
#include "windows/MediaFoundationDecoder.h"
#include "Bitmap.h"

bool DecodeVideoFile(const char* videoPath) {
    // Create decoder
    auto decoder = std::make_shared<MediaFoundationDecoder>();
    if (!decoder->Create("video/avc", MediaFoundationDecoder::Mode::BUFFER)) {
        LogError("Failed to create decoder");
        return false;
    }
    
    // Configure decoder
    if (!decoder->Configure(1920, 1080, 30.0f)) {
        LogError("Failed to configure decoder");
        return false;
    }
    
    // Start decoder
    if (!decoder->Start()) {
        LogError("Failed to start decoder");
        return false;
    }
    
    // Create bitmap for output
    auto bitmap = std::make_shared<Bitmap>();
    
    // Read and decode video data
    // (You would typically read from a file or network stream)
    std::vector<uint8_t> encodedData = ReadVideoPacket(videoPath);
    int64_t timestamp = 0;
    int frameCount = 0;
    
    while (!encodedData.empty()) {
        // Queue input data
        bool isKeyFrame = (frameCount % 30 == 0); // Assuming keyframe every 30 frames
        if (!decoder->QueueInputBuffer(encodedData.data(), encodedData.size(), 
                                       timestamp, isKeyFrame)) {
            // Decoder might need to output frames first
            // Try to get decoded frames
        }
        
        // Get decoded frames
        int64_t pts;
        while (decoder->GetDecodedBitmap(bitmap, &pts)) {
            LogInfo("Decoded frame %d at timestamp %lld", frameCount, pts);
            
            // Process the decoded frame
            // - Convert format if needed
            // - Upload to texture
            // - Save to file
            // - Apply effects, etc.
            
            frameCount++;
        }
        
        // Read next packet
        encodedData = ReadVideoPacket(videoPath);
        timestamp += 33333; // ~30 fps (33.333ms per frame in microseconds)
    }
    
    // Signal end of stream
    decoder->SignalEndOfStream();
    
    // Drain remaining frames
    int64_t pts;
    while (decoder->GetDecodedBitmap(bitmap, &pts)) {
        LogInfo("Drained frame at timestamp %lld", pts);
        frameCount++;
    }
    
    // Stop and cleanup
    decoder->Stop();
    decoder->Release();
    
    LogInfo("Decoded total of %d frames", frameCount);
    return true;
}
```

### Supported Video Codecs

The decoder supports various video codecs through MIME types:

| MIME Type | Codec | Description |
|-----------|-------|-------------|
| `video/avc` | H.264 | Most common, widely supported |
| `video/x-h264` | H.264 | Alternative MIME type |
| `video/hevc` | H.265/HEVC | High efficiency, newer codec |
| `video/x-h265` | H.265/HEVC | Alternative MIME type |
| `video/mp4` | MPEG-4 | Legacy MPEG-4 video |
| `video/x-ms-wmv` | WMV | Windows Media Video |
| `video/mpeg` | MPEG-1 | Legacy MPEG-1 video |

### Error Handling

```cpp
auto decoder = std::make_shared<MediaFoundationDecoder>();

if (!decoder->Create("video/avc", MediaFoundationDecoder::Mode::BUFFER)) {
    LogError("Failed to create decoder - check codec support");
    return false;
}

if (!decoder->Configure(1920, 1080)) {
    LogError("Failed to configure decoder - check resolution");
    decoder->Release();
    return false;
}

if (!decoder->Start()) {
    LogError("Failed to start decoder");
    decoder->Release();
    return false;
}

// Queue input with error checking
if (!decoder->QueueInputBuffer(data, size, timestamp, isKeyFrame)) {
    if (decoder->HasDecodedFrame()) {
        // Need to drain output first
        decoder->GetDecodedBitmap(bitmap);
    } else {
        LogError("Failed to queue input");
    }
}
```

### Resource Management

Always properly release resources:

```cpp
// Option 1: Explicit cleanup
decoder->Stop();
decoder->Release();

// Option 2: Automatic cleanup via destructor
{
    auto decoder = std::make_shared<MediaFoundationDecoder>();
    // ... use decoder ...
} // Automatically released when going out of scope
```

### Performance Tips

1. **Reuse Bitmaps**: Create one bitmap and reuse it for multiple frames to avoid allocations
2. **Batch Processing**: Queue multiple input buffers before reading output
3. **Check Frame Availability**: Use `HasDecodedFrame()` to check before calling `GetDecodedBitmap()`
4. **Flush When Seeking**: Call `Flush()` when seeking to clear decoder state

```cpp
// Efficient decoding loop
auto bitmap = std::make_shared<Bitmap>(); // Reuse same bitmap

for (auto& packet : videoPackets) {
    decoder->QueueInputBuffer(packet.data, packet.size, packet.pts, packet.isKey);
    
    // Check if frames are available
    while (decoder->HasDecodedFrame()) {
        if (decoder->GetDecodedBitmap(bitmap, nullptr)) {
            ProcessFrame(bitmap); // Process without reallocating
        }
    }
}
```

### Troubleshooting

**Problem**: Decoder fails to create
- **Solution**: Ensure the codec is supported on your Windows version. H.264 is universally supported.

**Problem**: No output frames
- **Solution**: Check that you're queuing complete video packets with proper timestamps and keyframe flags.

**Problem**: Poor performance
- **Solution**: Verify hardware acceleration is enabled. Check Windows hardware acceleration settings.

**Problem**: Frames are corrupted
- **Solution**: Ensure input data is valid H.264/H.265 elementary stream data, not container format (MP4, MKV).

## API Reference

### Methods

- `Create(mimeType, mode)` - Initialize decoder with codec type and mode
- `Configure(width, height, frameRate, bitrate)` - Set video parameters
- `Start()` - Start the decoder
- `Stop()` - Stop the decoder
- `Release()` - Release all resources
- `QueueInputBuffer(data, size, timestamp, isKeyFrame)` - Queue encoded data
- `SignalEndOfStream()` - Signal no more input
- `GetDecodedBitmap(bitmap, timestamp)` - Get decoded frame
- `HasDecodedFrame()` - Check if frame is available
- `Flush()` - Clear decoder buffers
- `SetOutputFormat(format)` - Set output pixel format

### Properties

- `GetWidth()` - Get configured width
- `GetHeight()` - Get configured height
- `GetMode()` - Get decoding mode
- `IsRunning()` - Check if decoder is running

## Integration with ULUI Components

### With OpenGL Textures

```cpp
auto decoder = std::make_shared<MediaFoundationDecoder>();
auto bitmap = std::make_shared<Bitmap>();
auto texture = std::make_shared<gl::Texture2D>();

// Decode frame
decoder->GetDecodedBitmap(bitmap);

// Upload to texture
bitmap->UploadToTexture(texture.get());

// Use texture in rendering
texture->Bind(0);
// ... render ...
```

### With File System

```cpp
// Save decoded frames
if (decoder->GetDecodedBitmap(bitmap)) {
    std::string path = FileSystem::GetExternalDataPath() + "frame_" + 
                       std::to_string(frameNum) + ".png";
    bitmap->SaveToFile(path.c_str());
}
```

## See Also

- [Bitmap Documentation](BITMAP_CN.md)
- [Android MediaCodec Integration](MEDIACODEC_CN.md)
- [OpenGL Classes Documentation](OPENGL_CLASSES.md)
