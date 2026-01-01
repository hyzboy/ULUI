#ifdef __ANDROID__

#include "android/MediaCodecEncoder.h"
#include "gl/RenderTarget.h"
#include <EGL/egl.h>

namespace ului {

MediaCodecEncoder::MediaCodecEncoder()
    : Object("MediaCodecEncoder")
    , m_codec(nullptr)
    , m_format(nullptr)
    , m_inputSurface(nullptr)
    , m_mode(Mode::BUFFER)
    , m_width(0)
    , m_height(0)
    , m_bitrate(0)
    , m_frameRate(0)
    , m_isRunning(false)
    , m_currentOutputIndex(-1)
{
}

MediaCodecEncoder::~MediaCodecEncoder() {
    Release();
}

bool MediaCodecEncoder::Create(const char* mimeType, int width, int height, 
                               int bitrate, int frameRate, Mode mode) {
    if (m_codec) {
        LogError("Encoder already created");
        return false;
    }

    m_width = width;
    m_height = height;
    m_bitrate = bitrate;
    m_frameRate = frameRate;
    m_mode = mode;

    // Create codec by MIME type
    m_codec = AMediaCodec_createEncoderByType(mimeType);
    if (!m_codec) {
        LogError("Failed to create encoder for MIME type: %s", mimeType);
        return false;
    }

    // Create format
    m_format = AMediaFormat_new();
    AMediaFormat_setString(m_format, AMEDIAFORMAT_KEY_MIME, mimeType);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_HEIGHT, height);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_BIT_RATE, bitrate);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_FRAME_RATE, frameRate);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1); // 1 second keyframe interval
    
    // Color format depends on mode
    if (mode == Mode::SURFACE) {
        // Surface mode - color format is handled by surface
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 0x7f000789); // COLOR_FormatSurface
    } else {
        // Buffer mode - use YUV420 flexible
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 0x7f420888); // COLOR_FormatYUV420Flexible
    }

    // Configure codec
    media_status_t status = AMediaCodec_configure(m_codec, m_format, nullptr, nullptr, 
                                                  AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    if (status != AMEDIA_OK) {
        LogError("Failed to configure encoder: %d", status);
        Release();
        return false;
    }

    // Create input surface for SURFACE mode
    if (mode == Mode::SURFACE) {
        status = AMediaCodec_createInputSurface(m_codec, &m_inputSurface);
        if (status != AMEDIA_OK || !m_inputSurface) {
            LogError("Failed to create input surface: %d", status);
            Release();
            return false;
        }
        LogInfo("Created input surface: %p", m_inputSurface);
    }

    LogInfo("Created encoder: %s, %dx%d @ %d bps, %d fps, mode: %s", 
            mimeType, width, height, bitrate, frameRate,
            mode == Mode::SURFACE ? "SURFACE" : "BUFFER");
    
    return true;
}

bool MediaCodecEncoder::Start() {
    if (!m_codec) {
        LogError("Encoder not created");
        return false;
    }

    if (m_isRunning) {
        LogWarning("Encoder already running");
        return true;
    }

    media_status_t status = AMediaCodec_start(m_codec);
    if (status != AMEDIA_OK) {
        LogError("Failed to start encoder: %d", status);
        return false;
    }

    m_isRunning = true;
    LogInfo("Encoder started");
    return true;
}

bool MediaCodecEncoder::Stop() {
    if (!m_codec) {
        return false;
    }

    if (!m_isRunning) {
        return true;
    }

    media_status_t status = AMediaCodec_stop(m_codec);
    if (status != AMEDIA_OK) {
        LogError("Failed to stop encoder: %d", status);
        return false;
    }

    m_isRunning = false;
    LogInfo("Encoder stopped");
    return true;
}

void MediaCodecEncoder::Release() {
    if (m_isRunning) {
        Stop();
    }

    if (m_inputSurface) {
        ANativeWindow_release(m_inputSurface);
        m_inputSurface = nullptr;
    }

    if (m_codec) {
        AMediaCodec_delete(m_codec);
        m_codec = nullptr;
    }

    if (m_format) {
        AMediaFormat_delete(m_format);
        m_format = nullptr;
    }

    LogInfo("Encoder released");
}

ANativeWindow* MediaCodecEncoder::GetInputSurface() {
    if (m_mode != Mode::SURFACE) {
        LogError("GetInputSurface() only available in SURFACE mode");
        return nullptr;
    }
    return m_inputSurface;
}

std::shared_ptr<RenderTarget> MediaCodecEncoder::CreateInputRenderTarget() {
    ANativeWindow* surface = GetInputSurface();
    if (!surface) {
        return nullptr;
    }

    // Create RenderTarget from native window
    auto renderTarget = std::make_shared<RenderTarget>(surface, m_width, m_height);
    if (!renderTarget->Initialize()) {
        LogError("Failed to initialize RenderTarget from input surface");
        return nullptr;
    }

    LogInfo("Created input RenderTarget for encoder");
    return renderTarget;
}

bool MediaCodecEncoder::EncodeFrame(const uint8_t* data, size_t size, int64_t presentationTimeUs) {
    if (!m_codec || !m_isRunning) {
        LogError("Encoder not running");
        return false;
    }

    if (m_mode != Mode::BUFFER) {
        LogError("EncodeFrame() only available in BUFFER mode. Use SURFACE mode for zero-copy.");
        return false;
    }

    // Get input buffer
    ssize_t bufferIndex = AMediaCodec_dequeueInputBuffer(m_codec, 10000); // 10ms timeout
    if (bufferIndex < 0) {
        LogWarning("No input buffer available: %zd", bufferIndex);
        return false;
    }

    size_t bufferSize;
    uint8_t* buffer = AMediaCodec_getInputBuffer(m_codec, bufferIndex, &bufferSize);
    if (!buffer) {
        LogError("Failed to get input buffer");
        return false;
    }

    if (size > bufferSize) {
        LogError("Input data size %zu exceeds buffer size %zu", size, bufferSize);
        return false;
    }

    // Copy data to input buffer
    memcpy(buffer, data, size);

    // Queue input buffer
    media_status_t status = AMediaCodec_queueInputBuffer(m_codec, bufferIndex, 0, size, 
                                                        presentationTimeUs, 0);
    if (status != AMEDIA_OK) {
        LogError("Failed to queue input buffer: %d", status);
        return false;
    }

    return true;
}

bool MediaCodecEncoder::SignalEndOfStream() {
    if (!m_codec || !m_isRunning) {
        return false;
    }

    if (m_mode == Mode::SURFACE) {
        // For surface mode, signal EOS via surface
        media_status_t status = AMediaCodec_signalEndOfInputStream(m_codec);
        if (status != AMEDIA_OK) {
            LogError("Failed to signal end of stream: %d", status);
            return false;
        }
    } else {
        // For buffer mode, queue empty buffer with EOS flag
        ssize_t bufferIndex = AMediaCodec_dequeueInputBuffer(m_codec, 10000);
        if (bufferIndex < 0) {
            LogError("Failed to get input buffer for EOS");
            return false;
        }

        media_status_t status = AMediaCodec_queueInputBuffer(m_codec, bufferIndex, 0, 0, 0,
                                                            AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
        if (status != AMEDIA_OK) {
            LogError("Failed to queue EOS buffer: %d", status);
            return false;
        }
    }

    LogInfo("Signaled end of stream");
    return true;
}

bool MediaCodecEncoder::GetEncodedData(uint8_t** buffer, size_t* size, 
                                      int64_t* presentationTimeUs, uint32_t* flags) {
    if (!m_codec || !m_isRunning) {
        return false;
    }

    // Dequeue output buffer
    AMediaCodecBufferInfo info;
    ssize_t bufferIndex = AMediaCodec_dequeueOutputBuffer(m_codec, &info, 0); // Non-blocking

    if (bufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
        return false; // No output available yet
    }

    if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        AMediaFormat* format = AMediaCodec_getOutputFormat(m_codec);
        LogInfo("Output format changed");
        AMediaFormat_delete(format);
        return false;
    }

    if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        LogInfo("Output buffers changed");
        return false;
    }

    if (bufferIndex < 0) {
        LogError("Unexpected buffer index: %zd", bufferIndex);
        return false;
    }

    // Get output buffer
    size_t bufferSize;
    uint8_t* outputBuffer = AMediaCodec_getOutputBuffer(m_codec, bufferIndex, &bufferSize);
    if (!outputBuffer) {
        LogError("Failed to get output buffer");
        AMediaCodec_releaseOutputBuffer(m_codec, bufferIndex, false);
        return false;
    }

    // Return buffer info
    *buffer = outputBuffer + info.offset;
    *size = info.size;
    *presentationTimeUs = info.presentationTimeUs;
    *flags = info.flags;

    m_currentOutputIndex = bufferIndex;
    m_outputInfo = info;

    return true;
}

bool MediaCodecEncoder::ReleaseOutputBuffer(size_t index) {
    if (!m_codec) {
        return false;
    }

    media_status_t status = AMediaCodec_releaseOutputBuffer(m_codec, index, false);
    if (status != AMEDIA_OK) {
        LogError("Failed to release output buffer: %d", status);
        return false;
    }

    return true;
}

bool MediaCodecEncoder::GetConfigData(uint8_t** buffer, size_t* size) {
    if (!m_codec) {
        return false;
    }

    AMediaFormat* format = AMediaCodec_getOutputFormat(m_codec);
    if (!format) {
        return false;
    }

    void* data;
    size_t dataSize;
    
    // Try to get CSD-0 (codec specific data - SPS/PPS for H.264)
    if (AMediaFormat_getBuffer(format, "csd-0", &data, &dataSize)) {
        *buffer = static_cast<uint8_t*>(data);
        *size = dataSize;
        AMediaFormat_delete(format);
        return true;
    }

    AMediaFormat_delete(format);
    return false;
}

} // namespace ului

#endif // __ANDROID__
