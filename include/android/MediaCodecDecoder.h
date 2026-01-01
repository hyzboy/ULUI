#pragma once

#ifdef __ANDROID__

#include "object.h"
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <android/native_window.h>
#include <memory>

namespace ului {

class Bitmap;
class Texture2D;

/**
 * @brief Android MediaCodec hardware video decoder wrapper
 * 
 * Provides high-level interface for hardware video decoding on Android.
 * Supports both buffer mode and surface (zero-copy) mode.
 */
class MediaCodecDecoder : public Object {
public:
    enum class Mode {
        BUFFER,     // CPU buffer mode (output to CPU memory)
        SURFACE     // Surface mode (zero-copy to GPU texture)
    };

    MediaCodecDecoder();
    virtual ~MediaCodecDecoder();

    /**
     * @brief Create decoder for specified codec
     * @param mimeType MIME type (e.g., "video/avc", "video/hevc")
     * @param mode Decoding mode (BUFFER or SURFACE)
     * @return true on success
     */
    bool Create(const char* mimeType, Mode mode = Mode::SURFACE);

    /**
     * @brief Configure decoder with format parameters
     * @param width Video width
     * @param height Video height
     * @param csd0 Codec specific data 0 (SPS for H.264)
     * @param csd0Size Size of csd0
     * @param csd1 Codec specific data 1 (PPS for H.264, optional)
     * @param csd1Size Size of csd1
     * @return true on success
     */
    bool Configure(int width, int height, 
                   const uint8_t* csd0 = nullptr, size_t csd0Size = 0,
                   const uint8_t* csd1 = nullptr, size_t csd1Size = 0);

    /**
     * @brief Set output surface for SURFACE mode (must be called before Start)
     * @param window Native window for output (from ANativeWindow or SurfaceTexture)
     * @return true on success
     */
    bool SetOutputSurface(ANativeWindow* window);

    /**
     * @brief Set output texture for SURFACE mode (must be called before Start)
     * @param texture OES EXTERNAL texture for zero-copy output
     * @return true on success
     */
    bool SetOutputTexture(std::shared_ptr<Texture2D> texture);

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
     * @param flags Input flags (e.g., AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG)
     * @return true on success
     */
    bool QueueInputBuffer(const uint8_t* data, size_t size, 
                         int64_t presentationTimeUs, uint32_t flags = 0);

    /**
     * @brief Signal end of stream
     * @return true on success
     */
    bool SignalEndOfStream();

    /**
     * @brief Get decoded output (BUFFER mode only)
     * @param buffer Output buffer pointer (owned by decoder)
     * @param size Output buffer size
     * @param presentationTimeUs Presentation timestamp
     * @param flags Output buffer flags
     * @return true if output available, false if need to try again
     */
    bool GetDecodedData(uint8_t** buffer, size_t* size, 
                       int64_t* presentationTimeUs, uint32_t* flags);

    /**
     * @brief Release output buffer after processing
     * @param index Buffer index from GetDecodedData
     * @param render true to render to surface (SURFACE mode only)
     * @return true on success
     */
    bool ReleaseOutputBuffer(size_t index, bool render = false);

    /**
     * @brief Render decoded frame to output surface (SURFACE mode only)
     * @return true if frame available and rendered
     */
    bool RenderFrame();

    /**
     * @brief Get decoded output as Bitmap (BUFFER mode only)
     * @param bitmap Bitmap to receive decoded data
     * @return true on success
     */
    bool GetDecodedBitmap(std::shared_ptr<Bitmap> bitmap);

    // Getters
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    Mode GetMode() const { return m_mode; }
    bool IsRunning() const { return m_isRunning; }

private:
    AMediaCodec* m_codec;
    AMediaFormat* m_format;
    ANativeWindow* m_outputSurface;
    std::shared_ptr<Texture2D> m_outputTexture;
    
    Mode m_mode;
    int m_width;
    int m_height;
    bool m_isRunning;
    
    ssize_t m_currentOutputIndex;
    AMediaCodecBufferInfo m_outputInfo;
};

} // namespace ului

#endif // __ANDROID__
