#include "Bitmap.h"
#include "gl/Texture2D.h"
#include <cstring>
#include <cstdlib>

namespace ului {

Bitmap::Bitmap()
    : Object("Bitmap")
    , m_ownership(BitmapOwnership::EXTERNAL)
    , m_data(nullptr)
    , m_numPlanes(0)
    , m_platformHandle(nullptr)
{
    m_planes[0] = nullptr;
    m_planes[1] = nullptr;
    m_planes[2] = nullptr;
    LogD("Bitmap constructed");
}

Bitmap::~Bitmap()
{
    Clear();
}

bool Bitmap::Create(const BitmapFormat& format)
{
    Clear();
    
    m_format = format;
    m_ownership = BitmapOwnership::INTERNAL;
    
    // Calculate stride if not specified
    if (m_format.stride == 0) {
        int bpp = m_format.GetBytesPerPixel();
        if (bpp > 0) {
            m_format.stride = m_format.width * bpp;
        }
    }
    
    AllocateInternalMemory();
    
    if (!m_data) {
        LogE("Failed to allocate bitmap memory");
        return false;
    }
    
    LogI("Bitmap created: %dx%d, format=%d", m_format.width, m_format.height, 
         static_cast<int>(m_format.pixelFormat));
    return true;
}

bool Bitmap::WrapExternalData(void* data, const BitmapFormat& format)
{
    if (!data) {
        LogE("Cannot wrap null data");
        return false;
    }
    
    Clear();
    
    m_format = format;
    m_ownership = BitmapOwnership::EXTERNAL;
    m_data = data;
    m_numPlanes = 1;
    m_planes[0] = data;
    
    LogD("Bitmap wrapped external data: %dx%d, format=%d", 
         m_format.width, m_format.height, static_cast<int>(m_format.pixelFormat));
    return true;
}

bool Bitmap::WrapExternalDataPlanes(void** planes, int numPlanes, const BitmapFormat& format)
{
    if (!planes || numPlanes <= 0 || numPlanes > 3) {
        LogE("Invalid planes data");
        return false;
    }
    
    Clear();
    
    m_format = format;
    m_ownership = BitmapOwnership::EXTERNAL;
    m_data = planes[0];
    m_numPlanes = numPlanes;
    
    for (int i = 0; i < numPlanes && i < 3; ++i) {
        m_planes[i] = planes[i];
    }
    
    LogD("Bitmap wrapped %d planes: %dx%d, format=%d", numPlanes,
         m_format.width, m_format.height, static_cast<int>(m_format.pixelFormat));
    return true;
}

bool Bitmap::WrapFFmpegFrame(AVFrame* frame)
{
    if (!frame) {
        LogE("Cannot wrap null AVFrame");
        return false;
    }
    
    // Note: This function requires FFmpeg headers to be available
    // For now, just return false with a warning
    LogW("FFmpeg frame wrapping requires FFmpeg headers - not available in current build");
    return false;
    
    /* Full implementation would look like:
    // Map AVFrame format to PixelFormat
    PixelFormat format = MapAVPixelFormatToPixelFormat(frame->format);
    
    BitmapFormat bitmapFormat;
    bitmapFormat.pixelFormat = format;
    bitmapFormat.width = frame->width;
    bitmapFormat.height = frame->height;
    bitmapFormat.stride = frame->linesize[0];
    
    // Wrap the frame data
    if (frame->data[1] || frame->data[2]) {
        // Planar format
        void* planes[3] = { frame->data[0], frame->data[1], frame->data[2] };
        int numPlanes = frame->data[2] ? 3 : 2;
        return WrapExternalDataPlanes(planes, numPlanes, bitmapFormat);
    } else {
        // Packed format
        return WrapExternalData(frame->data[0], bitmapFormat);
    }
    */
}

bool Bitmap::LoadFromFile(const char* path)
{
    if (!path) {
        LogE("Invalid file path");
        return false;
    }
    
    // File loading not implemented yet - would use stb_image or platform API
    LogW("LoadFromFile not implemented yet: %s", path);
    return false;
}

bool Bitmap::SaveToFile(const char* path)
{
    if (!path || !IsValid()) {
        LogE("Cannot save: invalid path or bitmap");
        return false;
    }
    
    // File saving not implemented yet - would use stb_image_write or platform API
    LogW("SaveToFile not implemented yet: %s", path);
    return false;
}

bool Bitmap::ConvertTo(PixelFormat targetFormat)
{
    if (!IsValid()) {
        LogE("Cannot convert invalid bitmap");
        return false;
    }
    
    if (m_format.pixelFormat == targetFormat) {
        LogD("Already in target format");
        return true;
    }
    
    // Format conversion not fully implemented yet
    LogW("Format conversion not fully implemented: %d -> %d", 
         static_cast<int>(m_format.pixelFormat), static_cast<int>(targetFormat));
    return false;
}

bool Bitmap::UploadToTexture(gl::Texture2D* texture)
{
    if (!texture || !IsValid()) {
        LogE("Cannot upload: invalid texture or bitmap");
        return false;
    }
    
    // Map PixelFormat to OpenGL format
    GLenum glFormat = GL_RGBA;
    GLenum glType = GL_UNSIGNED_BYTE;
    GLint glInternalFormat = GL_RGBA8;
    
    switch (m_format.pixelFormat) {
        case PixelFormat::RGB8:
            glFormat = GL_RGB;
            glInternalFormat = GL_RGB8;
            break;
        case PixelFormat::RGBA8:
            glFormat = GL_RGBA;
            glInternalFormat = GL_RGBA8;
            break;
        case PixelFormat::BGR8:
        case PixelFormat::BGRA8:
            LogW("BGR/BGRA formats require conversion to RGB/RGBA");
            // Would need to convert BGR -> RGB first
            return false;
        default:
            LogW("Unsupported pixel format for OpenGL upload: %d", 
                 static_cast<int>(m_format.pixelFormat));
            return false;
    }
    
    texture->SetImage(m_format.width, m_format.height, glInternalFormat, 
                     glFormat, glType, m_data);
    
    LogD("Bitmap uploaded to texture: %dx%d", m_format.width, m_format.height);
    return true;
}

bool Bitmap::DownloadFromTexture(gl::Texture2D* texture)
{
    if (!texture || !texture->IsValid()) {
        LogE("Cannot download from invalid texture");
        return false;
    }
    
    // Download from texture not fully implemented yet
    // Would use glReadPixels
    LogW("DownloadFromTexture not fully implemented yet");
    return false;
}

void* Bitmap::GetPlaneData(int plane)
{
    if (plane < 0 || plane >= m_numPlanes || plane >= 3) {
        return nullptr;
    }
    return m_planes[plane];
}

const void* Bitmap::GetPlaneData(int plane) const
{
    if (plane < 0 || plane >= m_numPlanes || plane >= 3) {
        return nullptr;
    }
    return m_planes[plane];
}

void Bitmap::Clear()
{
    if (m_ownership == BitmapOwnership::INTERNAL) {
        FreeInternalMemory();
    }
    
    m_data = nullptr;
    m_planes[0] = nullptr;
    m_planes[1] = nullptr;
    m_planes[2] = nullptr;
    m_numPlanes = 0;
    m_platformHandle = nullptr;
    m_ownership = BitmapOwnership::EXTERNAL;
    m_format = BitmapFormat();
}

void Bitmap::AllocateInternalMemory()
{
    size_t dataSize = m_format.GetDataSize();
    if (dataSize == 0) {
        LogE("Cannot allocate memory for format with unknown size");
        return;
    }
    
    m_data = std::malloc(dataSize);
    if (m_data) {
        std::memset(m_data, 0, dataSize);
        m_numPlanes = 1;
        m_planes[0] = m_data;
        
        // For planar formats, set up plane pointers
        if (m_format.IsPlanar()) {
            uint8_t* ptr = static_cast<uint8_t*>(m_data);
            switch (m_format.pixelFormat) {
                case PixelFormat::YUV420P: {
                    int ySize = m_format.width * m_format.height;
                    int uvSize = (m_format.width / 2) * (m_format.height / 2);
                    m_planes[0] = ptr;
                    m_planes[1] = ptr + ySize;
                    m_planes[2] = ptr + ySize + uvSize;
                    m_numPlanes = 3;
                    break;
                }
                case PixelFormat::YUV422P: {
                    int ySize = m_format.width * m_format.height;
                    int uvSize = (m_format.width / 2) * m_format.height;
                    m_planes[0] = ptr;
                    m_planes[1] = ptr + ySize;
                    m_planes[2] = ptr + ySize + uvSize;
                    m_numPlanes = 3;
                    break;
                }
                case PixelFormat::YUV444P: {
                    int planeSize = m_format.width * m_format.height;
                    m_planes[0] = ptr;
                    m_planes[1] = ptr + planeSize;
                    m_planes[2] = ptr + planeSize * 2;
                    m_numPlanes = 3;
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void Bitmap::FreeInternalMemory()
{
    if (m_data) {
        std::free(m_data);
        m_data = nullptr;
    }
}

#ifdef ANDROID
bool Bitmap::WrapHardwareBuffer(AHardwareBuffer* buffer)
{
    if (!buffer) {
        LogE("Cannot wrap null AHardwareBuffer");
        return false;
    }
    
    // Android Hardware Buffer wrapping not fully implemented
    LogW("WrapHardwareBuffer not fully implemented yet");
    m_platformHandle = buffer;
    return false;
}
#endif

#ifdef __APPLE__
bool Bitmap::WrapCVPixelBuffer(CVPixelBufferRef buffer)
{
    if (!buffer) {
        LogE("Cannot wrap null CVPixelBuffer");
        return false;
    }
    
    // iOS CVPixelBuffer wrapping not fully implemented
    LogW("WrapCVPixelBuffer not fully implemented yet");
    m_platformHandle = (void*)buffer;
    return false;
}
#endif

} // namespace ului
