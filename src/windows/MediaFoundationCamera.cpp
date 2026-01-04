#ifdef _WIN32

#include "windows/MediaFoundationCamera.h"
#include "Bitmap.h"
#include "BitmapFormat.h"

// Windows Media Foundation headers
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wmcodecdsp.h>
#include <Mfobjects.h>

// COM support
#include <comdef.h>
#include <initguid.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Note: Library linking is handled by CMakeLists.txt

namespace ului {

// Constants for camera operations
static const int64_t FRAME_TIMEOUT_MS = 5000;

// Media Foundation context holding COM objects
struct MediaFoundationCamera::MediaFoundationContext {
    IMFMediaSource* mediaSource;
    IMFSourceReader* sourceReader;
    IMFActivate** videoDevices;
    UINT32 deviceCount;
    bool mfInitialized;
    bool comInitialized;
    
    // Frame buffer
    std::mutex frameMutex;
    std::shared_ptr<Bitmap> latestFrame;
    int64_t latestTimestamp;
    bool newFrameAvailable;
    
    // Thread synchronization
    std::atomic<bool> threadRunning;
    std::thread* captureThread;

    MediaFoundationContext()
        : mediaSource(nullptr)
        , sourceReader(nullptr)
        , videoDevices(nullptr)
        , deviceCount(0)
        , mfInitialized(false)
        , comInitialized(false)
        , latestTimestamp(0)
        , newFrameAvailable(false)
        , threadRunning(false)
        , captureThread(nullptr)
    {
    }
};

MediaFoundationCamera::MediaFoundationCamera()
    : Object("MediaFoundationCamera")
    , m_context(nullptr)
    , m_isOpen(false)
    , m_isCapturing(false)
    , m_width(0)
    , m_height(0)
    , m_frameRate(0)
    , m_threadRunning(false)
    , m_captureThread(nullptr)
{
    m_context = new MediaFoundationContext();
}

MediaFoundationCamera::~MediaFoundationCamera() {
    Cleanup();
    delete m_context;
}

bool MediaFoundationCamera::InitializeMediaFoundation() {
    if (m_context->mfInitialized) {
        return true;
    }

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        m_context->comInitialized = true;
    } else if (hr != RPC_E_CHANGED_MODE) {
        LogError("Failed to initialize COM: 0x%08X", hr);
        return false;
    }

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

void MediaFoundationCamera::CleanupMediaFoundation() {
    if (!m_context->mfInitialized) {
        return;
    }

    if (m_context->sourceReader) {
        m_context->sourceReader->Release();
        m_context->sourceReader = nullptr;
    }

    if (m_context->mediaSource) {
        m_context->mediaSource->Shutdown();
        m_context->mediaSource->Release();
        m_context->mediaSource = nullptr;
    }

    if (m_context->videoDevices) {
        for (UINT32 i = 0; i < m_context->deviceCount; i++) {
            m_context->videoDevices[i]->Release();
        }
        CoTaskMemFree(m_context->videoDevices);
        m_context->videoDevices = nullptr;
        m_context->deviceCount = 0;
    }

    MFShutdown();
    
    if (m_context->comInitialized) {
        CoUninitialize();
        m_context->comInitialized = false;
    }
    
    m_context->mfInitialized = false;
    LogInfo("Media Foundation cleaned up");
}

bool MediaFoundationCamera::Initialize() {
    if (m_context->mfInitialized) {
        LogWarning("Camera manager already initialized");
        return true;
    }

    if (!InitializeMediaFoundation()) {
        return false;
    }

    LogInfo("Camera manager initialized");
    return true;
}

void MediaFoundationCamera::Cleanup() {
    StopCapture();
    CloseCamera();
    CleanupMediaFoundation();
    m_cameras.clear();
}

CameraFacing MediaFoundationCamera::DetermineCameraFacing(const std::string& name) const {
    // Try to determine camera facing from device name
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    if (lowerName.find("front") != std::string::npos) {
        return CameraFacing::FRONT;
    } else if (lowerName.find("back") != std::string::npos || 
               lowerName.find("rear") != std::string::npos) {
        return CameraFacing::BACK;
    } else if (lowerName.find("external") != std::string::npos || 
               lowerName.find("usb") != std::string::npos) {
        return CameraFacing::EXTERNAL;
    }
    
    return CameraFacing::UNKNOWN;
}

bool MediaFoundationCamera::LoadCameraInfo(void* devicePtr, CameraInfo& info) {
    IMFActivate* device = static_cast<IMFActivate*>(devicePtr);
    HRESULT hr;
    
    // Get device name
    WCHAR* friendlyName = nullptr;
    UINT32 nameLength = 0;
    hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendlyName, &nameLength);
    if (SUCCEEDED(hr)) {
        // Convert wide string to string
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, friendlyName, -1, nullptr, 0, nullptr, nullptr);
        std::string name(sizeNeeded - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, friendlyName, -1, &name[0], sizeNeeded, nullptr, nullptr);
        info.name = name;
        CoTaskMemFree(friendlyName);
    }
    
    // Get symbolic link (use as ID)
    WCHAR* symbolicLink = nullptr;
    UINT32 linkLength = 0;
    hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, 
                                    &symbolicLink, &linkLength);
    if (SUCCEEDED(hr)) {
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, symbolicLink, -1, nullptr, 0, nullptr, nullptr);
        std::string id(sizeNeeded - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, symbolicLink, -1, &id[0], sizeNeeded, nullptr, nullptr);
        info.id = id;
        CoTaskMemFree(symbolicLink);
    }
    
    // Determine camera facing
    info.facing = DetermineCameraFacing(info.name);
    
    // Get supported formats by activating the device temporarily
    IMFMediaSource* tempSource = nullptr;
    hr = device->ActivateObject(IID_PPV_ARGS(&tempSource));
    if (SUCCEEDED(hr)) {
        IMFSourceReader* tempReader = nullptr;
        hr = MFCreateSourceReaderFromMediaSource(tempSource, nullptr, &tempReader);
        if (SUCCEEDED(hr)) {
            // Enumerate formats
            DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM;
            for (DWORD mediaTypeIndex = 0; ; mediaTypeIndex++) {
                IMFMediaType* mediaType = nullptr;
                hr = tempReader->GetNativeMediaType(streamIndex, mediaTypeIndex, &mediaType);
                if (FAILED(hr)) break;
                
                UINT32 width = 0, height = 0;
                MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
                
                UINT32 frameRateNum = 0, frameRateDen = 0;
                MFGetAttributeRatio(mediaType, MF_MT_FRAME_RATE, &frameRateNum, &frameRateDen);
                
                if (width > 0 && height > 0) {
                    CameraFormat format;
                    format.width = width;
                    format.height = height;
                    format.frameRateNumerator = frameRateNum;
                    format.frameRateDenominator = frameRateDen > 0 ? frameRateDen : 1;
                    info.formats.push_back(format);
                }
                
                mediaType->Release();
            }
            
            tempReader->Release();
        }
        
        tempSource->Shutdown();
        tempSource->Release();
    }
    
    // Set capabilities
    info.supportAutoExposure = true;
    info.supportAutoFocus = true;
    info.supportAutoWhiteBalance = true;
    
    return true;
}

bool MediaFoundationCamera::EnumerateCameras() {
    if (!m_context->mfInitialized) {
        LogError("Camera manager not initialized");
        return false;
    }
    
    m_cameras.clear();
    
    // Clean up previous device list
    if (m_context->videoDevices) {
        for (UINT32 i = 0; i < m_context->deviceCount; i++) {
            m_context->videoDevices[i]->Release();
        }
        CoTaskMemFree(m_context->videoDevices);
        m_context->videoDevices = nullptr;
        m_context->deviceCount = 0;
    }
    
    // Enumerate video capture devices
    IMFAttributes* attributes = nullptr;
    HRESULT hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) {
        LogError("Failed to create attributes: 0x%08X", hr);
        return false;
    }
    
    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
                             MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
        attributes->Release();
        LogError("Failed to set source type: 0x%08X", hr);
        return false;
    }
    
    hr = MFEnumDeviceSources(attributes, &m_context->videoDevices, &m_context->deviceCount);
    attributes->Release();
    
    if (FAILED(hr)) {
        LogError("Failed to enumerate devices: 0x%08X", hr);
        return false;
    }
    
    LogInfo("Found %u video capture devices", m_context->deviceCount);
    
    // Load info for each device
    for (UINT32 i = 0; i < m_context->deviceCount; i++) {
        CameraInfo info;
        if (LoadCameraInfo(m_context->videoDevices[i], info)) {
            m_cameras.push_back(info);
            LogInfo("Camera %u: %s, %zu formats", i, info.name.c_str(), info.formats.size());
        }
    }
    
    return !m_cameras.empty();
}

const CameraInfo* MediaFoundationCamera::GetCameraInfo(const std::string& cameraId) const {
    for (const auto& info : m_cameras) {
        if (info.id == cameraId) {
            return &info;
        }
    }
    return nullptr;
}

const CameraInfo* MediaFoundationCamera::GetCurrentCameraInfo() const {
    if (m_currentCameraId.empty()) {
        return nullptr;
    }
    return GetCameraInfo(m_currentCameraId);
}

bool MediaFoundationCamera::CreateSourceReader(const std::string& symbolicLink) {
    HRESULT hr;
    
    // Find the device activate object
    IMFActivate* targetDevice = nullptr;
    for (UINT32 i = 0; i < m_context->deviceCount; i++) {
        WCHAR* link = nullptr;
        UINT32 linkLen = 0;
        hr = m_context->videoDevices[i]->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &link, &linkLen);
        
        if (SUCCEEDED(hr)) {
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, link, -1, nullptr, 0, nullptr, nullptr);
            std::string deviceLink(sizeNeeded - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, link, -1, &deviceLink[0], sizeNeeded, nullptr, nullptr);
            CoTaskMemFree(link);
            
            if (deviceLink == symbolicLink) {
                targetDevice = m_context->videoDevices[i];
                break;
            }
        }
    }
    
    if (!targetDevice) {
        LogError("Camera device not found");
        return false;
    }
    
    // Activate the media source
    hr = targetDevice->ActivateObject(IID_PPV_ARGS(&m_context->mediaSource));
    if (FAILED(hr)) {
        LogError("Failed to activate media source: 0x%08X", hr);
        return false;
    }
    
    // Create source reader
    IMFAttributes* attributes = nullptr;
    MFCreateAttributes(&attributes, 1);
    if (attributes) {
        // Enable hardware transforms for better performance
        attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
        attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    }
    
    hr = MFCreateSourceReaderFromMediaSource(m_context->mediaSource, attributes, 
                                             &m_context->sourceReader);
    if (attributes) {
        attributes->Release();
    }
    
    if (FAILED(hr)) {
        LogError("Failed to create source reader: 0x%08X", hr);
        m_context->mediaSource->Shutdown();
        m_context->mediaSource->Release();
        m_context->mediaSource = nullptr;
        return false;
    }
    
    return true;
}

bool MediaFoundationCamera::OpenCamera(const std::string& cameraId) {
    if (m_isOpen) {
        LogWarning("Camera already open");
        return true;
    }
    
    const CameraInfo* info = GetCameraInfo(cameraId);
    if (!info) {
        LogError("Camera not found: %s", cameraId.c_str());
        return false;
    }
    
    if (!CreateSourceReader(info->id)) {
        return false;
    }
    
    m_currentCameraId = cameraId;
    m_isOpen = true;
    
    LogInfo("Opened camera: %s", info->name.c_str());
    return true;
}

void MediaFoundationCamera::CloseCamera() {
    if (!m_isOpen) {
        return;
    }
    
    StopCapture();
    
    if (m_context->sourceReader) {
        m_context->sourceReader->Release();
        m_context->sourceReader = nullptr;
    }
    
    if (m_context->mediaSource) {
        m_context->mediaSource->Shutdown();
        m_context->mediaSource->Release();
        m_context->mediaSource = nullptr;
    }
    
    m_currentCameraId.clear();
    m_isOpen = false;
    
    LogInfo("Closed camera");
}

bool MediaFoundationCamera::SetCameraFormat(int32_t width, int32_t height, float frameRate) {
    if (!m_context->sourceReader) {
        LogError("Camera not opened");
        return false;
    }
    
    HRESULT hr;
    DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    
    // Find matching format
    for (DWORD mediaTypeIndex = 0; ; mediaTypeIndex++) {
        IMFMediaType* nativeType = nullptr;
        hr = m_context->sourceReader->GetNativeMediaType(streamIndex, mediaTypeIndex, &nativeType);
        if (FAILED(hr)) break;
        
        UINT32 nativeWidth = 0, nativeHeight = 0;
        MFGetAttributeSize(nativeType, MF_MT_FRAME_SIZE, &nativeWidth, &nativeHeight);
        
        if (nativeWidth == (UINT32)width && nativeHeight == (UINT32)height) {
            // Set this as current media type
            hr = m_context->sourceReader->SetCurrentMediaType(streamIndex, nullptr, nativeType);
            nativeType->Release();
            
            if (SUCCEEDED(hr)) {
                m_width = width;
                m_height = height;
                m_frameRate = frameRate;
                LogInfo("Set camera format: %dx%d @ %.1f fps", width, height, frameRate);
                return true;
            } else {
                LogError("Failed to set media type: 0x%08X", hr);
                return false;
            }
        }
        
        nativeType->Release();
    }
    
    LogError("Format not found: %dx%d", width, height);
    return false;
}

bool MediaFoundationCamera::ConfigureSourceReader() {
    if (!m_context->sourceReader) {
        return false;
    }
    
    HRESULT hr;
    DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    
    // Set output media type to RGB32 for easy processing
    IMFMediaType* outputType = nullptr;
    hr = MFCreateMediaType(&outputType);
    if (FAILED(hr)) {
        LogError("Failed to create media type: 0x%08X", hr);
        return false;
    }
    
    outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    
    hr = m_context->sourceReader->SetCurrentMediaType(streamIndex, nullptr, outputType);
    outputType->Release();
    
    if (FAILED(hr)) {
        LogError("Failed to set current media type: 0x%08X", hr);
        return false;
    }
    
    return true;
}

void MediaFoundationCamera::ReadFrameThread() {
    LogInfo("Capture thread started");
    
    DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    
    while (m_context->threadRunning) {
        DWORD streamFlags = 0;
        LONGLONG timestamp = 0;
        IMFSample* sample = nullptr;
        
        HRESULT hr = m_context->sourceReader->ReadSample(
            streamIndex,
            0,  // No control flags
            nullptr,  // Actual stream index (not needed)
            &streamFlags,
            &timestamp,
            &sample
        );
        
        if (FAILED(hr)) {
            LogError("ReadSample failed: 0x%08X", hr);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        if (sample) {
            // Convert sample to bitmap
            IMFMediaBuffer* buffer = nullptr;
            hr = sample->ConvertToContiguousBuffer(&buffer);
            if (SUCCEEDED(hr)) {
                BYTE* data = nullptr;
                DWORD currentLength = 0;
                
                hr = buffer->Lock(&data, nullptr, &currentLength);
                if (SUCCEEDED(hr)) {
                    // Create bitmap
                    auto bitmap = std::make_shared<Bitmap>();
                    BitmapFormat format(PixelFormat::BGRA8, m_width, m_height);
                    
                    if (bitmap->Create(format)) {
                        void* bitmapData = bitmap->GetData();
                        size_t expectedSize = m_width * m_height * 4; // BGRA8
                        
                        if (currentLength >= expectedSize) {
                            memcpy(bitmapData, data, expectedSize);
                            
                            // Store frame
                            {
                                std::lock_guard<std::mutex> lock(m_context->frameMutex);
                                m_context->latestFrame = bitmap;
                                m_context->latestTimestamp = timestamp / 10; // Convert to microseconds
                                m_context->newFrameAvailable = true;
                            }
                            
                            // Call callback if set
                            if (m_frameCallback) {
                                m_frameCallback(bitmap, timestamp / 10);
                            }
                        }
                    }
                    
                    buffer->Unlock();
                }
                
                buffer->Release();
            }
            
            sample->Release();
        }
        
        if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            LogWarning("End of stream reached");
            break;
        }
        
        // Small sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    LogInfo("Capture thread stopped");
}

bool MediaFoundationCamera::StartCapture(int32_t width, int32_t height, float frameRate) {
    if (!m_isOpen) {
        LogError("Camera not opened");
        return false;
    }
    
    if (m_isCapturing) {
        LogWarning("Capture already active");
        return true;
    }
    
    // Set camera format
    if (!SetCameraFormat(width, height, frameRate)) {
        // Try to configure anyway with current settings
        if (!ConfigureSourceReader()) {
            return false;
        }
    } else {
        if (!ConfigureSourceReader()) {
            return false;
        }
    }
    
    // Start capture thread
    m_context->threadRunning = true;
    m_context->captureThread = new std::thread(&MediaFoundationCamera::ReadFrameThread, this);
    
    m_isCapturing = true;
    LogInfo("Started capture: %dx%d @ %.1f fps", m_width, m_height, m_frameRate);
    return true;
}

void MediaFoundationCamera::StopCapture() {
    if (!m_isCapturing) {
        return;
    }
    
    // Stop capture thread
    m_context->threadRunning = false;
    if (m_context->captureThread) {
        if (m_context->captureThread->joinable()) {
            m_context->captureThread->join();
        }
        delete m_context->captureThread;
        m_context->captureThread = nullptr;
    }
    
    m_isCapturing = false;
    LogInfo("Stopped capture");
}

bool MediaFoundationCamera::GetFrame(std::shared_ptr<Bitmap> bitmap, int64_t* timestampUs) {
    if (!m_isCapturing) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_context->frameMutex);
    
    if (!m_context->newFrameAvailable || !m_context->latestFrame) {
        return false;
    }
    
    // Copy frame data
    if (bitmap) {
        // Ensure bitmap is correct size and format
        if (!bitmap->IsValid() || 
            bitmap->GetWidth() != m_context->latestFrame->GetWidth() ||
            bitmap->GetHeight() != m_context->latestFrame->GetHeight()) {
            
            BitmapFormat format = m_context->latestFrame->GetFormat();
            if (!bitmap->Create(format)) {
                LogError("Failed to create bitmap");
                return false;
            }
        }
        
        // Copy pixel data
        void* srcData = m_context->latestFrame->GetData();
        void* dstData = bitmap->GetData();
        size_t dataSize = m_context->latestFrame->GetFormat().GetDataSize();
        memcpy(dstData, srcData, dataSize);
    }
    
    if (timestampUs) {
        *timestampUs = m_context->latestTimestamp;
    }
    
    m_context->newFrameAvailable = false;
    return true;
}

bool MediaFoundationCamera::SetCameraParams(const CameraParams& params) {
    // Windows Media Foundation camera controls are accessed via IAMCameraControl
    // and IAMVideoProcAmp interfaces. Implementation would require additional COM queries.
    // For now, store the parameters
    m_currentParams = params;
    
    LogInfo("Camera parameters updated");
    return true;
}

bool MediaFoundationCamera::ApplyCameraParams() {
    // This would apply the stored parameters to the actual camera hardware
    // Requires IAMCameraControl and IAMVideoProcAmp interfaces
    return true;
}

} // namespace ului

#endif // _WIN32
