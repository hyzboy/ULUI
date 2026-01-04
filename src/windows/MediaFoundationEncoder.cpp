#ifdef _WIN32

#include "windows/MediaFoundationEncoder.h"
#include "Bitmap.h"
#include "BitmapFormat.h"

// Windows Media Foundation headers
#include <windows.h>
#include <mfapi.h>
#include <mftransform.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wmcodecdsp.h>
#include <codecapi.h>

// COM support
#include <comdef.h>
#include <initguid.h>

// Note: Library linking is handled by CMakeLists.txt

namespace ului {

// Constants for time conversions
static const int64_t MICROSECONDS_TO_100NS_UNITS = 10;
static const int64_t SECONDS_TO_100NS_UNITS = 10000000;

// Constants for format calculations
static const float NV12_BYTES_PER_PIXEL_RATIO = 1.5f;

// Media Foundation context holding COM objects
struct MediaFoundationEncoder::MediaFoundationContext {
    IMFTransform* transform;              // The encoder transform
    IMFSample* outputSample;              // Current output sample
    IMFMediaBuffer* outputBuffer;         // Current output buffer
    DWORD outputStreamID;                 // Output stream identifier
    DWORD inputStreamID;                  // Input stream identifier
    bool mfInitialized;                   // Track MF initialization
    bool comInitialized;                  // Track if we initialized COM
    GUID codecSubtype;                    // Current codec subtype GUID
    
    // Config data buffer (SPS/PPS)
    std::vector<uint8_t> configData;
    bool configDataExtracted;

    MediaFoundationContext()
        : transform(nullptr)
        , outputSample(nullptr)
        , outputBuffer(nullptr)
        , outputStreamID(0)
        , inputStreamID(0)
        , mfInitialized(false)
        , comInitialized(false)
        , codecSubtype(MFVideoFormat_H264)
        , configDataExtracted(false)
    {
    }
};

MediaFoundationEncoder::MediaFoundationEncoder()
    : Object("MediaFoundationEncoder")
    , m_context(nullptr)
    , m_mode(Mode::BUFFER)
    , m_width(0)
    , m_height(0)
    , m_bitrate(0)
    , m_frameRate(0)
    , m_keyframeInterval(1)
    , m_isRunning(false)
    , m_hasData(false)
    , m_inputFormat(PixelFormat::NV12)
{
    m_context = new MediaFoundationContext();
}

MediaFoundationEncoder::~MediaFoundationEncoder() {
    Release();
    delete m_context;
}

bool MediaFoundationEncoder::InitializeMediaFoundation() {
    if (m_context->mfInitialized) {
        return true;
    }

    // Initialize COM with MULTITHREADED apartment for better compatibility
    // in multi-threaded applications
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        m_context->comInitialized = true;
    } else if (hr != RPC_E_CHANGED_MODE) {
        LogError("Failed to initialize COM: 0x%08X", hr);
        return false;
    }
    // If RPC_E_CHANGED_MODE, COM was already initialized with different threading model,
    // which is acceptable - we don't need to uninitialize

    // Initialize Media Foundation
    hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(hr)) {
        LogError("Failed to initialize Media Foundation: 0x%08X", hr);
        if (m_context->comInitialized) {
            CoUninitialize();
            m_context->comInitialized = false;
        }
        return false;
    }

    m_context->mfInitialized = true;
    LogInfo("Media Foundation initialized successfully");
    return true;
}

void MediaFoundationEncoder::CleanupMediaFoundation() {
    if (!m_context->mfInitialized) {
        return;
    }

    if (m_context->outputBuffer) {
        m_context->outputBuffer->Release();
        m_context->outputBuffer = nullptr;
    }

    if (m_context->outputSample) {
        m_context->outputSample->Release();
        m_context->outputSample = nullptr;
    }

    if (m_context->transform) {
        m_context->transform->Release();
        m_context->transform = nullptr;
    }

    MFShutdown();
    
    // Only uninitialize COM if we initialized it
    if (m_context->comInitialized) {
        CoUninitialize();
        m_context->comInitialized = false;
    }
    
    m_context->mfInitialized = false;
    LogInfo("Media Foundation cleaned up");
}

const char* MediaFoundationEncoder::MimeTypeToSubtype(const char* mimeType) {
    if (strcmp(mimeType, "video/avc") == 0 || strcmp(mimeType, "video/x-h264") == 0) {
        return "H264";
    } else if (strcmp(mimeType, "video/hevc") == 0 || strcmp(mimeType, "video/x-h265") == 0) {
        return "HEVC";
    } else if (strcmp(mimeType, "video/mp4") == 0 || strcmp(mimeType, "video/mp4v-es") == 0) {
        return "MP4V";
    } else if (strcmp(mimeType, "video/x-ms-wmv") == 0) {
        return "WMV3";
    }
    
    return "H264"; // Default to H.264
}

bool MediaFoundationEncoder::CreateEncoderTransform(const char* mimeType) {
    HRESULT hr;
    
    // Determine the encoder subtype based on MIME type
    const char* subtypeName = MimeTypeToSubtype(mimeType);
    
    // Map subtype name to GUID
    GUID subtypeGuid;
    if (strcmp(subtypeName, "H264") == 0) {
        subtypeGuid = MFVideoFormat_H264;
    } else if (strcmp(subtypeName, "HEVC") == 0) {
        subtypeGuid = MFVideoFormat_HEVC;
    } else if (strcmp(subtypeName, "MP4V") == 0) {
        subtypeGuid = MFVideoFormat_MP4V;
    } else if (strcmp(subtypeName, "WMV3") == 0) {
        subtypeGuid = MFVideoFormat_WMV3;
    } else {
        subtypeGuid = MFVideoFormat_H264; // Default
    }
    
    // Store the codec subtype for later use
    m_context->codecSubtype = subtypeGuid;

    // Create encoder category enumeration
    MFT_REGISTER_TYPE_INFO inputInfo = { MFMediaType_Video, MFVideoFormat_NV12 };
    MFT_REGISTER_TYPE_INFO outputInfo = { MFMediaType_Video, subtypeGuid };
    
    IMFActivate** activates = nullptr;
    UINT32 count = 0;
    
    hr = MFTEnumEx(
        MFT_CATEGORY_VIDEO_ENCODER,
        MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE,
        &inputInfo,
        &outputInfo,
        &activates,
        &count
    );
    
    if (FAILED(hr) || count == 0) {
        LogError("Failed to enumerate video encoders: 0x%08X", hr);
        return false;
    }
    
    // Activate the first encoder
    hr = activates[0]->ActivateObject(IID_PPV_ARGS(&m_context->transform));
    
    // Clean up activates
    for (UINT32 i = 0; i < count; i++) {
        activates[i]->Release();
    }
    CoTaskMemFree(activates);
    
    if (FAILED(hr)) {
        LogError("Failed to activate encoder transform: 0x%08X", hr);
        return false;
    }
    
    LogInfo("Encoder transform created for subtype: %s", subtypeName);
    return true;
}

bool MediaFoundationEncoder::Create(const char* mimeType, int width, int height, 
                                   int bitrate, int frameRate, Mode mode) {
    if (m_context->transform) {
        LogError("Encoder already created");
        return false;
    }

    m_mode = mode;
    m_width = width;
    m_height = height;
    m_bitrate = bitrate;
    m_frameRate = frameRate;

    if (!InitializeMediaFoundation()) {
        return false;
    }

    if (!CreateEncoderTransform(mimeType)) {
        CleanupMediaFoundation();
        return false;
    }

    HRESULT hr;
    IMFMediaType* inputType = nullptr;
    IMFMediaType* outputType = nullptr;

    // Create and set input media type
    hr = MFCreateMediaType(&inputType);
    if (FAILED(hr)) {
        LogError("Failed to create input media type: 0x%08X", hr);
        return false;
    }

    inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    inputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(inputType, MF_MT_FRAME_SIZE, width, height);
    MFSetAttributeRatio(inputType, MF_MT_FRAME_RATE, frameRate, 1);
    MFSetAttributeRatio(inputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    hr = m_context->transform->SetInputType(0, inputType, 0);
    inputType->Release();
    
    if (FAILED(hr)) {
        LogError("Failed to set input type: 0x%08X", hr);
        return false;
    }

    // Create and set output media type
    hr = MFCreateMediaType(&outputType);
    if (FAILED(hr)) {
        LogError("Failed to create output media type: 0x%08X", hr);
        return false;
    }

    outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outputType->SetGUID(MF_MT_SUBTYPE, m_context->codecSubtype);
    outputType->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
    outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(outputType, MF_MT_FRAME_SIZE, width, height);
    MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, frameRate, 1);
    MFSetAttributeRatio(outputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    hr = m_context->transform->SetOutputType(0, outputType, 0);
    outputType->Release();
    
    if (FAILED(hr)) {
        LogError("Failed to set output type: 0x%08X", hr);
        return false;
    }

    // Set encoder properties
    ICodecAPI* codecAPI = nullptr;
    hr = m_context->transform->QueryInterface(IID_PPV_ARGS(&codecAPI));
    if (SUCCEEDED(hr)) {
        // Set bitrate mode to VBR
        VARIANT var;
        var.vt = VT_UI4;
        var.ulVal = eAVEncCommonRateControlMode_Quality;
        codecAPI->SetValue(&CODECAPI_AVEncCommonRateControlMode, &var);

        // Set quality level (for VBR)
        var.ulVal = 70; // 0-100 scale
        codecAPI->SetValue(&CODECAPI_AVEncCommonQuality, &var);

        codecAPI->Release();
    }

    LogInfo("Created encoder: %s, %dx%d @ %d bps, %d fps, mode: %s", 
            mimeType, width, height, bitrate, frameRate,
            mode == Mode::TEXTURE ? "TEXTURE" : "BUFFER");
    
    return true;
}

bool MediaFoundationEncoder::Start() {
    if (!m_context->transform) {
        LogError("Encoder not configured");
        return false;
    }

    if (m_isRunning) {
        LogWarning("Encoder already running");
        return true;
    }

    HRESULT hr = m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    if (FAILED(hr)) {
        LogError("Failed to start encoder: 0x%08X", hr);
        return false;
    }

    hr = m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
    if (FAILED(hr)) {
        LogError("Failed to notify start of stream: 0x%08X", hr);
        return false;
    }

    m_isRunning = true;
    LogInfo("Encoder started");
    return true;
}

bool MediaFoundationEncoder::Stop() {
    if (!m_isRunning) {
        return true;
    }

    if (m_context->transform) {
        m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
        m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
    }

    m_isRunning = false;
    LogInfo("Encoder stopped");
    return true;
}

void MediaFoundationEncoder::Release() {
    Stop();
    CleanupMediaFoundation();
    m_width = 0;
    m_height = 0;
    m_hasData = false;
    m_context->configData.clear();
    m_context->configDataExtracted = false;
}

bool MediaFoundationEncoder::ConvertBitmapToSample(std::shared_ptr<Bitmap> bitmap, void** samplePtr, int64_t timestampUs) {
    if (!bitmap || !bitmap->IsValid()) {
        LogError("Invalid bitmap");
        return false;
    }

    // Ensure bitmap is in NV12 format
    if (bitmap->GetPixelFormat() != PixelFormat::NV12) {
        LogWarning("Bitmap is not in NV12 format, converting...");
        if (!bitmap->ConvertTo(PixelFormat::NV12)) {
            LogError("Failed to convert bitmap to NV12");
            return false;
        }
    }

    HRESULT hr;
    IMFSample* sample = nullptr;
    IMFMediaBuffer* buffer = nullptr;

    // Calculate buffer size for NV12 format (width * height * 1.5)
    size_t bufferSize = (size_t)(m_width * m_height * NV12_BYTES_PER_PIXEL_RATIO);

    // Create media buffer
    hr = MFCreateMemoryBuffer((DWORD)bufferSize, &buffer);
    if (FAILED(hr)) {
        LogError("Failed to create media buffer: 0x%08X", hr);
        return false;
    }

    // Copy bitmap data to buffer
    BYTE* bufferData = nullptr;
    DWORD maxLength = 0;
    hr = buffer->Lock(&bufferData, &maxLength, nullptr);
    if (SUCCEEDED(hr)) {
        void* bitmapData = bitmap->GetData();
        size_t bitmapSize = bitmap->GetFormat().GetDataSize();
        
        // Validate size
        if (bitmapSize != bufferSize) {
            buffer->Unlock();
            buffer->Release();
            LogError("Bitmap size (%zu) does not match expected buffer size (%zu)", bitmapSize, bufferSize);
            return false;
        }
        
        memcpy(bufferData, bitmapData, bitmapSize);
        buffer->Unlock();
        buffer->SetCurrentLength((DWORD)bitmapSize);
    } else {
        buffer->Release();
        LogError("Failed to lock buffer: 0x%08X", hr);
        return false;
    }

    // Create sample
    hr = MFCreateSample(&sample);
    if (FAILED(hr)) {
        buffer->Release();
        LogError("Failed to create sample: 0x%08X", hr);
        return false;
    }

    sample->AddBuffer(buffer);
    buffer->Release();

    // Set sample time
    LONGLONG sampleTime = timestampUs * MICROSECONDS_TO_100NS_UNITS; // Convert to 100-nanosecond units
    sample->SetSampleTime(sampleTime);
    
    // Set sample duration
    LONGLONG duration = (LONGLONG)((1.0 / m_frameRate) * SECONDS_TO_100NS_UNITS); // 100-ns units
    sample->SetSampleDuration(duration);

    *samplePtr = sample;
    return true;
}

bool MediaFoundationEncoder::EncodeFrame(std::shared_ptr<Bitmap> bitmap, int64_t presentationTimeUs, 
                                        bool forceKeyFrame) {
    if (!m_context->transform || !m_isRunning) {
        LogError("Encoder not ready");
        return false;
    }

    if (m_mode != Mode::BUFFER) {
        LogError("EncodeFrame only available in BUFFER mode");
        return false;
    }

    void* samplePtr = nullptr;
    if (!ConvertBitmapToSample(bitmap, &samplePtr, presentationTimeUs)) {
        return false;
    }

    IMFSample* sample = static_cast<IMFSample*>(samplePtr);

    // Set keyframe flag if requested
    if (forceKeyFrame) {
        sample->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
    }

    // Process input
    HRESULT hr = m_context->transform->ProcessInput(0, sample, 0);
    sample->Release();

    if (FAILED(hr)) {
        if (hr == MF_E_NOTACCEPTING) {
            // Transform is not accepting input right now, need to process output first
            LogDebug("Transform not accepting input, need to drain output");
            return false;
        }
        LogError("Failed to process input: 0x%08X", hr);
        return false;
    }

    LogDebug("Encoded frame: timestamp: %lld", presentationTimeUs);
    return true;
}

bool MediaFoundationEncoder::SignalEndOfStream() {
    if (!m_context->transform) {
        return false;
    }

    HRESULT hr = m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
    return SUCCEEDED(hr);
}

bool MediaFoundationEncoder::HasEncodedData() {
    if (!m_context->transform || !m_isRunning) {
        return false;
    }

    // Check if output is available using GetOutputStatus
    DWORD flags = 0;
    HRESULT hr = m_context->transform->GetOutputStatus(&flags);
    
    if (FAILED(hr)) {
        return false;
    }
    
    // MFT_OUTPUT_STATUS_SAMPLE_READY means a sample is available
    return (flags & MFT_OUTPUT_STATUS_SAMPLE_READY) != 0;
}

bool MediaFoundationEncoder::GetEncodedData(uint8_t** buffer, size_t* size, 
                                           int64_t* presentationTimeUs, bool* isKeyFrame) {
    if (!m_context->transform || !m_isRunning) {
        LogError("Encoder not ready");
        return false;
    }

    if (!buffer || !size) {
        LogError("Invalid output parameters");
        return false;
    }

    HRESULT hr;
    MFT_OUTPUT_DATA_BUFFER outputBuffer;
    MFT_OUTPUT_STREAM_INFO streamInfo;

    // Clean up previous output sample if any
    if (m_context->outputSample) {
        m_context->outputSample->Release();
        m_context->outputSample = nullptr;
    }
    if (m_context->outputBuffer) {
        m_context->outputBuffer->Release();
        m_context->outputBuffer = nullptr;
    }

    // Get output stream info
    hr = m_context->transform->GetOutputStreamInfo(0, &streamInfo);
    if (FAILED(hr)) {
        LogError("Failed to get output stream info: 0x%08X", hr);
        return false;
    }

    // Allocate output sample if needed
    IMFSample* sample = nullptr;
    if (!(streamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES)) {
        hr = MFCreateSample(&sample);
        if (FAILED(hr)) {
            LogError("Failed to create output sample: 0x%08X", hr);
            return false;
        }

        IMFMediaBuffer* mediaBuffer = nullptr;
        hr = MFCreateMemoryBuffer(streamInfo.cbSize, &mediaBuffer);
        if (FAILED(hr)) {
            sample->Release();
            LogError("Failed to create output buffer: 0x%08X", hr);
            return false;
        }

        sample->AddBuffer(mediaBuffer);
        mediaBuffer->Release();
    }

    outputBuffer.dwStreamID = 0;
    outputBuffer.pSample = sample;
    outputBuffer.dwStatus = 0;
    outputBuffer.pEvents = nullptr;

    DWORD status = 0;
    hr = m_context->transform->ProcessOutput(0, 1, &outputBuffer, &status);

    if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
        if (sample) sample->Release();
        return false; // Need more input data
    }

    if (FAILED(hr)) {
        if (sample) sample->Release();
        LogError("Failed to process output: 0x%08X", hr);
        return false;
    }

    // Get the output sample
    IMFSample* outputSample = outputBuffer.pSample;
    if (!outputSample) {
        LogError("No output sample available");
        return false;
    }

    // Store sample for later cleanup
    m_context->outputSample = outputSample;

    // Get presentation timestamp
    if (presentationTimeUs) {
        LONGLONG sampleTime = 0;
        if (SUCCEEDED(outputSample->GetSampleTime(&sampleTime))) {
            *presentationTimeUs = sampleTime / MICROSECONDS_TO_100NS_UNITS; // Convert from 100-ns to microseconds
        }
    }

    // Check if this is a keyframe
    if (isKeyFrame) {
        UINT32 cleanPoint = 0;
        outputSample->GetUINT32(MFSampleExtension_CleanPoint, &cleanPoint);
        *isKeyFrame = (cleanPoint == TRUE);
    }

    // Get buffer from sample
    hr = outputSample->ConvertToContiguousBuffer(&m_context->outputBuffer);
    if (FAILED(hr)) {
        LogError("Failed to get contiguous buffer: 0x%08X", hr);
        return false;
    }

    BYTE* data = nullptr;
    DWORD currentLength = 0;
    
    hr = m_context->outputBuffer->Lock(&data, nullptr, &currentLength);
    if (FAILED(hr)) {
        LogError("Failed to lock buffer: 0x%08X", hr);
        return false;
    }

    *buffer = data;
    *size = currentLength;

    // Note: Buffer will be unlocked in next call or cleanup
    // Don't unlock here as caller needs access to buffer

    LogDebug("Got encoded data: %zu bytes, keyframe: %d", currentLength, isKeyFrame ? *isKeyFrame : 0);
    
    // Extract config data (SPS/PPS) from first keyframe if not done yet
    // For H.264/H.265, config data is typically in the first keyframe
    if (!m_context->configDataExtracted && isKeyFrame && *isKeyFrame) {
        // Store this as potential config data
        // In H.264/H.265, this would include SPS/PPS NAL units
        m_context->configData.resize(currentLength);
        memcpy(m_context->configData.data(), data, currentLength);
        m_context->configDataExtracted = true;
        LogInfo("Extracted codec config data: %zu bytes", currentLength);
    }
    
    return true;
}

bool MediaFoundationEncoder::GetConfigData(uint8_t** buffer, size_t* size) {
    if (!buffer || !size) {
        return false;
    }

    if (m_context->configData.empty()) {
        LogWarning("No config data available yet");
        return false;
    }

    *buffer = m_context->configData.data();
    *size = m_context->configData.size();
    return true;
}

bool MediaFoundationEncoder::Flush() {
    if (!m_context->transform) {
        return false;
    }

    HRESULT hr = m_context->transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    if (FAILED(hr)) {
        LogError("Failed to flush encoder: 0x%08X", hr);
        return false;
    }

    m_hasData = false;
    LogInfo("Encoder flushed");
    return true;
}

bool MediaFoundationEncoder::SetInputFormat(PixelFormat format) {
    if (format == PixelFormat::NV12) {
        m_inputFormat = format;
        LogInfo("SetInputFormat: NV12 format (default)");
        return true;
    }
    
    LogWarning("SetInputFormat: Format not directly supported, will convert from input");
    LogInfo("Input bitmaps will be converted to NV12 if needed");
    m_inputFormat = format;
    return true;
}

bool MediaFoundationEncoder::SetKeyframeInterval(int intervalSeconds) {
    if (intervalSeconds <= 0) {
        LogError("Invalid keyframe interval: %d", intervalSeconds);
        return false;
    }
    
    m_keyframeInterval = intervalSeconds;
    LogInfo("Keyframe interval set to %d seconds", intervalSeconds);
    
    // Apply to encoder if already created
    if (m_context->transform) {
        ICodecAPI* codecAPI = nullptr;
        HRESULT hr = m_context->transform->QueryInterface(IID_PPV_ARGS(&codecAPI));
        if (SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_UI4;
            
            // Calculate GOP size with overflow protection
            int64_t gopSize = (int64_t)intervalSeconds * (int64_t)m_frameRate;
            if (gopSize > UINT32_MAX) {
                LogWarning("GOP size too large, clamping to UINT32_MAX");
                gopSize = UINT32_MAX;
            }
            
            var.ulVal = (UINT32)gopSize;
            codecAPI->SetValue(&CODECAPI_AVEncMPVGOPSize, &var);
            codecAPI->Release();
            return true;
        }
    }
    
    return true;
}

} // namespace ului

#endif // _WIN32
