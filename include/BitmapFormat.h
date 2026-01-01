#pragma once

#include <cstdint>
#include <cstddef>

namespace ului {

/**
 * Pixel format enumeration for various image/video formats
 */
enum class PixelFormat {
    // Unknown/Invalid
    UNKNOWN = 0,
    
    // RGB formats (8-bit per channel)
    RGB8,           // 24-bit RGB
    RGBA8,          // 32-bit RGBA
    BGR8,           // 24-bit BGR
    BGRA8,          // 32-bit BGRA
    
    // RGB formats (16-bit per channel)
    RGB16,          // 48-bit RGB
    RGBA16,         // 64-bit RGBA
    
    // Grayscale
    GRAY8,          // 8-bit grayscale
    GRAY16,         // 16-bit grayscale
    
    // YUV planar formats (common in video)
    YUV420P,        // YUV 4:2:0 planar (I420)
    YUV422P,        // YUV 4:2:2 planar
    YUV444P,        // YUV 4:4:4 planar
    
    // YUV packed/semi-planar formats
    NV12,           // YUV 4:2:0 semi-planar (Y plane + interleaved UV)
    NV21,           // YUV 4:2:0 semi-planar (Y plane + interleaved VU) - Android
    YUYV,           // YUV 4:2:2 packed
    UYVY,           // YUV 4:2:2 packed
    
    // Platform-specific formats (opaque handles)
    ANDROID_HARDWARE_BUFFER,    // Android AHardwareBuffer
    IOS_CVPIXELBUFFER,          // iOS CVPixelBuffer
    MEDIACODEC_BUFFER,          // Android MediaCodec buffer
    
    // Compressed formats (not raw pixels)
    JPEG,
    PNG,
    H264,
    H265
};

/**
 * Color space definition
 */
enum class ColorSpace {
    UNKNOWN = 0,
    SRGB,           // Standard RGB
    LINEAR,         // Linear RGB
    BT601,          // ITU-R BT.601 (SD video)
    BT709,          // ITU-R BT.709 (HD video)
    BT2020,         // ITU-R BT.2020 (UHD video)
};

/**
 * Data ownership mode for Bitmap
 */
enum class BitmapOwnership {
    EXTERNAL,       // Bitmap wraps external data (no ownership)
    INTERNAL        // Bitmap owns the data (allocated internally)
};

/**
 * Complete format description for bitmap data
 */
struct BitmapFormat {
    PixelFormat pixelFormat;
    int width;
    int height;
    int stride;                 // Bytes per row (0 = auto-calculate)
    ColorSpace colorSpace;
    bool premultipliedAlpha;    // Alpha is premultiplied
    
    BitmapFormat()
        : pixelFormat(PixelFormat::UNKNOWN)
        , width(0)
        , height(0)
        , stride(0)
        , colorSpace(ColorSpace::UNKNOWN)
        , premultipliedAlpha(false)
    {
    }
    
    BitmapFormat(PixelFormat fmt, int w, int h, int s = 0)
        : pixelFormat(fmt)
        , width(w)
        , height(h)
        , stride(s)
        , colorSpace(ColorSpace::SRGB)
        , premultipliedAlpha(false)
    {
    }
    
    /**
     * Calculate bytes per pixel for the format
     * @return Bytes per pixel, or 0 for planar/compressed formats
     */
    int GetBytesPerPixel() const;
    
    /**
     * Calculate total data size in bytes
     * @return Total size, or 0 if cannot be calculated
     */
    size_t GetDataSize() const;
    
    /**
     * Check if format is a YUV variant
     */
    bool IsYUV() const;
    
    /**
     * Check if format is RGB variant
     */
    bool IsRGB() const;
    
    /**
     * Check if format is planar (YUV420P, YUV422P, etc.)
     */
    bool IsPlanar() const;
    
    /**
     * Check if format is compressed
     */
    bool IsCompressed() const;
};

} // namespace ului
