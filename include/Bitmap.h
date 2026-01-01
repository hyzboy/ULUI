#pragma once

#include "BitmapFormat.h"
#include "object.h"
#include <memory>
#include <cstdint>

// Forward declarations for platform-specific types
#ifdef ANDROID
struct AHardwareBuffer;
#endif

#ifdef __APPLE__
typedef struct __CVBuffer *CVPixelBufferRef;
#endif

// Forward declaration for FFmpeg
struct AVFrame;

namespace ului {
namespace gl {
class Texture2D;
}

/**
 * Bitmap class for image/video frame data interchange
 * 
 * This class serves as a bridge between various image/video sources
 * (cameras, decoders, files) and OpenGL textures. It supports:
 * - Zero-copy wrapping of external data
 * - Multiple pixel formats (RGB, YUV, platform-specific)
 * - Conversion between formats
 * - Upload to/download from GPU textures
 * 
 * Design philosophy:
 * - Avoid unnecessary memory copies
 * - Support platform-specific optimizations
 * - Provide unified interface across platforms
 */
class Bitmap : public Object {
public:
    Bitmap();
    ~Bitmap() override;

    // Disable copy (use shared_ptr for sharing)
    Bitmap(const Bitmap&) = delete;
    Bitmap& operator=(const Bitmap&) = delete;

    /**
     * Create bitmap with internal memory allocation
     * @param format Bitmap format specification
     * @return true if successful
     */
    bool Create(const BitmapFormat& format);

    /**
     * Wrap external data (zero-copy, no ownership)
     * @param data Pointer to pixel data
     * @param format Bitmap format specification
     * @return true if successful
     */
    bool WrapExternalData(void* data, const BitmapFormat& format);

    /**
     * Wrap external data with multiple planes (for planar YUV)
     * @param planes Array of pointers to plane data
     * @param numPlanes Number of planes
     * @param format Bitmap format specification
     * @return true if successful
     */
    bool WrapExternalDataPlanes(void** planes, int numPlanes, const BitmapFormat& format);

    // Platform-specific wrapping functions
#ifdef ANDROID
    /**
     * Wrap Android Hardware Buffer
     * @param buffer Android AHardwareBuffer pointer
     * @return true if successful
     */
    bool WrapHardwareBuffer(AHardwareBuffer* buffer);
#endif

#ifdef __APPLE__
    /**
     * Wrap iOS CVPixelBuffer
     * @param buffer CVPixelBuffer reference
     * @return true if successful
     */
    bool WrapCVPixelBuffer(CVPixelBufferRef buffer);
#endif

    /**
     * Wrap FFmpeg AVFrame
     * @param frame FFmpeg AVFrame pointer
     * @return true if successful
     */
    bool WrapFFmpegFrame(AVFrame* frame);

    /**
     * Load bitmap from file
     * @param path File path
     * @return true if successful
     */
    bool LoadFromFile(const char* path);

    /**
     * Save bitmap to file
     * @param path File path
     * @return true if successful
     */
    bool SaveToFile(const char* path);

    /**
     * Convert bitmap to a different format
     * @param targetFormat Target pixel format
     * @return true if successful (modifies this bitmap)
     */
    bool ConvertTo(PixelFormat targetFormat);

    /**
     * Upload bitmap data to OpenGL texture
     * @param texture Texture2D to upload to
     * @return true if successful
     */
    bool UploadToTexture(gl::Texture2D* texture);

    /**
     * Download data from OpenGL texture to bitmap
     * @param texture Texture2D to download from
     * @return true if successful
     */
    bool DownloadFromTexture(gl::Texture2D* texture);

    /**
     * Get bitmap format
     */
    const BitmapFormat& GetFormat() const { return m_format; }

    /**
     * Get bitmap width
     */
    int GetWidth() const { return m_format.width; }

    /**
     * Get bitmap height
     */
    int GetHeight() const { return m_format.height; }

    /**
     * Get pixel format
     */
    PixelFormat GetPixelFormat() const { return m_format.pixelFormat; }

    /**
     * Get pointer to pixel data (plane 0 for planar formats)
     */
    void* GetData() { return m_data; }
    const void* GetData() const { return m_data; }

    /**
     * Get pointer to specific plane data (for planar formats)
     * @param plane Plane index (0, 1, 2 for YUV)
     */
    void* GetPlaneData(int plane);
    const void* GetPlaneData(int plane) const;

    /**
     * Get number of planes
     */
    int GetNumPlanes() const { return m_numPlanes; }

    /**
     * Check if bitmap has valid data
     */
    bool IsValid() const { return m_data != nullptr && m_format.width > 0 && m_format.height > 0; }

    /**
     * Get ownership mode
     */
    BitmapOwnership GetOwnership() const { return m_ownership; }

    /**
     * Clear bitmap and release resources
     */
    void Clear();

private:
    BitmapFormat m_format;
    BitmapOwnership m_ownership;
    
    // Data pointers
    void* m_data;               // Main data pointer (or plane 0)
    void* m_planes[3];          // Additional planes for planar formats
    int m_numPlanes;
    
    // Platform-specific handles
    void* m_platformHandle;     // Opaque platform-specific handle
    
    // Helper functions
    void AllocateInternalMemory();
    void FreeInternalMemory();
    bool ConvertRGBToRGB(PixelFormat targetFormat);
    bool ConvertYUVToRGB(PixelFormat targetFormat);
    bool ConvertRGBToYUV(PixelFormat targetFormat);
};

} // namespace ului
