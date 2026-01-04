#pragma once

#ifdef _WIN32

#include "object.h"
#include "BitmapFormat.h"
#include <memory>
#include <cstdint>

namespace ului {

class Bitmap;

/**
 * @brief Windows Media Foundation hardware video encoder wrapper
 * 
 * Provides high-level interface for hardware video encoding on Windows.
 * Uses Media Foundation API for efficient video encoding.
 */
class MediaFoundationEncoder : public Object {
public:
    enum class Mode {
        BUFFER,     // CPU buffer mode (input from Bitmap)
        TEXTURE     // Hardware texture mode (zero-copy from GPU, future)
    };

    MediaFoundationEncoder();
    virtual ~MediaFoundationEncoder();

    /**
     * @brief Create encoder for specified codec
     * @param mimeType MIME type (e.g., "video/avc", "video/hevc")
     * @param width Video width
     * @param height Video height
     * @param bitrate Target bitrate in bps
     * @param frameRate Target frame rate
     * @param mode Encoding mode (BUFFER or TEXTURE)
     * @return true on success
     */
    bool Create(const char* mimeType, int width, int height, 
                int bitrate, int frameRate, Mode mode = Mode::BUFFER);

    /**
     * @brief Start the encoder
     * @return true on success
     */
    bool Start();

    /**
     * @brief Stop the encoder
     * @return true on success
     */
    bool Stop();

    /**
     * @brief Release all resources
     */
    void Release();

    /**
     * @brief Encode a frame from Bitmap (BUFFER mode only)
     * @param bitmap Source bitmap with video frame data
     * @param presentationTimeUs Presentation timestamp in microseconds
     * @param forceKeyFrame Force this frame to be a keyframe
     * @return true on success
     */
    bool EncodeFrame(std::shared_ptr<Bitmap> bitmap, int64_t presentationTimeUs, 
                    bool forceKeyFrame = false);

    /**
     * @brief Signal end of stream
     * @return true on success
     */
    bool SignalEndOfStream();

    /**
     * @brief Get encoded output
     * @param buffer Output buffer pointer (owned by encoder, valid until next call)
     * @param size Output buffer size
     * @param presentationTimeUs Presentation timestamp
     * @param isKeyFrame Whether this is a keyframe
     * @return true if output available, false if need to try again
     */
    bool GetEncodedData(uint8_t** buffer, size_t* size, 
                       int64_t* presentationTimeUs, bool* isKeyFrame);

    /**
     * @brief Get codec configuration data (SPS/PPS for H.264/H.265)
     * @param buffer Output buffer pointer (owned by encoder)
     * @param size Output buffer size
     * @return true on success
     */
    bool GetConfigData(uint8_t** buffer, size_t* size);

    /**
     * @brief Check if encoded data is available
     * @return true if encoded data is ready to be retrieved
     */
    bool HasEncodedData();

    /**
     * @brief Flush encoder buffers
     * @return true on success
     */
    bool Flush();

    /**
     * @brief Set input format (pixel format for input frames)
     * @param format Input pixel format (e.g., PixelFormat::NV12, PixelFormat::RGBA8)
     * @return true on success
     */
    bool SetInputFormat(PixelFormat format);

    /**
     * @brief Set keyframe interval
     * @param intervalSeconds Interval between keyframes in seconds
     * @return true on success
     */
    bool SetKeyframeInterval(int intervalSeconds);

    // Getters
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetBitrate() const { return m_bitrate; }
    int GetFrameRate() const { return m_frameRate; }
    Mode GetMode() const { return m_mode; }
    bool IsRunning() const { return m_isRunning; }

private:
    // Forward declaration for Media Foundation types
    struct MediaFoundationContext;
    MediaFoundationContext* m_context;
    
    Mode m_mode;
    int m_width;
    int m_height;
    int m_bitrate;
    int m_frameRate;
    int m_keyframeInterval;
    bool m_isRunning;
    bool m_hasData;
    PixelFormat m_inputFormat;
    
    // Internal helper methods
    bool InitializeMediaFoundation();
    void CleanupMediaFoundation();
    bool CreateEncoderTransform(const char* mimeType);
    bool ConvertBitmapToSample(std::shared_ptr<Bitmap> bitmap, void** sample, int64_t timestampUs);
    const char* MimeTypeToSubtype(const char* mimeType);
};

} // namespace ului

#endif // _WIN32
