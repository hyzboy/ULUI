#ifdef __ANDROID__

#include "android/Camera2Manager.h"
#include <camera/NdkCameraError.h>
#include <android/log.h>

namespace ului {
namespace android {

Camera2Manager::Camera2Manager()
    : Object("Camera2Manager")
    , cameraManager_(nullptr)
    , cameraDevice_(nullptr)
    , captureSession_(nullptr)
    , captureRequest_(nullptr)
    , outputTarget_(nullptr)
    , sessionOutput_(nullptr)
    , outputContainer_(nullptr)
    , previewWindow_(nullptr)
    , frameCallback_(nullptr)
{
}

Camera2Manager::~Camera2Manager() {
    Cleanup();
}

bool Camera2Manager::Initialize() {
    if (cameraManager_) {
        LogWarning("Camera manager already initialized");
        return true;
    }
    
    cameraManager_ = ACameraManager_create();
    if (!cameraManager_) {
        LogError("Failed to create camera manager");
        return false;
    }
    
    LogInfo("Camera manager initialized");
    return true;
}

void Camera2Manager::Cleanup() {
    CloseCamera();
    
    if (cameraManager_) {
        ACameraManager_delete(cameraManager_);
        cameraManager_ = nullptr;
    }
    
    cameras_.clear();
}

bool Camera2Manager::EnumerateCameras() {
    if (!cameraManager_) {
        LogError("Camera manager not initialized");
        return false;
    }
    
    cameras_.clear();
    
    ACameraIdList* cameraIdList = nullptr;
    camera_status_t status = ACameraManager_getCameraIdList(cameraManager_, &cameraIdList);
    
    if (status != ACAMERA_OK || !cameraIdList) {
        LogError("Failed to get camera ID list");
        return false;
    }
    
    for (int i = 0; i < cameraIdList->numCameras; i++) {
        CameraInfo info;
        if (LoadCameraInfo(cameraIdList->cameraIds[i], info)) {
            cameras_.push_back(info);
            LogInfo("Found camera: %s, facing: %d, formats: %d", 
                   info.id.c_str(), (int)info.facing, (int)info.formats.size());
        }
    }
    
    ACameraManager_deleteCameraIdList(cameraIdList);
    
    LogInfo("Enumerated %d cameras", (int)cameras_.size());
    return !cameras_.empty();
}

const CameraInfo* Camera2Manager::GetCameraInfo(const std::string& cameraId) const {
    for (const auto& info : cameras_) {
        if (info.id == cameraId) {
            return &info;
        }
    }
    return nullptr;
}

const CameraInfo* Camera2Manager::GetCurrentCameraInfo() const {
    if (currentCameraId_.empty()) {
        return nullptr;
    }
    return GetCameraInfo(currentCameraId_);
}

bool Camera2Manager::LoadCameraInfo(const char* cameraId, CameraInfo& info) {
    ACameraMetadata* metadata = nullptr;
    camera_status_t status = ACameraManager_getCameraCharacteristics(
        cameraManager_, cameraId, &metadata);
    
    if (status != ACAMERA_OK || !metadata) {
        LogError("Failed to get camera characteristics for camera %s", cameraId);
        return false;
    }
    
    info.id = cameraId;
    ParseCameraCharacteristics(metadata, info);
    
    ACameraMetadata_free(metadata);
    return true;
}

void Camera2Manager::ParseCameraCharacteristics(ACameraMetadata* metadata, CameraInfo& info) {
    // Get camera facing
    ACameraMetadata_const_entry entry;
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &entry) == ACAMERA_OK) {
        if (entry.data.u8[0] == ACAMERA_LENS_FACING_FRONT) {
            info.facing = CameraFacing::FRONT;
        } else if (entry.data.u8[0] == ACAMERA_LENS_FACING_BACK) {
            info.facing = CameraFacing::BACK;
        } else {
            info.facing = CameraFacing::EXTERNAL;
        }
    }
    
    // Get sensor orientation
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_ORIENTATION, &entry) == ACAMERA_OK) {
        info.sensorOrientation = entry.data.i32[0];
    }
    
    // Get capabilities
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_REQUEST_AVAILABLE_CAPABILITIES, &entry) == ACAMERA_OK) {
        for (uint32_t i = 0; i < entry.count; i++) {
            switch (entry.data.u8[i]) {
                case ACAMERA_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE:
                    info.capabilities.push_back(CameraCapability::BACKWARD_COMPATIBLE);
                    break;
                case ACAMERA_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_SENSOR:
                    info.capabilities.push_back(CameraCapability::MANUAL_SENSOR);
                    break;
                case ACAMERA_REQUEST_AVAILABLE_CAPABILITIES_RAW:
                    info.capabilities.push_back(CameraCapability::RAW);
                    break;
                // Add more capabilities as needed
            }
        }
    }
    
    // Get supported formats and sizes
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry) == ACAMERA_OK) {
        // Each configuration is 4 int32_t values: format, width, height, input/output
        for (uint32_t i = 0; i + 3 < entry.count; i += 4) {
            int32_t format = entry.data.i32[i];
            int32_t width = entry.data.i32[i + 1];
            int32_t height = entry.data.i32[i + 2];
            int32_t inputOutput = entry.data.i32[i + 3];
            
            // Only output streams
            if (inputOutput == ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT) {
                CameraFormat fmt;
                fmt.width = width;
                fmt.height = height;
                fmt.format = format;
                fmt.minFrameDuration = 0;
                fmt.stallDuration = 0;
                info.formats.push_back(fmt);
            }
        }
    }
    
    // Get frame durations
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_MIN_FRAME_DURATIONS, &entry) == ACAMERA_OK) {
        for (uint32_t i = 0; i + 3 < entry.count; i += 4) {
            int32_t format = entry.data.i32[i];
            int32_t width = entry.data.i32[i + 1];
            int32_t height = entry.data.i32[i + 2];
            int64_t duration = entry.data.i64[i + 3];
            
            for (auto& fmt : info.formats) {
                if (fmt.format == format && fmt.width == width && fmt.height == height) {
                    fmt.minFrameDuration = duration;
                    break;
                }
            }
        }
    }
    
    // Check HDR support
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_CONTROL_AVAILABLE_SCENE_MODES, &entry) == ACAMERA_OK) {
        for (uint32_t i = 0; i < entry.count; i++) {
            if (entry.data.u8[i] == ACAMERA_CONTROL_SCENE_MODE_HDR) {
                info.supportHDR = true;
                break;
            }
        }
    }
    
    // Check OIS support
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION, &entry) == ACAMERA_OK) {
        info.supportOIS = (entry.count > 0);
    }
    
    // Check AF support
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_CONTROL_AF_AVAILABLE_MODES, &entry) == ACAMERA_OK) {
        info.supportAF = (entry.count > 1);  // More than just OFF mode
    }
    
    // Check AE support
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_CONTROL_AE_AVAILABLE_MODES, &entry) == ACAMERA_OK) {
        info.supportAE = (entry.count > 1);
    }
    
    // Check AWB support
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_CONTROL_AWB_AVAILABLE_MODES, &entry) == ACAMERA_OK) {
        info.supportAWB = (entry.count > 1);
    }
    
    // Get sensor info
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_INFO_SENSITIVITY_RANGE, &entry) == ACAMERA_OK && entry.count >= 2) {
        info.maxAnalogSensitivity = entry.data.i32[1];
    }
    
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM, &entry) == ACAMERA_OK) {
        info.maxDigitalZoom = (int32_t)entry.data.f[0];
    }
    
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, &entry) == ACAMERA_OK) {
        info.focalLength = entry.data.f[0];
    }
    
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_INFO_AVAILABLE_APERTURES, &entry) == ACAMERA_OK) {
        info.aperture = entry.data.f[0];
    }
}

bool Camera2Manager::OpenCamera(const std::string& cameraId) {
    if (cameraDevice_) {
        LogWarning("Camera already open, closing first");
        CloseCamera();
    }
    
    if (!cameraManager_) {
        LogError("Camera manager not initialized");
        return false;
    }
    
    ACameraDevice_StateCallbacks deviceCallbacks = {
        .context = this,
        .onDisconnected = OnDeviceDisconnected,
        .onError = OnDeviceError,
    };
    
    camera_status_t status = ACameraManager_openCamera(
        cameraManager_, cameraId.c_str(), &deviceCallbacks, &cameraDevice_);
    
    if (status != ACAMERA_OK || !cameraDevice_) {
        LogError("Failed to open camera %s, error: %d", cameraId.c_str(), status);
        return false;
    }
    
    currentCameraId_ = cameraId;
    LogInfo("Opened camera: %s", cameraId.c_str());
    return true;
}

void Camera2Manager::CloseCamera() {
    StopPreview();
    
    if (cameraDevice_) {
        ACameraDevice_close(cameraDevice_);
        cameraDevice_ = nullptr;
        currentCameraId_.clear();
        LogInfo("Closed camera");
    }
}

std::shared_ptr<Texture2D> Camera2Manager::CreateCameraTexture() {
    auto texture = std::make_shared<Texture2D>(TextureType::TEXTURE_EXTERNAL_OES);
    if (!texture->Create()) {
        LogError("Failed to create camera texture");
        return nullptr;
    }
    return texture;
}

bool Camera2Manager::StartPreview(std::shared_ptr<Texture2D> texture, int32_t width, int32_t height) {
    if (!cameraDevice_) {
        LogError("Camera not open");
        return false;
    }
    
    if (!texture || !texture->IsExternalOES()) {
        LogError("Invalid texture or not OES EXTERNAL texture");
        return false;
    }
    
    StopPreview();
    
    previewTexture_ = texture;
    previewTexture_->SetExternalTextureSize(width, height);
    
    // Create ANativeWindow from texture (via JNI SurfaceTexture)
    // Note: This requires Java/JNI integration to create SurfaceTexture
    // from the OpenGL texture ID and get the ANativeWindow
    // For now, this is a placeholder - actual implementation needs JNI
    LogWarning("StartPreview requires JNI integration to create SurfaceTexture");
    
    // TODO: Implement JNI bridge to:
    // 1. Create android.graphics.SurfaceTexture from texture->GetHandle()
    // 2. Create android.view.Surface from SurfaceTexture
    // 3. Get ANativeWindow* from Surface
    
    if (!previewWindow_) {
        LogError("Failed to get preview window");
        return false;
    }
    
    if (!CreateCaptureSession()) {
        LogError("Failed to create capture session");
        return false;
    }
    
    LogInfo("Started preview: %dx%d", width, height);
    return true;
}

void Camera2Manager::StopPreview() {
    if (captureSession_) {
        ACameraCaptureSession_stopRepeating(captureSession_);
        ACameraCaptureSession_close(captureSession_);
        captureSession_ = nullptr;
    }
    
    if (captureRequest_) {
        ACaptureRequest_free(captureRequest_);
        captureRequest_ = nullptr;
    }
    
    if (outputTarget_) {
        ACameraOutputTarget_free(outputTarget_);
        outputTarget_ = nullptr;
    }
    
    if (sessionOutput_) {
        ACaptureSessionOutput_free(sessionOutput_);
        sessionOutput_ = nullptr;
    }
    
    if (outputContainer_) {
        ACaptureSessionOutputContainer_free(outputContainer_);
        outputContainer_ = nullptr;
    }
    
    if (previewWindow_) {
        ANativeWindow_release(previewWindow_);
        previewWindow_ = nullptr;
    }
    
    previewTexture_.reset();
}

bool Camera2Manager::CreateCaptureSession() {
    // Create output target
    camera_status_t status = ACameraOutputTarget_create(previewWindow_, &outputTarget_);
    if (status != ACAMERA_OK) {
        LogError("Failed to create output target");
        return false;
    }
    
    // Create capture request
    status = ACameraDevice_createCaptureRequest(cameraDevice_, 
                                                TEMPLATE_PREVIEW, 
                                                &captureRequest_);
    if (status != ACAMERA_OK) {
        LogError("Failed to create capture request");
        return false;
    }
    
    // Add target to request
    status = ACaptureRequest_addTarget(captureRequest_, outputTarget_);
    if (status != ACAMERA_OK) {
        LogError("Failed to add target to request");
        return false;
    }
    
    // Create session output
    status = ACaptureSessionOutput_create(previewWindow_, &sessionOutput_);
    if (status != ACAMERA_OK) {
        LogError("Failed to create session output");
        return false;
    }
    
    // Create output container
    status = ACaptureSessionOutputContainer_create(&outputContainer_);
    if (status != ACAMERA_OK) {
        LogError("Failed to create output container");
        return false;
    }
    
    // Add output to container
    status = ACaptureSessionOutputContainer_add(outputContainer_, sessionOutput_);
    if (status != ACAMERA_OK) {
        LogError("Failed to add output to container");
        return false;
    }
    
    // Session callbacks
    ACameraCaptureSession_stateCallbacks sessionCallbacks = {
        .context = this,
        .onActive = OnSessionActive,
        .onReady = OnSessionReady,
        .onClosed = OnSessionClosed,
    };
    
    // Create capture session
    status = ACameraDevice_createCaptureSession(cameraDevice_, 
                                                outputContainer_,
                                                &sessionCallbacks,
                                                &captureSession_);
    if (status != ACAMERA_OK) {
        LogError("Failed to create capture session");
        return false;
    }
    
    // Apply camera parameters
    if (!ApplyCameraParams()) {
        LogWarning("Failed to apply camera parameters");
    }
    
    // Capture callbacks
    ACameraCaptureSession_captureCallbacks captureCallbacks = {
        .context = this,
        .onCaptureStarted = OnCaptureStarted,
        .onCaptureCompleted = OnCaptureCompleted,
        .onCaptureFailed = OnCaptureFailed,
    };
    
    // Start repeating request
    status = ACameraCaptureSession_setRepeatingRequest(captureSession_,
                                                       &captureCallbacks,
                                                       1,
                                                       &captureRequest_,
                                                       nullptr);
    if (status != ACAMERA_OK) {
        LogError("Failed to set repeating request");
        return false;
    }
    
    return true;
}

bool Camera2Manager::SetCameraParams(const CameraParams& params) {
    currentParams_ = params;
    
    if (captureRequest_) {
        return ApplyCameraParams();
    }
    
    return true;
}

bool Camera2Manager::ApplyCameraParams() {
    if (!captureRequest_) {
        return false;
    }
    
    // Auto Exposure
    uint8_t aeMode = currentParams_.autoExposure ? 
                     ACAMERA_CONTROL_AE_MODE_ON : ACAMERA_CONTROL_AE_MODE_OFF;
    ACaptureRequest_setEntry_u8(captureRequest_, ACAMERA_CONTROL_AE_MODE, 1, &aeMode);
    
    if (!currentParams_.autoExposure && currentParams_.exposureTime > 0) {
        ACaptureRequest_setEntry_i64(captureRequest_, ACAMERA_SENSOR_EXPOSURE_TIME, 
                                    1, &currentParams_.exposureTime);
        ACaptureRequest_setEntry_i32(captureRequest_, ACAMERA_SENSOR_SENSITIVITY,
                                    1, &currentParams_.sensitivity);
    }
    
    // Auto Focus
    uint8_t afMode = currentParams_.autoFocus ?
                     ACAMERA_CONTROL_AF_MODE_CONTINUOUS_PICTURE : ACAMERA_CONTROL_AF_MODE_OFF;
    ACaptureRequest_setEntry_u8(captureRequest_, ACAMERA_CONTROL_AF_MODE, 1, &afMode);
    
    if (!currentParams_.autoFocus && currentParams_.focusDistance > 0.0f) {
        ACaptureRequest_setEntry_float(captureRequest_, ACAMERA_LENS_FOCUS_DISTANCE,
                                      1, &currentParams_.focusDistance);
    }
    
    // Auto White Balance
    uint8_t awbMode = currentParams_.autoWhiteBalance ?
                      ACAMERA_CONTROL_AWB_MODE_AUTO : ACAMERA_CONTROL_AWB_MODE_OFF;
    ACaptureRequest_setEntry_u8(captureRequest_, ACAMERA_CONTROL_AWB_MODE, 1, &awbMode);
    
    // HDR
    if (currentParams_.hdrEnabled) {
        uint8_t sceneMode = ACAMERA_CONTROL_SCENE_MODE_HDR;
        ACaptureRequest_setEntry_u8(captureRequest_, ACAMERA_CONTROL_SCENE_MODE, 1, &sceneMode);
    }
    
    // OIS
    if (currentParams_.oisEnabled) {
        uint8_t oisMode = ACAMERA_LENS_OPTICAL_STABILIZATION_MODE_ON;
        ACaptureRequest_setEntry_u8(captureRequest_, ACAMERA_LENS_OPTICAL_STABILIZATION_MODE, 
                                   1, &oisMode);
    }
    
    // Flash
    if (currentParams_.flashEnabled) {
        uint8_t flashMode = ACAMERA_FLASH_MODE_TORCH;
        ACaptureRequest_setEntry_u8(captureRequest_, ACAMERA_FLASH_MODE, 1, &flashMode);
    }
    
    // FPS range
    int32_t fpsRange[2] = { currentParams_.minFPS, currentParams_.maxFPS };
    ACaptureRequest_setEntry_i32(captureRequest_, ACAMERA_CONTROL_AE_TARGET_FPS_RANGE, 
                                2, fpsRange);
    
    return true;
}

// Static callbacks
void Camera2Manager::OnDeviceDisconnected(void* context, ACameraDevice* device) {
    auto* manager = static_cast<Camera2Manager*>(context);
    manager->LogWarning("Camera disconnected");
}

void Camera2Manager::OnDeviceError(void* context, ACameraDevice* device, int error) {
    auto* manager = static_cast<Camera2Manager*>(context);
    manager->LogError("Camera error: %d", error);
}

void Camera2Manager::OnSessionActive(void* context, ACameraCaptureSession* session) {
    auto* manager = static_cast<Camera2Manager*>(context);
    manager->LogInfo("Capture session active");
}

void Camera2Manager::OnSessionReady(void* context, ACameraCaptureSession* session) {
    auto* manager = static_cast<Camera2Manager*>(context);
    manager->LogInfo("Capture session ready");
}

void Camera2Manager::OnSessionClosed(void* context, ACameraCaptureSession* session) {
    auto* manager = static_cast<Camera2Manager*>(context);
    manager->LogInfo("Capture session closed");
}

void Camera2Manager::OnCaptureStarted(void* context, ACameraCaptureSession* session,
                                     const ACaptureRequest* request, int64_t timestamp) {
    // Called for each frame start
}

void Camera2Manager::OnCaptureCompleted(void* context, ACameraCaptureSession* session,
                                       ACaptureRequest* request,
                                       const ACameraMetadata* result) {
    auto* manager = static_cast<Camera2Manager*>(context);
    
    if (manager->frameCallback_) {
        // Extract timestamp from metadata
        ACameraMetadata_const_entry entry;
        int64_t timestamp = 0;
        if (ACameraMetadata_getConstEntry(result, ACAMERA_SENSOR_TIMESTAMP, &entry) == ACAMERA_OK) {
            timestamp = entry.data.i64[0];
        }
        
        manager->frameCallback_->OnFrameAvailable(timestamp);
    }
}

void Camera2Manager::OnCaptureFailed(void* context, ACameraCaptureSession* session,
                                    ACaptureRequest* request,
                                    ACameraCaptureFailure* failure) {
    auto* manager = static_cast<Camera2Manager*>(context);
    manager->LogWarning("Capture failed");
}

} // namespace android
} // namespace ului

#endif // __ANDROID__
