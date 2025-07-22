#ifndef INTEL_GLASSES_CONFIG_H
#define INTEL_GLASSES_CONFIG_H

// ===================
// 4G Module Configuration
// ===================
#define GSM_PIN_TX      17
#define GSM_PIN_RX      18
#define GSM_PIN_PWR     16
#define GSM_PIN_RST     5
#define GSM_BAUD        9600

// ===================
// Cloud API Configuration
// ===================
#define CLOUD_API_HOST      "your-cloud-platform.com"
#define CLOUD_API_PORT      443
#define CLOUD_API_KEY       "your-api-key-here"
#define CLOUD_API_TIMEOUT   30000  // 30 seconds

// ===================
// API Endpoints
// ===================
#define HAZARD_DETECTION_ENDPOINT   "/api/v1/hazard-detection"
#define VISUAL_CAPTION_ENDPOINT     "/api/v1/visual-caption"
#define SIGN_DETECTION_ENDPOINT     "/api/v1/sign-detection"
#define OCR_ENDPOINT               "/api/v1/ocr"

// ===================
// System Configuration
// ===================
#define CAPTURE_INTERVAL       5000   // 5 seconds between captures
#define MAX_RETRIES           3
#define JPEG_QUALITY          12
#define IMAGE_WIDTH           640
#define IMAGE_HEIGHT          480

// ===================
// LED Status Indicators
// ===================
#define STATUS_LED_PIN        2
#define HAZARD_LED_PIN        33
#define PROCESSING_LED_PIN    32

// ===================
// Audio/Haptic Feedback
// ===================
#define BUZZER_PIN           25
#define VIBRATION_PIN        26

// ===================
// Speech Recognition Configuration
// ===================
#define I2S_WS_PIN          15        // Word Select (LRC) pin for microphone
#define I2S_SCK_PIN         14        // Serial Clock (BCLK) pin for microphone
#define I2S_SD_PIN          32        // Serial Data (DIN) pin for microphone
#define SPEECH_SAMPLE_RATE  16000     // Sample rate for speech recognition (16kHz)
#define SPEECH_CONFIDENCE_THRESHOLD 0.8  // Minimum confidence for speech commands

// ===================
// Button Configuration
// ===================
#define CAPTURE_BUTTON_PIN   0
#define MODE_BUTTON_PIN      14

// ===================
// Operation Modes
// ===================
enum OperationMode {
    MODE_HAZARD_DETECTION,
    MODE_VISUAL_CAPTION,
    MODE_SIGN_DETECTION,
    MODE_OCR,
    MODE_AUTO_ALL
};

// ===================
// Response Structure
// ===================
struct APIResponse {
    bool success;
    String result;
    String error;
    float confidence;
    int processing_time;
    
    // Audio response fields
    bool hasAudio;
    String audioUrl;        // URL for audio file from cloud
    String audioFormat;     // mp3, wav, etc.
    size_t audioSize;       // Size of audio data
};

#endif // INTEL_GLASSES_CONFIG_H
