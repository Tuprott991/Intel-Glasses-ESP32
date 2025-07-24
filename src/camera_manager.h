#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include "esp_camera.h"
#include "intel_glasses_config.h"

class CameraManager {
private:
    bool isInitialized;
    camera_config_t config;
    sensor_t* sensor;
    unsigned long lastCaptureTime;
    int captureCount;
    
public:
    CameraManager();
    
    // Initialization and configuration
    bool initialize();
    bool reconfigure(framesize_t frameSize, int jpegQuality);
    void deinitialize();
    
    // Image capture
    camera_fb_t* captureImage();
    void releaseFrameBuffer(camera_fb_t* fb);
    bool captureToBuffer(uint8_t** imageData, size_t* imageSize);
    
    // Camera settings
    bool setFrameSize(framesize_t size);
    bool setJPEGQuality(int quality);
    bool setBrightness(int level);    // -2 to 2
    bool setContrast(int level);      // -2 to 2
    bool setSaturation(int level);    // -2 to 2
    bool setSpecialEffect(int effect); // 0-6
    bool setWhiteBalance(bool enable);
    bool setAutoWhiteBalance(bool enable);
    bool setAutoExposureControl(bool enable);
    bool setAutoExposureControl2(bool enable);
    bool setExposureControl(bool enable);
    bool setAutoGainControl(bool enable);
    bool setGainControlling(int level);
    bool setHorizontalMirror(bool enable);
    bool setVerticalFlip(bool enable);
    
    // Status and info
    bool isReady();
    String getCameraInfo();
    int getCaptureCount();
    unsigned long getLastCaptureTime();
    
    // Auto capture for continuous monitoring
    void enableAutoCaptureMode(bool enable);
    bool isAutoCaptureEnabled();
    bool shouldAutoCapture();
    
    // Default settings
    void setupDefaultSettings();
    
private:
    void logCameraStatus();
    bool autoCaptureEnabled;
};

// Global camera manager instance
extern CameraManager cameraManager;

#endif // CAMERA_MANAGER_H
