# Intel AI Glasses - ESP32-S3-CAM Project

## Overview

This project implements an intelligent glasses system using ESP32-S3-CAM with 4G connectivity to provide real-time AI-powered vision assistance. The system offers hazard detection, visual captioning, sign recognition, and optical character recognition (OCR) capabilities through cloud-based AI processing.

## Features

### **Core AI Capabilities**
- **Hazard Detection**: Real-time identification of dangers and safety hazards
- **Visual Captioning**: Natural language descriptions of the visual environment  
- **Sign Recognition**: Detection and classification of traffic signs, warning signs, and informational signage
- **OCR (Optical Character Recognition)**: Text extraction and reading from images

### **Connectivity**
- **4G/LTE Module**: Reliable cloud connectivity for AI processing
- **Cloud API Integration**: Seamless communication with your cloud AI platform
- **Offline Fallback**: Basic local processing when network is unavailable

### **User Interface**
- **Voice Control**: Hands-free operation with speech recognition (Edge Impulse models)
- **Dual Button Control**: Physical buttons for backup control
- **Audio Feedback**: Text-to-speech announcements and alerts
- **Haptic Feedback**: Vibration patterns for different alert types
- **LED Status Indicators**: Visual system status and alerts

###  **Operating Modes**
1. **Hazard Detection Mode**: Prioritizes safety and danger identification
2. **Visual Caption Mode**: Provides environmental descriptions
3. **Sign Detection Mode**: Focuses on sign recognition and classification
4. **OCR Mode**: Optimized for text recognition and reading
5. **Auto Mode**: Automatically runs all features based on context

## Hardware Requirements

### Main Components
- **ESP32-S3-DevKitC-1** with PSRAM (recommended: N16R8V)
- **ESP32-S3-CAM module** or compatible camera module
- **4G/LTE Module** (SIM7600, SIM800, or compatible)
- **Micro SD Card** (optional, for local storage)
- **Li-Po Battery** (3.7V, 1000mAh+ recommended)

### Additional Components
- **I2S Microphone** (INMP441 or compatible for speech recognition)
- **2x Push Buttons** (Capture and Mode control - backup to voice)
- **3x LEDs** (Status, Hazard, Processing indicators)
- **Buzzer/Speaker** (Audio feedback)
- **Vibration Motor** (Haptic feedback)
- **Resistors, capacitors** (standard values for LED and button circuits)

### Pin Connections

```cpp
// 4G Module
#define GSM_PIN_TX      17
#define GSM_PIN_RX      18
#define GSM_PIN_PWR     16
#define GSM_PIN_RST     5

// Speech Recognition (I2S Microphone)
#define I2S_WS_PIN      15
#define I2S_SCK_PIN     14
#define I2S_SD_PIN      32

// User Interface
#define CAPTURE_BUTTON_PIN   0
#define MODE_BUTTON_PIN      14

// Status Indicators
#define STATUS_LED_PIN       2
#define HAZARD_LED_PIN       33
#define PROCESSING_LED_PIN   32

// Feedback
#define BUZZER_PIN          25
#define VIBRATION_PIN       26
```

## Software Architecture

### Core Modules

1. **CameraManager** (`camera_manager.h/cpp`)
   - Camera initialization and configuration
   - Image capture and buffer management
   - Quality and settings optimization

2. **GSMModule** (`gsm_module.h/cpp`)  
   - 4G/LTE connectivity management
   - HTTP client for API communication
   - Network status monitoring

3. **AIProcessor** (`ai_processor.h/cpp`)
   - AI feature processing coordinator
   - Response handling and interpretation
   - Feedback generation (audio/haptic/visual)

4. **InputHandler** (`input_handler.h/cpp`)
   - Button press detection and debouncing
   - Gesture recognition (single/double/long press)
   - User input processing

5. **DisplayHandler** (`display_handler.h/cpp`)
   - Status display management
   - User feedback coordination
   - System information display

6. **SpeechRecognition** (`speech_recognition.h/cpp`)
   - Edge Impulse model integration
   - Voice command processing
   - I2S microphone interface
   - Audio preprocessing and inference

7. **IntelGlasses** (`intel_glasses.h/cpp`)
   - Main system controller
   - State management and coordination
   - Error handling and recovery

## Setup Instructions

### 1. Hardware Assembly
1. Connect ESP32-S3-CAM module to development board
2. Wire 4G module using UART pins (TX: 17, RX: 18)
3. Connect I2S microphone (INMP441) for speech recognition
4. Connect buttons with pull-up resistors (backup control)
5. Install status LEDs with current limiting resistors
6. Connect buzzer and vibration motor with appropriate drivers

### 2. Software Configuration

#### PlatformIO Setup
```ini
[env:esp32-s3-devkitc-1-n16r8v]
platform = espressif32
board = esp32-s3-devkitc-1-n16r8v
framework = arduino
lib_deps = 
    ArduinoJson
    TinyGSM
    HTTPClient
    StreamDebugger
    PubSubClient
    FASTLED
```

#### Cloud API Configuration
Edit `intel_glasses_config.h`:
```cpp
#define CLOUD_API_HOST      "your-cloud-platform.com"
#define CLOUD_API_KEY       "your-api-key-here"
```

#### 4G Module Setup
Configure APN settings in `gsm_module.cpp`:
```cpp
const char* apn = "your-carrier-apn";      // Your carrier's APN
const char* gprsUser = "username";         // If required
const char* gprsPass = "password";         // If required
```

### 3. Cloud Platform Setup

Your cloud platform should provide REST API endpoints for:

- `POST /api/v1/hazard-detection` - Hazard detection analysis
- `POST /api/v1/visual-caption` - Visual description generation  
- `POST /api/v1/sign-detection` - Sign recognition and classification
- `POST /api/v1/ocr` - Optical character recognition

Expected request format:
```json
{
    "image": "base64_encoded_image_data",
    "api_key": "your_api_key",
    "mode": 0,
    "timestamp": 1234567890
}
```

Expected response format:
```json
{
    "success": true,
    "result": "Description or detected text",
    "confidence": 0.95,
    "error": null
}
```

## Usage Guide

### Voice Commands (Primary Control)

The Intel Glasses responds to natural voice commands for hands-free operation:

**Mode Switching:**
- **"Hazard mode"** - Switch to hazard detection
- **"Caption mode"** - Switch to visual description
- **"Sign mode"** - Switch to sign recognition  
- **"Text mode"** - Switch to OCR/text reading
- **"Auto mode"** - Switch to automatic all-features mode

**Actions:**
- **"Capture"** or **"Take picture"** - Manual image capture
- **"Emergency"** or **"Help"** - Emergency alert
- **"Status"** - System status report
- **"Sleep"** - Enter power saving mode
- **"Wake up"** - Exit sleep mode

### Button Controls (Backup)

**Capture Button:**
- **Single Click**: Manual image capture and processing
- **Long Press**: Toggle auto-capture mode on/off
- **Double Click**: Emergency alert activation

**Mode Button:**
- **Single Click**: Cycle through operating modes
- **Long Press**: Speak system status and statistics
- **Double Click**: System calibration and optimization

### Operating Modes

1. **Hazard Detection**: Continuously monitors for safety hazards
   - Alerts for dangerous objects, situations, obstacles
   - High-priority audio and haptic warnings
   - Red LED indication for detected hazards

2. **Visual Caption**: Describes the environment
   - Natural language scene descriptions
   - Useful for navigation and awareness
   - Gentle audio feedback

3. **Sign Detection**: Recognizes and reads signs
   - Traffic signs, warning signs, informational signs
   - Audio announcement of sign content
   - Special alerts for warning/danger signs

4. **OCR Mode**: Reads text from images
   - Books, menus, labels, documents
   - Text-to-speech conversion
   - Useful for reading assistance

5. **Auto Mode**: Intelligent context switching
   - Prioritizes hazard detection
   - Switches between modes based on detected content
   - Comprehensive environmental awareness

### LED Status Indicators

- **Status LED (Blue)**: 
  - Solid = System ready
  - Blinking = Processing
  - Off = System error/sleep

- **Hazard LED (Red)**:
  - Solid = Hazard detected
  - Flashing = Emergency mode
  - Off = No hazards

- **Processing LED (Green)**:
  - On = AI processing active
  - Off = Idle

---

**Intel AI Glasses** - Empowering vision through artificial intelligence
