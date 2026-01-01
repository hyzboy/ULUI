#ifdef __ANDROID__

#include "android/MediaCodecDecoder.h"
#include "Bitmap.h"
#include "gl/Texture2D.h"
#include <android/native_window_jni.h>

namespace ului {

MediaCodecDecoder::MediaCodecDecoder()
    : Object("MediaCodecDecoder")
    , m_codec(nullptr)
    , m_format(nullptr)
    , m_outputSurface(nullptr)
    , m_outputTexture(nullptr)
    , m_mode(Mode::BUFFER)
    , m_width(0)
    , m_height(0)
    , m_isRunning(false)
    , m_currentOutputIndex(-1)
{
}

MediaCodecDecoder::~MediaCodecDecoder() {
    Release();
}

bool MediaCodecDecoder::Create(const char* mimeType, Mode mode) {
    if (m_codec) {
        LogError("Decoder already created");
        return false;
    }

    m_mode = mode;

    // Create codec by MIME type
    m_codec = AMediaCodec_createDecoderByType(mimeType);
    if (!m_codec) {
        LogError("Failed to create decoder for MIME type: %s", mimeType);
        return false;
    }

    LogInfo("Created decoder: %s, mode: %s", 
            mimeType, mode == Mode::SURFACE ? "SURFACE" : "BUFFER");
    
    return true;
}

bool MediaCodecDecoder::Configure(int width, int height, 
                                 const uint8_t* csd0, size_t csd0Size,
                                 const uint8_t* csd1, size_t csd1Size) {
    if (!m_codec) {
        LogError("Decoder not created");
        return false;
    }

    m_width = width;
    m_height = height;

    // Create format
    m_format = AMediaFormat_new();
    
    // Get MIME type from codec
    const char* mimeType = "video/avc"; // Default to H.264
    AMediaFormat_setString(m_format, AMEDIAFORMAT_KEY_MIME, mimeType);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_HEIGHT, height);

    // Set codec specific data if provided
    if (csd0 && csd0Size > 0) {
        AMediaFormat_setBuffer(m_format, "csd-0", csd0, csd0Size);
    }
    if (csd1 && csd1Size > 0) {
        AMediaFormat_setBuffer(m_format, "csd-1", csd1, csd1Size);
    }

    // Configure codec with output surface if in SURFACE mode
    ANativeWindow* surface = (m_mode == Mode::SURFACE) ? m_outputSurface : nullptr;
    media_status_t status = AMediaCodec_configure(m_codec, m_format, surface, nullptr, 0);
    if (status != AMEDIA_OK) {
        LogError("Failed to configure decoder: %d", status);
        return false;
    }

    LogInfo("Configured decoder: %dx%d", width, height);
    return true;
}

bool MediaCodecDecoder::SetOutputSurface(ANativeWindow* window) {
    if (m_mode != Mode::SURFACE) {
        LogError("SetOutputSurface() only available in SURFACE mode");
        return false;
    }

    if (m_isRunning) {
        LogError("Cannot set output surface while decoder is running");
        return false;
    }

    m_outputSurface = window;
    LogInfo("Set output surface: %p", window);
    return true;
}

bool MediaCodecDecoder::SetOutputTexture(std::shared_ptr<Texture2D> texture) {
    if (m_mode != Mode::SURFACE) {
        LogError("SetOutputTexture() only available in SURFACE mode");
        return false;
    }

    if (m_isRunning) {
        LogError("Cannot set output texture while decoder is running");
        return false;
    }

    if (!texture || !texture->IsExternalOES()) {
        LogError("Output texture must be OES EXTERNAL texture");
        return false;
    }

    m_outputTexture = texture;
    
    // TODO: Create SurfaceTexture from texture ID and get ANativeWindow
    // This requires JNI calls to Java SurfaceTexture class
    // For now, user must provide ANativeWindow via SetOutputSurface
    
    LogInfo("Set output texture (OES EXTERNAL)");
    return true;
}

bool MediaCodecDecoder::Start() {
    if (!m_codec) {
        LogError("Decoder not created");
        return false;
    }

    if (m_isRunning) {
        LogWarning("Decoder already running");
        return true;
    }

    media_status_t status = AMediaCodec_start(m_codec);
    if (status != AMEDIA_OK) {
        LogError("Failed to start decoder: %d", status);
        return false;
    }

    m_isRunning = true;
    LogInfo("Decoder started");
    return true;
}

bool MediaCodecDecoder::Stop() {
    if (!m_codec) {
        return false;
    }

    if (!m_isRunning) {
        return true;
    }

    media_status_t status = AMediaCodec_stop(m_codec);
    if (status != AMEDIA_OK) {
        LogError("Failed to stop decoder: %d", status);
        return false;
    }

    m_isRunning = false;
    LogInfo("Decoder stopped");
    return true;
}

void MediaCodecDecoder::Release() {
    if (m_isRunning) {
        Stop();
    }

    if (m_outputSurface) {
        // Don't release - we don't own it
        m_outputSurface = nullptr;
    }

    m_outputTexture = nullptr;

    if (m_codec) {
        AMediaCodec_delete(m_codec);
        m_codec = nullptr;
    }

    if (m_format) {
        AMediaFormat_delete(m_format);
        m_format = nullptr;
    }

    LogInfo("Decoder released");
}

bool MediaCodecDecoder::QueueInputBuffer(const uint8_t* data, size_t size, 
                                        int64_t presentationTimeUs, uint32_t flags) {
    if (!m_codec || !m_isRunning) {
        LogError("Decoder not running");
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
                                                        presentationTimeUs, flags);
    if (status != AMEDIA_OK) {
        LogError("Failed to queue input buffer: %d", status);
        return false;
    }

    return true;
}

bool MediaCodecDecoder::SignalEndOfStream() {
    if (!m_codec || !m_isRunning) {
        return false;
    }

    // Queue empty buffer with EOS flag
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

    LogInfo("Signaled end of stream");
    return true;
}

bool MediaCodecDecoder::GetDecodedData(uint8_t** buffer, size_t* size, 
                                      int64_t* presentationTimeUs, uint32_t* flags) {
    if (!m_codec || !m_isRunning) {
        return false;
    }

    if (m_mode != Mode::BUFFER) {
        LogError("GetDecodedData() only available in BUFFER mode. Use RenderFrame() for SURFACE mode.");
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

bool MediaCodecDecoder::ReleaseOutputBuffer(size_t index, bool render) {
    if (!m_codec) {
        return false;
    }

    media_status_t status = AMediaCodec_releaseOutputBuffer(m_codec, index, render);
    if (status != AMEDIA_OK) {
        LogError("Failed to release output buffer: %d", status);
        return false;
    }

    return true;
}

bool MediaCodecDecoder::RenderFrame() {
    if (!m_codec || !m_isRunning) {
        return false;
    }

    if (m_mode != Mode::SURFACE) {
        LogError("RenderFrame() only available in SURFACE mode");
        return false;
    }

    // Dequeue output buffer
    AMediaCodecBufferInfo info;
    ssize_t bufferIndex = AMediaCodec_dequeueOutputBuffer(m_codec, &info, 0);

    if (bufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
        return false; // No frame available
    }

    if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED ||
        bufferIndex == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        return false;
    }

    if (bufferIndex < 0) {
        return false;
    }

    // Release buffer with render=true to output to surface
    media_status_t status = AMediaCodec_releaseOutputBuffer(m_codec, bufferIndex, true);
    if (status != AMEDIA_OK) {
        LogError("Failed to render frame: %d", status);
        return false;
    }

    return true;
}

bool MediaCodecDecoder::GetDecodedBitmap(std::shared_ptr<Bitmap> bitmap) {
    if (m_mode != Mode::BUFFER) {
        LogError("GetDecodedBitmap() only available in BUFFER mode");
        return false;
    }

    uint8_t* buffer;
    size_t size;
    int64_t pts;
    uint32_t flags;

    if (!GetDecodedData(&buffer, &size, &pts, &flags)) {
        return false;
    }

    // TODO: Wrap buffer data into Bitmap
    // Need to determine format from decoder output format
    // Typically NV12 or YUV420P
    
    ReleaseOutputBuffer(m_currentOutputIndex, false);
    
    LogWarning("GetDecodedBitmap() not fully implemented yet");
    return false;
}

} // namespace ului

#endif // __ANDROID__
