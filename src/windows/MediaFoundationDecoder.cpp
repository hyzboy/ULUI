#ifdef _WIN32

#include "windows/MediaFoundationDecoder.h"
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

// Media Foundation context holding COM objects
struct MediaFoundationDecoder::MediaFoundationContext {
    IMFTransform* transform;              // The decoder transform
    IMFSample* outputSample;              // Current output sample
    IMFMediaBuffer* outputBuffer;         // Current output buffer
    DWORD outputStreamID;                 // Output stream identifier
    DWORD inputStreamID;                  // Input stream identifier
    bool mfInitialized;                   // Track MF initialization
    bool comInitialized;                  // Track if we initialized COM
    GUID codecSubtype;                    // Current codec subtype GUID

    MediaFoundationContext()
        : transform(nullptr)
        , outputSample(nullptr)
        , outputBuffer(nullptr)
        , outputStreamID(0)
        , inputStreamID(0)
        , mfInitialized(false)
        , comInitialized(false)
        , codecSubtype(MFVideoFormat_H264)
    {
    }
};

MediaFoundationDecoder::MediaFoundationDecoder()
    : Object("MediaFoundationDecoder")
    , m_context(nullptr)
    , m_mode(Mode::BUFFER)
    , m_width(0)
    , m_height(0)
    , m_frameRate(0)
    , m_bitrate(0)
    , m_isRunning(false)
    , m_hasFrame(false)
{
    m_context = new MediaFoundationContext();
}

MediaFoundationDecoder::~MediaFoundationDecoder() {
    Release();
    delete m_context;
}

bool MediaFoundationDecoder::InitializeMediaFoundation() {
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

void MediaFoundationDecoder::CleanupMediaFoundation() {
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

const char* MediaFoundationDecoder::MimeTypeToSubtype(const char* mimeType) {
    if (strcmp(mimeType, "video/avc") == 0 || strcmp(mimeType, "video/x-h264") == 0) {
        return "H264";
    } else if (strcmp(mimeType, "video/hevc") == 0 || strcmp(mimeType, "video/x-h265") == 0) {
        return "HEVC";
    } else if (strcmp(mimeType, "video/mp4") == 0 || strcmp(mimeType, "video/mp4v-es") == 0) {
        return "MP4V";
    } else if (strcmp(mimeType, "video/x-ms-wmv") == 0) {
        return "WMV3";
    } else if (strcmp(mimeType, "video/mpeg") == 0) {
        return "MPG1";
    }
    
    return "H264"; // Default to H.264
}

bool MediaFoundationDecoder::CreateDecoderTransform(const char* mimeType) {
    HRESULT hr;
    
    // Determine the decoder subtype based on MIME type
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
    } else if (strcmp(subtypeName, "MPG1") == 0) {
        subtypeGuid = MFVideoFormat_MPG1;
    } else {
        subtypeGuid = MFVideoFormat_H264; // Default
    }
    
    // Store the codec subtype for later use in Configure
    m_context->codecSubtype = subtypeGuid;

    // Create decoder category enumeration
    MFT_REGISTER_TYPE_INFO inputInfo = { MFMediaType_Video, subtypeGuid };
    MFT_REGISTER_TYPE_INFO outputInfo = { MFMediaType_Video, MFVideoFormat_NV12 };
    
    IMFActivate** activates = nullptr;
    UINT32 count = 0;
    
    hr = MFTEnumEx(
        MFT_CATEGORY_VIDEO_DECODER,
        MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE,
        &inputInfo,
        &outputInfo,
        &activates,
        &count
    );
    
    if (FAILED(hr) || count == 0) {
        LogError("Failed to enumerate video decoders: 0x%08X", hr);
        return false;
    }
    
    // Activate the first decoder
    hr = activates[0]->ActivateObject(IID_PPV_ARGS(&m_context->transform));
    
    // Clean up activates
    for (UINT32 i = 0; i < count; i++) {
        activates[i]->Release();
    }
    CoTaskMemFree(activates);
    
    if (FAILED(hr)) {
        LogError("Failed to activate decoder transform: 0x%08X", hr);
        return false;
    }
    
    LogInfo("Decoder transform created for subtype: %s", subtypeName);
    return true;
}

bool MediaFoundationDecoder::Create(const char* mimeType, Mode mode) {
    if (m_context->transform) {
        LogError("Decoder already created");
        return false;
    }

    m_mode = mode;

    if (!InitializeMediaFoundation()) {
        return false;
    }

    if (!CreateDecoderTransform(mimeType)) {
        CleanupMediaFoundation();
        return false;
    }

    LogInfo("Created decoder: %s, mode: %s", 
            mimeType, mode == Mode::TEXTURE ? "TEXTURE" : "BUFFER");
    
    return true;
}

bool MediaFoundationDecoder::Configure(int width, int height, float frameRate, int bitrate) {
    if (!m_context->transform) {
        LogError("Decoder not created");
        return false;
    }

    m_width = width;
    m_height = height;
    m_frameRate = frameRate;
    m_bitrate = bitrate;

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
    inputType->SetGUID(MF_MT_SUBTYPE, m_context->codecSubtype); // Use the codec from Create()
    inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(inputType, MF_MT_FRAME_SIZE, width, height);
    
    if (frameRate > 0) {
        MFSetAttributeRatio(inputType, MF_MT_FRAME_RATE, (UINT32)(frameRate * 1000), 1000);
    }

    hr = m_context->transform->SetInputType(0, inputType, 0);
    inputType->Release();
    
    if (FAILED(hr)) {
        LogError("Failed to set input type: 0x%08X", hr);
        return false;
    }

    // Get and set output media type
    DWORD typeIndex = 0;
    while (SUCCEEDED(m_context->transform->GetOutputAvailableType(0, typeIndex, &outputType))) {
        GUID subtype;
        hr = outputType->GetGUID(MF_MT_SUBTYPE, &subtype);
        
        // Prefer NV12 format for efficiency
        if (SUCCEEDED(hr) && (subtype == MFVideoFormat_NV12 || subtype == MFVideoFormat_RGB32)) {
            hr = m_context->transform->SetOutputType(0, outputType, 0);
            if (SUCCEEDED(hr)) {
                outputType->Release();
                LogInfo("Configured decoder: %dx%d", width, height);
                return true;
            }
        }
        
        outputType->Release();
        typeIndex++;
    }

    LogError("Failed to set output type");
    return false;
}

bool MediaFoundationDecoder::Start() {
    if (!m_context->transform) {
        LogError("Decoder not configured");
        return false;
    }

    if (m_isRunning) {
        LogWarning("Decoder already running");
        return true;
    }

    HRESULT hr = m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    if (FAILED(hr)) {
        LogError("Failed to start decoder: 0x%08X", hr);
        return false;
    }

    hr = m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
    if (FAILED(hr)) {
        LogError("Failed to notify start of stream: 0x%08X", hr);
        return false;
    }

    m_isRunning = true;
    LogInfo("Decoder started");
    return true;
}

bool MediaFoundationDecoder::Stop() {
    if (!m_isRunning) {
        return true;
    }

    if (m_context->transform) {
        m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
        m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
    }

    m_isRunning = false;
    LogInfo("Decoder stopped");
    return true;
}

void MediaFoundationDecoder::Release() {
    Stop();
    CleanupMediaFoundation();
    m_width = 0;
    m_height = 0;
    m_hasFrame = false;
}

bool MediaFoundationDecoder::QueueInputBuffer(const uint8_t* data, size_t size, 
                                             int64_t presentationTimeUs, bool isKeyFrame) {
    if (!m_context->transform || !m_isRunning) {
        LogError("Decoder not ready");
        return false;
    }

    HRESULT hr;
    IMFSample* sample = nullptr;
    IMFMediaBuffer* buffer = nullptr;

    // Create media buffer
    hr = MFCreateMemoryBuffer((DWORD)size, &buffer);
    if (FAILED(hr)) {
        LogError("Failed to create media buffer: 0x%08X", hr);
        return false;
    }

    // Copy data to buffer with bounds validation
    BYTE* bufferData = nullptr;
    DWORD maxLength = 0;
    hr = buffer->Lock(&bufferData, &maxLength, nullptr);
    if (SUCCEEDED(hr)) {
        // Validate buffer size - input data must fit completely
        if (size > maxLength) {
            buffer->Unlock();
            buffer->Release();
            LogError("Input data size (%zu) exceeds buffer capacity (%u)", size, maxLength);
            return false;
        }
        
        memcpy(bufferData, data, size);
        buffer->Unlock();
        buffer->SetCurrentLength((DWORD)size);
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
    LONGLONG sampleTime = presentationTimeUs * 10; // Convert to 100-nanosecond units
    sample->SetSampleTime(sampleTime);
    
    // Set sample duration (estimate)
    if (m_frameRate > 0) {
        LONGLONG duration = (LONGLONG)((1.0 / m_frameRate) * 10000000); // 100-ns units
        sample->SetSampleDuration(duration);
    }

    // Process input
    hr = m_context->transform->ProcessInput(0, sample, 0);
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

    LogDebug("Queued input buffer: %zu bytes, timestamp: %lld", size, presentationTimeUs);
    return true;
}

bool MediaFoundationDecoder::SignalEndOfStream() {
    if (!m_context->transform) {
        return false;
    }

    HRESULT hr = m_context->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
    return SUCCEEDED(hr);
}

bool MediaFoundationDecoder::HasDecodedFrame() {
    if (!m_context->transform || !m_isRunning) {
        return false;
    }

    // Check if output is available using GetOutputStatus
    // This is a non-destructive query that won't consume frames
    DWORD flags = 0;
    HRESULT hr = m_context->transform->GetOutputStatus(&flags);
    
    if (FAILED(hr)) {
        return false;
    }
    
    // MFT_OUTPUT_STATUS_SAMPLE_READY means a sample is available
    return (flags & MFT_OUTPUT_STATUS_SAMPLE_READY) != 0;
}

bool MediaFoundationDecoder::ConvertSampleToBitmap(void* samplePtr, std::shared_ptr<Bitmap> bitmap) {
    IMFSample* sample = static_cast<IMFSample*>(samplePtr);
    if (!sample || !bitmap) {
        return false;
    }

    HRESULT hr;
    IMFMediaBuffer* buffer = nullptr;
    
    hr = sample->ConvertToContiguousBuffer(&buffer);
    if (FAILED(hr)) {
        LogError("Failed to get contiguous buffer: 0x%08X", hr);
        return false;
    }

    BYTE* data = nullptr;
    DWORD maxLength = 0, currentLength = 0;
    
    hr = buffer->Lock(&data, &maxLength, &currentLength);
    if (FAILED(hr)) {
        buffer->Release();
        LogError("Failed to lock buffer: 0x%08X", hr);
        return false;
    }

    // Create bitmap format - assuming NV12 format from decoder
    BitmapFormat format(PixelFormat::NV12, m_width, m_height);
    
    // Check if bitmap needs to be created or recreated
    if (!bitmap->IsValid() || 
        bitmap->GetWidth() != m_width || 
        bitmap->GetHeight() != m_height ||
        bitmap->GetPixelFormat() != PixelFormat::NV12) {
        
        if (!bitmap->Create(format)) {
            buffer->Unlock();
            buffer->Release();
            LogError("Failed to create bitmap");
            return false;
        }
    }

    // Copy data to bitmap with strict size validation
    size_t dataSize = currentLength;
    void* bitmapData = bitmap->GetData();
    
    if (!bitmapData || dataSize == 0) {
        buffer->Unlock();
        buffer->Release();
        LogError("Invalid bitmap data or empty decoder output");
        return false;
    }
    
    // Get expected bitmap size
    size_t expectedSize = format.GetDataSize();
    
    // Validate sizes match - decoder output should match expected bitmap size
    if (dataSize != expectedSize) {
        buffer->Unlock();
        buffer->Release();
        LogError("Decoder output size (%zu) does not match expected bitmap size (%zu)", 
                dataSize, expectedSize);
        return false;
    }
    
    memcpy(bitmapData, data, dataSize);

    buffer->Unlock();
    buffer->Release();
    
    return true;
}

bool MediaFoundationDecoder::GetDecodedBitmap(std::shared_ptr<Bitmap> bitmap, int64_t* presentationTimeUs) {
    if (!m_context->transform || !m_isRunning) {
        LogError("Decoder not ready");
        return false;
    }

    if (m_mode != Mode::BUFFER) {
        LogError("GetDecodedBitmap only available in BUFFER mode");
        return false;
    }

    if (!bitmap) {
        LogError("Invalid bitmap pointer");
        return false;
    }

    HRESULT hr;
    MFT_OUTPUT_DATA_BUFFER outputBuffer;
    MFT_OUTPUT_STREAM_INFO streamInfo;

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

        IMFMediaBuffer* buffer = nullptr;
        hr = MFCreateMemoryBuffer(streamInfo.cbSize, &buffer);
        if (FAILED(hr)) {
            sample->Release();
            LogError("Failed to create output buffer: 0x%08X", hr);
            return false;
        }

        sample->AddBuffer(buffer);
        buffer->Release();
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

    // Get the output sample (either provided by transform or our allocated one)
    IMFSample* outputSample = outputBuffer.pSample;
    if (!outputSample) {
        LogError("No output sample available");
        return false;
    }

    // Get presentation timestamp
    if (presentationTimeUs) {
        LONGLONG sampleTime = 0;
        if (SUCCEEDED(outputSample->GetSampleTime(&sampleTime))) {
            *presentationTimeUs = sampleTime / 10; // Convert from 100-ns to microseconds
        }
    }

    // Convert sample to bitmap
    bool result = ConvertSampleToBitmap(outputSample, bitmap);
    
    outputSample->Release();
    
    if (result) {
        LogDebug("Decoded frame to bitmap: %dx%d", m_width, m_height);
    }
    
    return result;
}

bool MediaFoundationDecoder::Flush() {
    if (!m_context->transform) {
        return false;
    }

    HRESULT hr = m_context->transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    if (FAILED(hr)) {
        LogError("Failed to flush decoder: 0x%08X", hr);
        return false;
    }

    m_hasFrame = false;
    LogInfo("Decoder flushed");
    return true;
}

bool MediaFoundationDecoder::SetOutputFormat(PixelFormat format) {
    // Currently, Media Foundation decoder outputs NV12 by default
    // This method is a placeholder for future format conversion support
    // For now, only NV12 format is supported
    
    if (format == PixelFormat::NV12) {
        LogInfo("SetOutputFormat: NV12 format (default)");
        return true;
    }
    
    LogWarning("SetOutputFormat: Format not supported, using NV12");
    LogInfo("Only NV12 output format is currently supported. Use Bitmap::ConvertTo() for format conversion.");
    return false;
}

} // namespace ului

#endif // _WIN32
