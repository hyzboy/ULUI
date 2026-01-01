#pragma once

#ifdef __ANDROID__

#include "../core/Object.h"
#include "../gl/Texture2D.h"
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraMetadata.h>
#include <media/NdkImageReader.h>
#include <android/native_window.h>
#include <vector>
#include <string>
#include <memory>

namespace ului {
namespace android {

// Camera facing direction
enum class CameraFacing {
    BACK,
    FRONT,
    EXTERNAL
};

// Camera capability flags
enum class CameraCapability {
    BACKWARD_COMPATIBLE,
    MANUAL_SENSOR,
    MANUAL_POST_PROCESSING,
    RAW,
    PRIVATE_REPROCESSING,
    YUV_REPROCESSING,
    DEPTH_OUTPUT,
    CONSTRAINED_HIGH_SPEED_VIDEO,
    MOTION_TRACKING,
    LOGICAL_MULTI_CAMERA,
    MONOCHROME,
    SECURE_IMAGE_DATA
};

// Camera resolution and format info
struct CameraFormat {
    int32_t width;
    int32_t height;
    int32_t format;  // AIMAGE_FORMAT_*
    int64_t minFrameDuration;  // nanoseconds
    int64_t stallDuration;     // nanoseconds
    
    float GetFPS() const { 
        return minFrameDuration > 0 ? 1e9f / minFrameDuration : 0.0f; 
    }
};

// Camera information
struct CameraInfo {
    std::string id;
    CameraFacing facing;
    int32_t sensorOrientation;
    std::vector<CameraCapability> capabilities;
    std::vector<CameraFormat> formats;
    bool supportHDR;
    bool supportOIS;  // Optical Image Stabilization
    bool supportEIS;  // Electronic Image Stabilization
    bool supportAF;   // Auto Focus
    bool supportAE;   // Auto Exposure
    bool supportAWB;  // Auto White Balance
    
    // Sensor info
    int32_t maxAnalogSensitivity;
    int32_t maxDigitalZoom;
    float focalLength;
    float aperture;
};

// Camera control parameters
struct CameraParams {
    // Auto Exposure
    bool autoExposure = true;
    int64_t exposureTime = 0;  // nanoseconds (manual mode)
    int32_t sensitivity = 0;   // ISO (manual mode)
    
    // Auto Focus
    bool autoFocus = true;
    float focusDistance = 0.0f;  // diopters (manual mode)
    
    // Auto White Balance
    bool autoWhiteBalance = true;
    int32_t whiteBalanceMode = 0;
    
    // Zoom
    float zoomRatio = 1.0f;
    
    // Image quality
    int32_t jpegQuality = 95;
    
    // HDR
    bool hdrEnabled = false;
    
    // Stabilization
    bool oisEnabled = false;
    bool eisEnabled = false;
    
    // Flash
    bool flashEnabled = false;
    
    // FPS range (for video)
    int32_t minFPS = 30;
    int32_t maxFPS = 30;
};

// Camera frame callback
class ICameraFrameCallback {
public:
    virtual ~ICameraFrameCallback() = default;
    
    // Called when new frame is available on the texture
    // timestamp is in nanoseconds
    virtual void OnFrameAvailable(int64_t timestamp) = 0;
};

// Camera2 Manager - manages camera enumeration and access
class Camera2Manager : public Object {
public:
    Camera2Manager();
    virtual ~Camera2Manager();
    
    // Initialize camera manager
    bool Initialize();
    
    // Cleanup
    void Cleanup();
    
    // Camera enumeration
    bool EnumerateCameras();
    const std::vector<CameraInfo>& GetCameras() const { return cameras_; }
    const CameraInfo* GetCameraInfo(const std::string& cameraId) const;
    
    // Open/close camera
    bool OpenCamera(const std::string& cameraId);
    void CloseCamera();
    bool IsOpen() const { return cameraDevice_ != nullptr; }
    
    // Get current camera info
    const CameraInfo* GetCurrentCameraInfo() const;
    
    // Create camera texture (OES EXTERNAL)
    std::shared_ptr<Texture2D> CreateCameraTexture();
    
    // Start/stop preview with OpenGL texture
    bool StartPreview(std::shared_ptr<Texture2D> texture, int32_t width, int32_t height);
    void StopPreview();
    bool IsPreviewActive() const { return captureSession_ != nullptr; }
    
    // Camera parameter control
    bool SetCameraParams(const CameraParams& params);
    CameraParams GetCameraParams() const { return currentParams_; }
    
    // Frame callback
    void SetFrameCallback(ICameraFrameCallback* callback) { frameCallback_ = callback; }
    
    // Get native window for the texture (for SurfaceTexture)
    ANativeWindow* GetPreviewWindow() const { return previewWindow_; }
    
private:
    // Camera manager
    ACameraManager* cameraManager_;
    
    // Current camera
    ACameraDevice* cameraDevice_;
    std::string currentCameraId_;
    
    // Capture session
    ACameraCaptureSession* captureSession_;
    ACaptureRequest* captureRequest_;
    ACameraOutputTarget* outputTarget_;
    ACaptureSessionOutput* sessionOutput_;
    ACaptureSessionOutputContainer* outputContainer_;
    
    // Preview surface
    ANativeWindow* previewWindow_;
    std::shared_ptr<Texture2D> previewTexture_;
    
    // Camera info
    std::vector<CameraInfo> cameras_;
    
    // Current parameters
    CameraParams currentParams_;
    
    // Frame callback
    ICameraFrameCallback* frameCallback_;
    
    // Helper methods
    bool LoadCameraInfo(const char* cameraId, CameraInfo& info);
    void ParseCameraCharacteristics(ACameraMetadata* metadata, CameraInfo& info);
    bool CreateCaptureSession();
    bool ApplyCameraParams();
    
    // Camera callbacks
    static void OnDeviceDisconnected(void* context, ACameraDevice* device);
    static void OnDeviceError(void* context, ACameraDevice* device, int error);
    static void OnSessionActive(void* context, ACameraCaptureSession* session);
    static void OnSessionReady(void* context, ACameraCaptureSession* session);
    static void OnSessionClosed(void* context, ACameraCaptureSession* session);
    
    // Capture callbacks
    static void OnCaptureStarted(void* context, ACameraCaptureSession* session,
                                 const ACaptureRequest* request, int64_t timestamp);
    static void OnCaptureCompleted(void* context, ACameraCaptureSession* session,
                                   ACaptureRequest* request,
                                   const ACameraMetadata* result);
    static void OnCaptureFailed(void* context, ACameraCaptureSession* session,
                                ACaptureRequest* request,
                                ACameraCaptureFailure* failure);
};

} // namespace android
} // namespace ului

#endif // __ANDROID__
