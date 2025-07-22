/*
 * Intel Glasses Configuration Template
 * 
 * Copy this file to intel_glasses_config.h and configure your settings
 */

#ifndef INTEL_GLASSES_CONFIG_H
#define INTEL_GLASSES_CONFIG_H

// ===================
// Cloud Platform Configuration
// ===================
// Replace with your actual cloud platform details
#define CLOUD_API_HOST      "your-cloud-platform.com"  // Your cloud API hostname
#define CLOUD_API_PORT      443                         // HTTPS port (443) or HTTP port (80)
#define CLOUD_API_KEY       "your-api-key-here"        // Your API authentication key
#define CLOUD_API_TIMEOUT   30000                       // API timeout in milliseconds

// ===================
// 4G Module Configuration  
// ===================
// Configure these based on your 4G module and carrier
#define GSM_PIN_TX      17      // TX pin connected to GSM module RX
#define GSM_PIN_RX      18      // RX pin connected to GSM module TX  
#define GSM_PIN_PWR     16      // Power control pin for GSM module
#define GSM_PIN_RST     5       // Reset pin for GSM module
#define GSM_BAUD        9600    // Baud rate for GSM communication

// ===================
// Carrier APN Settings
// ===================
// Configure these for your mobile carrier
// Common APN settings:
// Verizon: "vzwinternet", "", ""
// AT&T: "broadband", "", ""  
// T-Mobile: "fast.t-mobile.com", "", ""
// Check with your carrier for correct settings
const char* APN_NAME = "internet";      // Your carrier's APN name
const char* APN_USER = "";              // APN username (leave empty if not required)
const char* APN_PASS = "";              // APN password (leave empty if not required)

// ===================
// API Endpoint Configuration
// ===================
// Configure these to match your cloud platform API
#define HAZARD_DETECTION_ENDPOINT   "/api/v1/hazard-detection"
#define VISUAL_CAPTION_ENDPOINT     "/api/v1/visual-caption"
#define SIGN_DETECTION_ENDPOINT     "/api/v1/sign-detection"
#define OCR_ENDPOINT               "/api/v1/ocr"

// ===================
// Hardware Pin Configuration
// ===================
// User Interface
#define CAPTURE_BUTTON_PIN   0      // GPIO pin for capture/trigger button
#define MODE_BUTTON_PIN      14     // GPIO pin for mode selection button

// LED Status Indicators
#define STATUS_LED_PIN       2      // GPIO pin for system status LED (blue)
#define HAZARD_LED_PIN       33     // GPIO pin for hazard alert LED (red)
#define PROCESSING_LED_PIN   32     // GPIO pin for processing indicator LED (green)

// Audio and Haptic Feedback
#define BUZZER_PIN          25      // GPIO pin for audio buzzer/speaker
#define VIBRATION_PIN       26      // GPIO pin for vibration motor

// ===================
// System Configuration
// ===================
#define CAPTURE_INTERVAL       5000   // Auto-capture interval in milliseconds (5 seconds)
#define MAX_RETRIES           3       // Maximum retry attempts for failed operations
#define JPEG_QUALITY          12      // JPEG compression quality (lower = higher quality, larger size)
#define IMAGE_WIDTH           640     // Target image width in pixels
#define IMAGE_HEIGHT          480     // Target image height in pixels

// ===================
// Performance Tuning
// ===================
// Adjust these based on your network conditions and requirements
#define LOW_BANDWIDTH_MODE    false   // Enable to reduce image quality for slower connections
#define AGGRESSIVE_TIMEOUT    false   // Enable to use shorter timeouts for faster response
#define BATTERY_SAVE_MODE     false   // Enable to reduce processing frequency for battery life

// ===================
// Debug Configuration
// ===================
#define DEBUG_SERIAL_OUTPUT   true    // Enable detailed serial debug output
#define DEBUG_NETWORK        false   // Enable network debugging (verbose)
#define DEBUG_CAMERA         false   // Enable camera debugging
#define DEBUG_AI_RESPONSES   true    // Enable AI response debugging

// ===================
// Feature Flags
// ===================
// Enable/disable specific features based on your needs
#define ENABLE_HAZARD_DETECTION   true
#define ENABLE_VISUAL_CAPTION     true  
#define ENABLE_SIGN_DETECTION     true
#define ENABLE_OCR               true
#define ENABLE_AUTO_MODE         true
#define ENABLE_EMERGENCY_ALERT   true

// ===================
// Advanced Settings
// ===================
#define CONFIDENCE_THRESHOLD     0.7    // Minimum confidence for accepting AI results (0.0 - 1.0)
#define HAZARD_SENSITIVITY      0.6     // Sensitivity for hazard detection (0.0 - 1.0)
#define MAX_IMAGE_SIZE          100000  // Maximum image size in bytes for transmission
#define MEMORY_CHECK_INTERVAL   60000   // How often to check system memory (ms)

// ===================
// Response Structure (DO NOT MODIFY)
// ===================
enum OperationMode {
    MODE_HAZARD_DETECTION,
    MODE_VISUAL_CAPTION,
    MODE_SIGN_DETECTION,
    MODE_OCR,
    MODE_AUTO_ALL
};

struct APIResponse {
    bool success;
    String result;
    String error;
    float confidence;
    int processing_time;
};

#endif // INTEL_GLASSES_CONFIG_H

/*
 * Configuration Notes:
 * 
 * 1. Cloud API Setup:
 *    - Replace CLOUD_API_HOST with your actual server
 *    - Ensure your API key is valid and has proper permissions
 *    - Test endpoints manually before deploying
 * 
 * 2. 4G Module Setup:
 *    - Verify pin connections match your hardware
 *    - Check APN settings with your mobile carrier
 *    - Ensure SIM card is activated and has data plan
 * 
 * 3. Hardware Configuration:
 *    - Adjust GPIO pins based on your actual connections
 *    - Use appropriate resistors for LEDs (typically 220Ω - 1kΩ)
 *    - Ensure button pins have pull-up resistors
 * 
 * 4. Performance Tuning:
 *    - Reduce JPEG_QUALITY for faster transmission (higher number = lower quality)
 *    - Increase CAPTURE_INTERVAL to save battery and reduce data usage
 *    - Adjust CONFIDENCE_THRESHOLD based on your accuracy requirements
 * 
 * 5. Troubleshooting:
 *    - Enable DEBUG flags for detailed logging
 *    - Monitor serial output during operation
 *    - Check network connectivity with AT commands
 *    - Verify API responses match expected format
 */
