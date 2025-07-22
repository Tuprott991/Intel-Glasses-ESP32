# Edge Impulse Integration Guide for Intel Glasses

This guide explains how to integrate your Edge Impulse speech recognition model into the Intel Glasses project.

## Prerequisites

1. **Edge Impulse Account**: Sign up at [edgeimpulse.com](https://edgeimpulse.com)
2. **Trained Model**: Complete speech recognition model trained on Edge Impulse
3. **ESP32-S3 with Microphone**: I2S-compatible microphone module

## Step 1: Train Your Model on Edge Impulse

### 1.1 Create New Project
- Go to Edge Impulse Studio
- Create a new project for speech recognition
- Select "ESP32" as your target device

### 1.2 Data Collection
Collect audio samples for these voice commands:
- "Hazard mode" - Switch to hazard detection
- "Caption mode" - Switch to visual description  
- "Sign mode" - Switch to sign detection
- "Text mode" or "OCR mode" - Switch to text recognition
- "Auto mode" - Switch to automatic mode
- "Capture" or "Take picture" - Manual image capture
- "Emergency" or "Help" - Emergency alert
- "Status" - System status report
- "Sleep" - Enter sleep mode
- "Wake up" - Exit sleep mode

**Collection Tips:**
- Record 10-20 samples per command
- Include different speakers, accents, and volumes
- Record in various noise conditions
- Each sample should be 1-2 seconds long

### 1.3 Model Configuration
```
Input: Audio (16kHz, 1 second window)
Processing Block: Audio (MFCC)
Learning Block: Classification (Keras)
Output: 10+ classes (commands + noise/unknown)
```

### 1.4 Model Training
- Use default MFCC parameters (good starting point)
- Train with 80/20 train/validation split
- Aim for >95% accuracy on validation set
- Include "noise" and "unknown" classes for robustness

## Step 2: Export Model for ESP32

### 2.1 Download Arduino Library
1. Go to "Deployment" in Edge Impulse Studio
2. Select "Arduino library"
3. Select "ESP32" as optimization target
4. Download the ZIP file

### 2.2 Extract Model Files
```bash
# Extract the downloaded ZIP file
unzip your-project-arduino-1.0.0.zip

# You'll find these important files:
src/
├── edge-impulse-sdk/           # Core Edge Impulse SDK
├── model-parameters/           # Model weights and config
├── tflite-model/              # TensorFlow Lite model
└── your_project_inferencing.h # Main header file
```

## Step 3: Integrate into Intel Glasses Project

### 3.1 Copy Model Files
```bash
# Copy Edge Impulse files to your project
cp -r your-project-arduino-1.0.0/src/* "AioT Intel Glasses/src/"

# The structure should look like:
src/
├── edge-impulse-sdk/
├── model-parameters/
├── tflite-model/
├── your_project_inferencing.h
├── speech_recognition.h
├── speech_recognition.cpp
├── intel_glasses.h
└── ... (other project files)
```

### 3.2 Update Speech Recognition Header
Edit `speech_recognition.h` to include your model:

```cpp
// Replace this line:
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

// With your specific model header:
#include "your_project_inferencing.h"
```

### 3.3 Update Model Configuration
In `speech_recognition.h`, adjust these parameters to match your model:

```cpp
// Update based on your Edge Impulse model settings
#define SAMPLE_RATE         16000    // Should match your model's sample rate
#define INFERENCE_BUFFER_SIZE 16000  // 1 second at 16kHz (adjust if needed)
#define CONFIDENCE_THRESHOLD 0.8     // Adjust based on your model's performance
```

### 3.4 Update Command Mapping
In `speech_recognition.cpp`, update the `getCommandFromText()` function to match your trained labels:

```cpp
SpeechCommand SpeechRecognition::getCommandFromText(const String& text) {
    String lowerText = text;
    lowerText.toLowerCase();
    
    // Update these to match your Edge Impulse model labels exactly
    if (lowerText == "hazard_mode") return CMD_HAZARD_MODE;
    if (lowerText == "caption_mode") return CMD_CAPTION_MODE;
    if (lowerText == "sign_mode") return CMD_SIGN_MODE;
    if (lowerText == "text_mode") return CMD_OCR_MODE;
    if (lowerText == "auto_mode") return CMD_AUTO_MODE;
    if (lowerText == "capture") return CMD_CAPTURE;
    if (lowerText == "emergency") return CMD_EMERGENCY;
    if (lowerText == "status") return CMD_STATUS;
    if (lowerText == "sleep") return CMD_SLEEP;
    if (lowerText == "wake_up") return CMD_WAKE_UP;
    
    return CMD_NONE;
}
```

## Step 4: Hardware Setup

### 4.1 Microphone Wiring
Connect an I2S microphone (e.g., INMP441) to ESP32-S3:

```
INMP441    ESP32-S3
--------   --------
VDD     -> 3.3V
GND     -> GND
L/R     -> GND (for left channel)
WS      -> GPIO 15 (I2S_WS_PIN)
SCK     -> GPIO 14 (I2S_SCK_PIN)
SD      -> GPIO 32 (I2S_SD_PIN)
```

### 4.2 Pin Configuration
Verify pin assignments in `intel_glasses_config.h`:

```cpp
#define I2S_WS_PIN          15        // Word Select (LRC)
#define I2S_SCK_PIN         14        // Serial Clock (BCLK)
#define I2S_SD_PIN          32        // Serial Data (DIN)
```

## Step 5: Build and Test

### 5.1 Build Project
```bash
# Build the project with Edge Impulse integration
pio run
```

### 5.2 Flash and Monitor
```bash
# Flash to device
pio run --target upload

# Start serial monitor
pio device monitor
```

### 5.3 Test Speech Commands
1. Power on the device
2. Wait for "Intel Glasses ready" message
3. Say voice commands clearly:
   - "Hazard mode" → Should switch to hazard detection
   - "Caption mode" → Should switch to visual caption
   - "Capture" → Should take a photo
   - etc.

## Step 6: Optimization and Tuning

### 6.1 Adjust Sensitivity
If commands aren't being recognized:

```cpp
// In speech_recognition.h, lower the threshold:
#define CONFIDENCE_THRESHOLD 0.6  // Instead of 0.8

// In speech_recognition.cpp, adjust voice activity detection:
const float voiceThreshold = 300.0f;  // Lower for quieter environments
```

### 6.2 Improve Accuracy
- Collect more training data for poorly recognized commands
- Add more "noise" samples to reduce false positives
- Retrain model with better balanced dataset
- Test in your actual use environment

### 6.3 Debug Speech Recognition
Enable debugging in `speech_recognition.cpp`:

```cpp
// Add debug output in processAudio():
Serial.printf("Voice activity: %.2f, Threshold: %.2f\n", rms, voiceThreshold);
Serial.printf("Classification results:\n");
for (size_t i = 0; i < result.classification_count; i++) {
    Serial.printf("  %s: %.3f\n", 
                  result.classification[i].label, 
                  result.classification[i].value);
}
```

## Troubleshooting

### Common Issues

1. **No Audio Input**
   - Check microphone wiring
   - Verify I2S pin configuration
   - Test with `testMicrophone()` function

2. **Low Recognition Accuracy**
   - Increase training data
   - Adjust confidence threshold
   - Check for environmental noise

3. **Memory Issues**
   - Ensure PSRAM is enabled
   - Monitor heap usage
   - Reduce model complexity if needed

4. **Build Errors**
   - Check Edge Impulse SDK version compatibility
   - Verify all model files are copied
   - Update PlatformIO libraries

### Debug Commands

```cpp
// In setup(), add debugging:
speechRecognizer.printAudioStats();
speechRecognizer.testMicrophone();

// Monitor classification results:
Serial.printf("Last command: %s (%.2f)\n", 
              speechRecognizer.getCommandText(lastCommand).c_str(),
              lastConfidence);
```

## Performance Optimization

### Memory Management
- Edge Impulse models can be memory-intensive
- Use PSRAM for audio buffers
- Monitor free heap during operation

### Power Consumption
- Speech recognition increases power usage
- Consider sleep modes when not in use
- Optimize inference frequency

### Real-time Performance
- Balance accuracy vs. latency
- Adjust buffer sizes for your use case
- Consider quantized models for faster inference

## Advanced Features

### Multi-language Support
- Train separate models for different languages
- Switch models based on user preference
- Implement language auto-detection

### Continuous Learning
- Collect usage data to improve model
- Implement online learning capabilities
- User-specific adaptation

### Integration with Cloud AI
- Combine local speech recognition with cloud NLP
- Use speech for complex queries
- Implement conversation capabilities

---

**Note**: Replace `your_project` with your actual Edge Impulse project name throughout this guide. The exact file names and configurations will depend on your specific Edge Impulse project setup.
