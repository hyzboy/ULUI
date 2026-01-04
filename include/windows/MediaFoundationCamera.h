#pragma once

#ifdef _WIN32

#include "object.h"
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

namespace ului {

class Bitmap;

namespace gl {
class Texture2D;
}

// Camera facing direction
enum class CameraFacing {
    FRONT,
    BACK,
    EXTERNAL,
    UNKNOWN
};

// Camera resolution and format info
struct CameraFormat {
    int32_t width;
    int32_t height;
    int32_t frameRateNumerator;
    int32_t frameRateDenominator;
    
    float GetFPS() const {
        return frameRateDenominator > 0 ? 
               (float)frameRateNumerator / (float)frameRateDenominator : 0.0f;
    }
};

// Camera information
struct CameraInfo {
    std::string id;
    std::string name;
    CameraFacing facing;
    std::vector<CameraFormat> formats;
    bool supportAutoExposure;
    bool supportAutoFocus;
    bool supportAutoWhiteBalance;
};

// Camera control parameters
struct CameraParams {
    // Auto Exposure
    bool autoExposure = true;
    double exposureTime = 0.0;  // seconds (manual mode)
    
    // Auto Focus
    bool autoFocus = true;
    double focusDistance = 0.0;  // meters (manual mode)
    
    // Auto White Balance
    bool autoWhiteBalance = true;
    int32_t whiteBalanceTemp = 0;  // Kelvin (manual mode)
    
    // Brightness/Gain
    double brightness = 0.0;    // -1.0 to 1.0
    double gain = 0.0;          // ISO gain
    
    // Zoom
    double zoomRatio = 1.0;
};

// Frame callback
using CameraFrameCallback = std::function<void(std::shared_ptr<Bitmap> frame, int64_t timestampUs)>;

/**
 * @brief Windows Media Foundation camera capture manager
 * 
 * Provides high-level interface for camera enumeration and capture on Windows.
 * Uses Media Foundation Source Reader for efficient camera access.
 */
class MediaFoundationCamera : public Object {
public:
    MediaFoundationCamera();
    virtual ~MediaFoundationCamera();

    /**
     * @brief Initialize camera manager
     * @return true on success
     */
    bool Initialize();

    /**
     * @brief Cleanup all resources
     */
    void Cleanup();

    /**
     * @brief Enumerate available cameras
     * @return true on success
     */
    bool EnumerateCameras();

    /**
     * @brief Get list of available cameras
     * @return Vector of camera information
     */
    const std::vector<CameraInfo>& GetCameras() const { return m_cameras; }

    /**
     * @brief Get camera information by ID
     * @param cameraId Camera identifier
     * @return Camera info or nullptr if not found
     */
    const CameraInfo* GetCameraInfo(const std::string& cameraId) const;

    /**
     * @brief Open a camera
     * @param cameraId Camera identifier
     * @return true on success
     */
    bool OpenCamera(const std::string& cameraId);

    /**
     * @brief Close current camera
     */
    void CloseCamera();

    /**
     * @brief Check if camera is open
     * @return true if camera is open
     */
    bool IsOpen() const { return m_isOpen; }

    /**
     * @brief Get current camera information
     * @return Camera info or nullptr if no camera is open
     */
    const CameraInfo* GetCurrentCameraInfo() const;

    /**
     * @brief Start camera capture
     * @param width Desired capture width
     * @param height Desired capture height
     * @param frameRate Desired frame rate (0 = default)
     * @return true on success
     */
    bool StartCapture(int32_t width, int32_t height, float frameRate = 0);

    /**
     * @brief Stop camera capture
     */
    void StopCapture();

    /**
     * @brief Check if capture is active
     * @return true if capturing
     */
    bool IsCapturing() const { return m_isCapturing; }

    /**
     * @brief Set frame callback for receiving captured frames
     * @param callback Callback function to receive frames
     */
    void SetFrameCallback(CameraFrameCallback callback) { m_frameCallback = callback; }

    /**
     * @brief Get latest captured frame
     * @param bitmap Output bitmap to receive frame data
     * @param timestampUs Output timestamp in microseconds
     * @return true if new frame available
     */
    bool GetFrame(std::shared_ptr<Bitmap> bitmap, int64_t* timestampUs = nullptr);

    /**
     * @brief Set camera parameters
     * @param params Camera control parameters
     * @return true on success
     */
    bool SetCameraParams(const CameraParams& params);

    /**
     * @brief Get current camera parameters
     * @return Current parameters
     */
    CameraParams GetCameraParams() const { return m_currentParams; }

    /**
     * @brief Set camera format
     * @param width Desired width
     * @param height Desired height
     * @param frameRate Desired frame rate
     * @return true on success
     */
    bool SetCameraFormat(int32_t width, int32_t height, float frameRate = 30.0f);

    // Getters
    int32_t GetWidth() const { return m_width; }
    int32_t GetHeight() const { return m_height; }
    float GetFrameRate() const { return m_frameRate; }

private:
    // Forward declaration for Media Foundation types
    struct MediaFoundationContext;
    MediaFoundationContext* m_context;
    
    // Camera state
    bool m_isOpen;
    bool m_isCapturing;
    std::string m_currentCameraId;
    std::vector<CameraInfo> m_cameras;
    
    // Capture parameters
    int32_t m_width;
    int32_t m_height;
    float m_frameRate;
    CameraParams m_currentParams;
    
    // Frame callback
    CameraFrameCallback m_frameCallback;
    
    // Internal helper methods
    bool InitializeMediaFoundation();
    void CleanupMediaFoundation();
    bool LoadCameraInfo(void* device, CameraInfo& info);
    bool CreateSourceReader(const std::string& symbolicLink);
    bool ConfigureSourceReader();
    bool ApplyCameraParams();
    void ReadFrameThread();
    CameraFacing DetermineCameraFacing(const std::string& name) const;
    
    // Thread management (thread handle is in context)
};

} // namespace ului

#endif // _WIN32
