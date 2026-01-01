#include "BitmapFormat.h"

namespace ului {

int BitmapFormat::GetBytesPerPixel() const
{
    switch (pixelFormat) {
        // 8-bit per channel RGB
        case PixelFormat::RGB8:
        case PixelFormat::BGR8:
            return 3;
            
        case PixelFormat::RGBA8:
        case PixelFormat::BGRA8:
            return 4;
            
        // 16-bit per channel RGB
        case PixelFormat::RGB16:
            return 6;
            
        case PixelFormat::RGBA16:
            return 8;
            
        // Grayscale
        case PixelFormat::GRAY8:
            return 1;
            
        case PixelFormat::GRAY16:
            return 2;
            
        // Packed YUV formats
        case PixelFormat::YUYV:
        case PixelFormat::UYVY:
            return 2;  // 2 bytes per pixel on average
            
        // Planar and semi-planar formats don't have simple bytes-per-pixel
        case PixelFormat::YUV420P:
        case PixelFormat::YUV422P:
        case PixelFormat::YUV444P:
        case PixelFormat::NV12:
        case PixelFormat::NV21:
            return 0;  // Must calculate separately
            
        // Platform-specific and compressed formats
        case PixelFormat::ANDROID_HARDWARE_BUFFER:
        case PixelFormat::IOS_CVPIXELBUFFER:
        case PixelFormat::MEDIACODEC_BUFFER:
        case PixelFormat::JPEG:
        case PixelFormat::PNG:
        case PixelFormat::H264:
        case PixelFormat::H265:
        case PixelFormat::UNKNOWN:
        default:
            return 0;
    }
}

size_t BitmapFormat::GetDataSize() const
{
    if (width <= 0 || height <= 0) {
        return 0;
    }
    
    int bpp = GetBytesPerPixel();
    if (bpp > 0) {
        // Simple packed formats
        int actualStride = stride > 0 ? stride : (width * bpp);
        return actualStride * height;
    }
    
    // Special handling for planar formats
    switch (pixelFormat) {
        case PixelFormat::YUV420P:
        case PixelFormat::NV12:
        case PixelFormat::NV21:
            // Y plane + UV planes (1/4 size each for 4:2:0)
            return width * height + (width / 2) * (height / 2) * 2;
            
        case PixelFormat::YUV422P:
            // Y plane + U plane (half width) + V plane (half width)
            return width * height + (width / 2) * height * 2;
            
        case PixelFormat::YUV444P:
            // Y + U + V planes (all same size)
            return width * height * 3;
            
        default:
            return 0;
    }
}

bool BitmapFormat::IsYUV() const
{
    switch (pixelFormat) {
        case PixelFormat::YUV420P:
        case PixelFormat::YUV422P:
        case PixelFormat::YUV444P:
        case PixelFormat::NV12:
        case PixelFormat::NV21:
        case PixelFormat::YUYV:
        case PixelFormat::UYVY:
            return true;
        default:
            return false;
    }
}

bool BitmapFormat::IsRGB() const
{
    switch (pixelFormat) {
        case PixelFormat::RGB8:
        case PixelFormat::RGBA8:
        case PixelFormat::BGR8:
        case PixelFormat::BGRA8:
        case PixelFormat::RGB16:
        case PixelFormat::RGBA16:
            return true;
        default:
            return false;
    }
}

bool BitmapFormat::IsPlanar() const
{
    switch (pixelFormat) {
        case PixelFormat::YUV420P:
        case PixelFormat::YUV422P:
        case PixelFormat::YUV444P:
            return true;
        default:
            return false;
    }
}

bool BitmapFormat::IsCompressed() const
{
    switch (pixelFormat) {
        case PixelFormat::JPEG:
        case PixelFormat::PNG:
        case PixelFormat::H264:
        case PixelFormat::H265:
            return true;
        default:
            return false;
    }
}

} // namespace ului
