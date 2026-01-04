#pragma once

#ifdef _WIN32

#include "object.h"
#include <memory>
#include <cstdint>

namespace ului {

class Bitmap;

/**
 * @brief Windows Media Foundation hardware video decoder wrapper
 * 
 * Provides high-level interface for hardware video decoding on Windows.
 * Uses Media Foundation API for efficient video decoding.
 */
class MediaFoundationDecoder : public Object {
public:
    enum class Mode {
        BUFFER,     // CPU buffer mode (output to CPU memory/Bitmap)
        TEXTURE     // Hardware texture mode (zero-copy to GPU texture)
    };

    MediaFoundationDecoder();
    virtual ~MediaFoundationDecoder();

    /**
     * @brief Create decoder for specified codec
     * @param mimeType MIME type (e.g., "video/mp4", "video/x-h264", "video/x-h265")
     * @param mode Decoding mode (BUFFER or TEXTURE)
     * @return true on success
     */
    bool Create(const char* mimeType, Mode mode = Mode::BUFFER);

    /**
     * @brief Configure decoder with format parameters
     * @param width Video width
     * @param height Video height
     * @param frameRate Frame rate (optional, 0 = auto)
     * @param bitrate Bitrate (optional, 0 = auto)
     * @return true on success
     */
    bool Configure(int width, int height, float frameRate = 0, int bitrate = 0);

    /**
     * @brief Start the decoder
     * @return true on success
     */
    bool Start();

    /**
     * @brief Stop the decoder
     * @return true on success
     */
    bool Stop();

    /**
     * @brief Release all resources
     */
    void Release();

    /**
     * @brief Queue encoded input data for decoding
     * @param data Encoded data
     * @param size Data size
     * @param presentationTimeUs Presentation timestamp in microseconds
     * @param isKeyFrame Whether this is a keyframe
     * @return true on success
     */
    bool QueueInputBuffer(const uint8_t* data, size_t size, 
                         int64_t presentationTimeUs, bool isKeyFrame = false);

    /**
     * @brief Signal end of stream
     * @return true on success
     */
    bool SignalEndOfStream();

    /**
     * @brief Get decoded output as Bitmap (BUFFER mode only)
     * @param bitmap Bitmap to receive decoded data
     * @param presentationTimeUs Output presentation timestamp
     * @return true if frame available and decoded
     */
    bool GetDecodedBitmap(std::shared_ptr<Bitmap> bitmap, int64_t* presentationTimeUs = nullptr);

    /**
     * @brief Check if a decoded frame is available
     * @return true if frame is ready to be retrieved
     * @note This method is non-const as it may query decoder state
     */
    bool HasDecodedFrame();

    /**
     * @brief Flush decoder buffers
     * @return true on success
     */
    bool Flush();

    /**
     * @brief Set output format (pixel format for decoded frames)
     * @param format Desired output format (e.g., PixelFormat::RGBA8, PixelFormat::NV12)
     * @return true on success
     */
    bool SetOutputFormat(int format);

    // Getters
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    Mode GetMode() const { return m_mode; }
    bool IsRunning() const { return m_isRunning; }

private:
    // Forward declaration for Media Foundation types (to avoid including Windows headers here)
    struct MediaFoundationContext;
    MediaFoundationContext* m_context;
    
    Mode m_mode;
    int m_width;
    int m_height;
    float m_frameRate;
    int m_bitrate;
    bool m_isRunning;
    bool m_hasFrame;
    
    // Internal helper methods
    bool InitializeMediaFoundation();
    void CleanupMediaFoundation();
    bool CreateDecoderTransform(const char* mimeType);
    bool ConvertSampleToBitmap(void* sample, std::shared_ptr<Bitmap> bitmap);
    const char* MimeTypeToSubtype(const char* mimeType);
};

} // namespace ului

#endif // _WIN32
