#include "camera_manager.h"
#include "board_config.h"
#include "camera_pins.h"
#include <cstring>

CameraManager cameraManager;

CameraManager::CameraManager() {
    isInitialized = false;
    sensor = nullptr;
    lastCaptureTime = 0;
    captureCount = 0;
    autoCaptureEnabled = true; // Enable by default for glasses
}

bool CameraManager::initialize() {
    Serial.println("Initializing camera...");
    
    // Configure camera
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_VGA; // 640x480 for AI processing
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = JPEG_QUALITY;
    config.fb_count = 1;
    
    // Optimize for PSRAM if available
    if (psramFound()) {
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
        Serial.println("PSRAM found - using optimized settings");
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
        Serial.println("PSRAM not found - using DRAM");
    }
    
#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get sensor handle
    sensor = esp_camera_sensor_get();
    if (!sensor) {
        Serial.println("Failed to get camera sensor");
        return false;
    }
    
    setupDefaultSettings();
    logCameraStatus();
    
    isInitialized = true;
    Serial.println("Camera initialized successfully");
    return true;
}

bool CameraManager::reconfigure(framesize_t frameSize, int jpegQuality) {
    if (!isInitialized) return false;
    
    setFrameSize(frameSize);
    setJPEGQuality(jpegQuality);
    
    Serial.printf("Camera reconfigured - Frame size: %d, JPEG quality: %d\n", 
                  frameSize, jpegQuality);
    return true;
}

void CameraManager::deinitialize() {
    if (isInitialized) {
        esp_camera_deinit();
        isInitialized = false;
        sensor = nullptr;
        Serial.println("Camera deinitialized");
    }
}

camera_fb_t* CameraManager::captureImage() {
    if (!isInitialized) {
        Serial.println("Camera not initialized");
        return nullptr;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return nullptr;
    }
    
    lastCaptureTime = millis();
    captureCount++;
    
    Serial.printf("Image captured: %d bytes, %dx%d\n", 
                  fb->len, fb->width, fb->height);
    
    return fb;
}

void CameraManager::releaseFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool CameraManager::captureToBuffer(uint8_t** imageData, size_t* imageSize) {
    camera_fb_t* fb = captureImage();
    if (!fb) {
        return false;
    }
    
    // Allocate buffer and copy data
    *imageSize = fb->len;
    *imageData = (uint8_t*)malloc(*imageSize);
    
    if (*imageData == nullptr) {
        Serial.println("Failed to allocate image buffer");
        releaseFrameBuffer(fb);
        return false;
    }
    
    memcpy(*imageData, fb->buf, fb->len);
    releaseFrameBuffer(fb);
    
    return true;
}

bool CameraManager::setFrameSize(framesize_t size) {
    if (!sensor) return false;
    return sensor->set_framesize(sensor, size) == 0;
}

bool CameraManager::setJPEGQuality(int quality) {
    if (!sensor) return false;
    return sensor->set_quality(sensor, quality) == 0;
}

bool CameraManager::setBrightness(int level) {
    if (!sensor) return false;
    return sensor->set_brightness(sensor, level) == 0;
}

bool CameraManager::setContrast(int level) {
    if (!sensor) return false;
    return sensor->set_contrast(sensor, level) == 0;
}

bool CameraManager::setSaturation(int level) {
    if (!sensor) return false;
    return sensor->set_saturation(sensor, level) == 0;
}

bool CameraManager::setSpecialEffect(int effect) {
    if (!sensor) return false;
    return sensor->set_special_effect(sensor, effect) == 0;
}

bool CameraManager::setWhiteBalance(bool enable) {
    if (!sensor) return false;
    return sensor->set_whitebal(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::setAutoWhiteBalance(bool enable) {
    if (!sensor) return false;
    return sensor->set_awb_gain(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::setAutoExposureControl(bool enable) {
    if (!sensor) return false;
    return sensor->set_aec2(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::setAutoExposureControl2(bool enable) {
    if (!sensor) return false;
    return sensor->set_aec2(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::setExposureControl(bool enable) {
    if (!sensor) return false;
    return sensor->set_ae_level(sensor, enable ? 0 : -2) == 0;
}

bool CameraManager::setAutoGainControl(bool enable) {
    if (!sensor) return false;
    return sensor->set_aec2(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::setGainControlling(int level) {
    if (!sensor) return false;
    return sensor->set_agc_gain(sensor, level) == 0;
}

bool CameraManager::setHorizontalMirror(bool enable) {
    if (!sensor) return false;
    return sensor->set_hmirror(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::setVerticalFlip(bool enable) {
    if (!sensor) return false;
    return sensor->set_vflip(sensor, enable ? 1 : 0) == 0;
}

bool CameraManager::isReady() {
    return isInitialized && sensor != nullptr;
}

String CameraManager::getCameraInfo() {
    if (!sensor) return "Camera not initialized";
    
    return "Camera Info: PID=0x" + String(sensor->id.PID, HEX) + 
           ", VER=0x" + String(sensor->id.VER, HEX) +
           ", MIDL=0x" + String(sensor->id.MIDL, HEX) +
           ", MIDH=0x" + String(sensor->id.MIDH, HEX);
}

int CameraManager::getCaptureCount() {
    return captureCount;
}

unsigned long CameraManager::getLastCaptureTime() {
    return lastCaptureTime;
}

void CameraManager::enableAutoCaptureMode(bool enable) {
    autoCaptureEnabled = enable;
    Serial.println("Auto capture mode: " + String(enable ? "ENABLED" : "DISABLED"));
}

bool CameraManager::isAutoCaptureEnabled() {
    return autoCaptureEnabled;
}

bool CameraManager::shouldAutoCapture() {
    if (!autoCaptureEnabled) return false;
    
    return (millis() - lastCaptureTime) >= CAPTURE_INTERVAL;
}

void CameraManager::setupDefaultSettings() {
    if (!sensor) return;
    
    // Apply optimal settings for Intel glasses use case
    
    // Initial sensors are flipped vertically and colors are saturated
    if (sensor->id.PID == OV3660_PID) {
        setVerticalFlip(true);        // flip it back
        setBrightness(1);             // up the brightness just a bit
        setSaturation(-2);            // lower the saturation
    }
    
#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    setVerticalFlip(true);
    setHorizontalMirror(true);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
    setVerticalFlip(true);
#endif
    
    // Optimize for AI processing
    setFrameSize(FRAMESIZE_VGA);  // 640x480 good balance of quality and processing speed
    setJPEGQuality(12);           // Medium quality for faster transmission
    
    // Auto settings for varying light conditions
    setAutoWhiteBalance(true);
    setAutoExposureControl(true);
    setAutoGainControl(true);
    
    // Enhance for outdoor use (Intel glasses context)
    setContrast(0);     // Normal contrast
    setBrightness(0);   // Normal brightness
    setSaturation(0);   // Normal saturation
    
    Serial.println("Default camera settings applied");
}

void CameraManager::logCameraStatus() {
    Serial.println("=== Camera Status ===");
    Serial.println(getCameraInfo());
    Serial.printf("Frame size: %d\n", config.frame_size);
    Serial.printf("JPEG quality: %d\n", config.jpeg_quality);
    Serial.printf("FB count: %d\n", config.fb_count);
    Serial.printf("FB location: %s\n", (config.fb_location == CAMERA_FB_IN_PSRAM) ? "PSRAM" : "DRAM");
    Serial.printf("Total captures: %d\n", captureCount);
    Serial.println("====================");
}
