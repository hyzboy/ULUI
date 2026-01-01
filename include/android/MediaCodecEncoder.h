#pragma once

#ifdef __ANDROID__

#include "object.h"
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <android/native_window.h>
#include <memory>

namespace ului {

class Texture2D;
class RenderTarget;

/**
 * @brief Android MediaCodec hardware video encoder wrapper
 * 
 * Provides high-level interface for hardware video encoding on Android.
 * Supports both buffer mode and surface (zero-copy) mode.
 */
class MediaCodecEncoder : public Object {
public:
    enum class Mode {
        BUFFER,     // CPU buffer mode (requires glReadPixels)
        SURFACE     // Surface mode (zero-copy from GPU)
    };

    MediaCodecEncoder();
    virtual ~MediaCodecEncoder();

    /**
     * @brief Create encoder for specified codec
     * @param mimeType MIME type (e.g., "video/avc", "video/hevc")
     * @param width Video width
     * @param height Video height
     * @param bitrate Target bitrate in bps
     * @param frameRate Target frame rate
     * @param mode Encoding mode (BUFFER or SURFACE)
     * @return true on success
     */
    bool Create(const char* mimeType, int width, int height, 
                int bitrate, int frameRate, Mode mode = Mode::SURFACE);

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
     * @brief Get input surface for SURFACE mode (zero-copy encoding)
     * @return ANativeWindow* for creating RenderTarget, nullptr if not in SURFACE mode
     */
    ANativeWindow* GetInputSurface();

    /**
     * @brief Create RenderTarget for direct rendering (SURFACE mode only)
     * @return Shared pointer to RenderTarget, nullptr on failure
     */
    std::shared_ptr<RenderTarget> CreateInputRenderTarget();

    /**
     * @brief Encode a frame from buffer (BUFFER mode only)
     * @param data Frame data
     * @param size Data size in bytes
     * @param presentationTimeUs Presentation timestamp in microseconds
     * @return true on success
     */
    bool EncodeFrame(const uint8_t* data, size_t size, int64_t presentationTimeUs);

    /**
     * @brief Signal end of stream
     * @return true on success
     */
    bool SignalEndOfStream();

    /**
     * @brief Get encoded output
     * @param buffer Output buffer pointer (owned by encoder)
     * @param size Output buffer size
     * @param presentationTimeUs Presentation timestamp
     * @param flags Output buffer flags
     * @return true if output available, false if need to try again
     */
    bool GetEncodedData(uint8_t** buffer, size_t* size, 
                       int64_t* presentationTimeUs, uint32_t* flags);

    /**
     * @brief Release output buffer after processing
     * @param index Buffer index from GetEncodedData
     * @return true on success
     */
    bool ReleaseOutputBuffer(size_t index);

    /**
     * @brief Get codec configuration data (SPS/PPS for H.264/H.265)
     * @param buffer Output buffer pointer
     * @param size Output buffer size
     * @return true on success
     */
    bool GetConfigData(uint8_t** buffer, size_t* size);

    // Getters
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    Mode GetMode() const { return m_mode; }
    bool IsRunning() const { return m_isRunning; }

private:
    AMediaCodec* m_codec;
    AMediaFormat* m_format;
    ANativeWindow* m_inputSurface;
    
    Mode m_mode;
    int m_width;
    int m_height;
    int m_bitrate;
    int m_frameRate;
    bool m_isRunning;
    
    ssize_t m_currentOutputIndex;
    AMediaCodecBufferInfo m_outputInfo;
};

} // namespace ului

#endif // __ANDROID__
