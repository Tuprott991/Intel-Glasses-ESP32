#include "ai_processor.h"
#include <ArduinoJson.h>

AIProcessor aiProcessor;

AIProcessor::AIProcessor() {
    currentMode = MODE_HAZARD_DETECTION;  // Start with hazard detection as default
    isProcessing = false;
    lastProcessTime = 0;
    consecutiveFailures = 0;
    
    // Initialize feedback pins
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(HAZARD_LED_PIN, OUTPUT);
    pinMode(PROCESSING_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(VIBRATION_PIN, OUTPUT);
    
    // Turn off all LEDs initially
    updateStatusLEDs(false, false, false);
}

bool AIProcessor::processImage(uint8_t* imageData, size_t imageSize) {
    if (isProcessing) {
        Serial.println("Already processing an image, skipping...");
        return false;
    }
    
    if (millis() - lastProcessTime < 1000) { // Prevent rapid fire processing
        return false;
    }
    
    isProcessing = true;
    updateStatusLEDs(true, false, false);
    
    bool success = false;
    
    switch (currentMode) {
        case MODE_HAZARD_DETECTION:
            success = processHazardDetection(imageData, imageSize);
            break;
        case MODE_VISUAL_CAPTION:
            success = processVisualCaption(imageData, imageSize);
            break;
        case MODE_SIGN_DETECTION:
            success = processSignDetection(imageData, imageSize);
            break;
        case MODE_OCR:
            success = processOCR(imageData, imageSize);
            break;
        case MODE_AUTO_ALL:
            success = processAutoMode(imageData, imageSize);
            break;
    }
    
    if (success) {
        consecutiveFailures = 0;
    } else {
        consecutiveFailures++;
        if (consecutiveFailures >= MAX_RETRIES) {
            provideAudioFeedback("Connection error. Please check network.", false);
        }
    }
    
    lastProcessTime = millis();
    isProcessing = false;
    updateStatusLEDs(false, false, success);
    
    return success;
}

bool AIProcessor::processHazardDetection(uint8_t* imageData, size_t imageSize) {
    Serial.println("Processing hazard detection...");
    
    APIResponse response = gsmModule.callHazardDetection(imageData, imageSize);
    
    if (response.success) {
        handleHazardResponse(response);
        return true;
    } else {
        Serial.println("Hazard detection failed: " + response.error);
        return false;
    }
}

bool AIProcessor::processVisualCaption(uint8_t* imageData, size_t imageSize) {
    Serial.println("Processing visual caption...");
    
    APIResponse response = gsmModule.callVisualCaption(imageData, imageSize);
    
    if (response.success) {
        handleVisualCaptionResponse(response);
        return true;
    } else {
        Serial.println("Visual caption failed: " + response.error);
        return false;
    }
}

bool AIProcessor::processSignDetection(uint8_t* imageData, size_t imageSize) {
    Serial.println("Processing sign detection...");
    
    APIResponse response = gsmModule.callSignDetection(imageData, imageSize);
    
    if (response.success) {
        handleSignDetectionResponse(response);
        return true;
    } else {
        Serial.println("Sign detection failed: " + response.error);
        return false;
    }
}

bool AIProcessor::processOCR(uint8_t* imageData, size_t imageSize) {
    Serial.println("Processing OCR...");
    
    APIResponse response = gsmModule.callOCR(imageData, imageSize);
    
    if (response.success) {
        handleOCRResponse(response);
        return true;
    } else {
        Serial.println("OCR failed: " + response.error);
        return false;
    }
}

bool AIProcessor::processAutoMode(uint8_t* imageData, size_t imageSize) {
    Serial.println("Processing auto mode (all features)...");
    
    // In auto mode, we prioritize hazard detection first
    bool hazardSuccess = processHazardDetection(imageData, imageSize);
    delay(500); // Small delay between API calls
    
    bool captionSuccess = processVisualCaption(imageData, imageSize);
    delay(500);
    
    bool signSuccess = processSignDetection(imageData, imageSize);
    delay(500);
    
    bool ocrSuccess = processOCR(imageData, imageSize);
    
    return hazardSuccess || captionSuccess || signSuccess || ocrSuccess;
}

void AIProcessor::setOperationMode(OperationMode mode) {
    currentMode = mode;
    Serial.println("Mode changed to: " + getCurrentModeString());
    provideAudioFeedback("Mode: " + getCurrentModeString(), false);
}

OperationMode AIProcessor::getOperationMode() {
    return currentMode;
}

void AIProcessor::cycleMode() {
    int nextMode = (int)currentMode + 1;
    if (nextMode > MODE_AUTO_ALL) {
        nextMode = MODE_HAZARD_DETECTION;
    }
    setOperationMode((OperationMode)nextMode);
}

bool AIProcessor::getProcessingStatus() {
    return isProcessing;
}

String AIProcessor::getCurrentModeString() {
    switch (currentMode) {
        case MODE_HAZARD_DETECTION: return "Hazard Detection";
        case MODE_VISUAL_CAPTION: return "Visual Caption";
        case MODE_SIGN_DETECTION: return "Sign Detection";
        case MODE_OCR: return "Text Recognition";
        case MODE_AUTO_ALL: return "Auto All Features";
        default: return "Unknown";
    }
}

int AIProcessor::getConsecutiveFailures() {
    return consecutiveFailures;
}

void AIProcessor::resetFailureCount() {
    consecutiveFailures = 0;
}

void AIProcessor::handleHazardResponse(const APIResponse& response) {
    Serial.println("Hazard Detection Result: " + response.result);
    Serial.printf("Confidence: %.2f%%\n", response.confidence * 100);
    
    bool isHazard = isHazardDetected(response.result);
    
    if (isHazard && isHighConfidence(response.confidence)) {
        // High priority hazard alert
        updateStatusLEDs(false, true, true);
        
        // Extract direction if available from the result
        String direction = "";
        String lowerResult = response.result;
        lowerResult.toLowerCase();
        if (lowerResult.indexOf("right") >= 0) direction = "right";
        else if (lowerResult.indexOf("left") >= 0) direction = "left";
        else if (lowerResult.indexOf("front") >= 0) direction = "front";
        else if (lowerResult.indexOf("behind") >= 0) direction = "behind";
        
        // Play local hazard audio with direction
        audioManager.playHazardAlert(response.result, direction);
        provideHapticFeedback(3); // Strong vibration pattern
        
    } else if (isHazard) {
        // Low confidence hazard
        updateStatusLEDs(false, true, true);
        audioManager.playHazardAlert(response.result, "");
        provideHapticFeedback(1); // Light vibration
    } else {
        // No hazard detected
        updateStatusLEDs(false, false, true);
        if (currentMode == MODE_HAZARD_DETECTION) {
            audioManager.playLocalMP3("area_clear.mp3", AUDIO_HAZARD, false);
        }
    }
}

void AIProcessor::handleVisualCaptionResponse(const APIResponse& response) {
    Serial.println("Visual Caption Result: " + response.result);
    Serial.printf("Confidence: %.2f%%\n", response.confidence * 100);
    
    if (isHighConfidence(response.confidence)) {
        updateStatusLEDs(false, false, true);
        
        // Check if cloud provided audio
        if (response.hasAudio && response.audioUrl.length() > 0) {
            Serial.println("Playing cloud audio for visual caption");
            audioManager.playCloudAudio(response.audioUrl, AUDIO_CAPTION, false);
        } else {
            // Fallback to text-based audio feedback
            String caption = formatResultForSpeech(response.result);
            provideAudioFeedback("I see: " + caption, false);
        }
    } else {
        audioManager.playLocalMP3("caption_unclear.mp3", AUDIO_CAPTION, false);
    }
}

void AIProcessor::handleSignDetectionResponse(const APIResponse& response) {
    Serial.println("Sign Detection Result: " + response.result);
    Serial.printf("Confidence: %.2f%%\n", response.confidence * 100);
    
    if (response.result.length() > 0 && isHighConfidence(response.confidence)) {
        updateStatusLEDs(false, false, true);
        
        // Check if cloud provided audio
        if (response.hasAudio && response.audioUrl.length() > 0) {
            Serial.println("Playing cloud audio for sign detection");
            audioManager.playCloudAudio(response.audioUrl, AUDIO_SIGN, false);
        } else {
            // Fallback to local audio
            String sign = formatResultForSpeech(response.result);
            provideAudioFeedback("Sign detected: " + sign, false);
        }
        
        // Check if it's a warning sign
        if (response.result.indexOf("warning") >= 0 || 
            response.result.indexOf("danger") >= 0 || 
            response.result.indexOf("caution") >= 0 ||
            response.result.indexOf("stop") >= 0) {
            provideHapticFeedback(2); // Medium vibration for warning signs
        }
    } else {
        if (currentMode == MODE_SIGN_DETECTION) {
            audioManager.playLocalMP3("no_signs.mp3", AUDIO_SIGN, false);
        }
    }
}

void AIProcessor::handleOCRResponse(const APIResponse& response) {
    Serial.println("OCR Result: " + response.result);
    Serial.printf("Confidence: %.2f%%\n", response.confidence * 100);
    
    if (response.result.length() > 0 && isHighConfidence(response.confidence)) {
        updateStatusLEDs(false, false, true);
        
        // Check if cloud provided audio
        if (response.hasAudio && response.audioUrl.length() > 0) {
            Serial.println("Playing cloud audio for OCR text");
            audioManager.playCloudAudio(response.audioUrl, AUDIO_OCR, false);
        } else {
            // Fallback to text-based audio feedback
            String text = formatResultForSpeech(response.result);
            provideAudioFeedback("Text found: " + text, false);
        }
    } else {
        if (currentMode == MODE_OCR) {
            audioManager.playLocalMP3("no_text.mp3", AUDIO_OCR, false);
        }
    }
}

void AIProcessor::provideAudioFeedback(const String& message, bool isHazard) {
    // This is now primarily used for system messages and fallbacks
    // Most content audio comes from the cloud
    Serial.println("AUDIO: " + message);
    
    if (isHazard) {
        // Use audio manager for hazard alerts
        audioManager.playHazardAlert("general", "");
    } else {
        // Play simple confirmation tone
        playTone(800, 150);
    }
    
    // TODO: Could implement local TTS as fallback if needed
}

void AIProcessor::provideCloudAudioFeedback(const APIResponse& response) {
    // Play audio received from cloud API
    if (response.hasAudio && response.audioUrl.length() > 0) {
        Serial.println("Playing cloud audio: " + response.audioUrl);
        
        AudioCategory category = AUDIO_CAPTION; // Default
        
        // Determine audio category based on current mode
        switch (currentMode) {
            case MODE_HAZARD_DETECTION:
                category = AUDIO_HAZARD;
                break;
            case MODE_VISUAL_CAPTION:
                category = AUDIO_CAPTION;
                break;
            case MODE_SIGN_DETECTION:
                category = AUDIO_SIGN;
                break;
            case MODE_OCR:
                category = AUDIO_OCR;
                break;
            default:
                category = AUDIO_CAPTION;
                break;
        }
        
        bool priority = (category == AUDIO_HAZARD); // Hazards are high priority
        audioManager.playCloudAudio(response.audioUrl, category, priority);
    } else {
        // Fallback to text-based feedback
        provideAudioFeedback(response.result, false);
    }
}

void AIProcessor::provideHapticFeedback(int pattern) {
    switch (pattern) {
        case 1: // Light vibration
            vibrate(200, 1);
            break;
        case 2: // Medium vibration
            vibrate(400, 2);
            break;
        case 3: // Strong vibration pattern
            for (int i = 0; i < 3; i++) {
                vibrate(300, 1);
                delay(200);
            }
            break;
        default:
            vibrate(100, 1);
            break;
    }
}

void AIProcessor::updateStatusLEDs(bool processing, bool hazard, bool success) {
    digitalWrite(PROCESSING_LED_PIN, processing ? HIGH : LOW);
    digitalWrite(HAZARD_LED_PIN, hazard ? HIGH : LOW);
    
    if (processing) {
        // Blinking status LED during processing
        digitalWrite(STATUS_LED_PIN, (millis() / 250) % 2);
    } else {
        digitalWrite(STATUS_LED_PIN, success ? HIGH : LOW);
    }
}

void AIProcessor::playTone(int frequency, int duration) {
    // Simple tone generation using PWM
    ledcSetup(0, frequency, 8);
    ledcAttachPin(BUZZER_PIN, 0);
    ledcWrite(0, 128);
    delay(duration);
    ledcWrite(0, 0);
    ledcDetachPin(BUZZER_PIN);
}

void AIProcessor::vibrate(int duration, int pattern) {
    for (int i = 0; i < pattern; i++) {
        digitalWrite(VIBRATION_PIN, HIGH);
        delay(duration / pattern);
        digitalWrite(VIBRATION_PIN, LOW);
        if (i < pattern - 1) {
            delay(100); // Short pause between pulses
        }
    }
}

bool AIProcessor::isHighConfidence(float confidence) {
    return confidence >= 0.7; // 70% confidence threshold
}

bool AIProcessor::isHazardDetected(const String& result) {
    // Check for hazard keywords in the result
    String lowerResult = result;
    lowerResult.toLowerCase();
    
    return (lowerResult.indexOf("hazard") >= 0 ||
            lowerResult.indexOf("danger") >= 0 ||
            lowerResult.indexOf("warning") >= 0 ||
            lowerResult.indexOf("obstacle") >= 0 ||
            lowerResult.indexOf("fire") >= 0 ||
            lowerResult.indexOf("caution") >= 0 ||
            lowerResult.indexOf("risk") >= 0 ||
            lowerResult.indexOf("unsafe") >= 0);
}

String AIProcessor::formatResultForSpeech(const String& result) {
    // Clean up the result for better speech synthesis
    String formatted = result;
    
    // Replace common abbreviations and symbols
    formatted.replace("&", " and ");
    formatted.replace("%", " percent");
    formatted.replace("$", " dollar");
    formatted.replace("@", " at ");
    formatted.replace("#", " number ");
    
    // Remove extra whitespace
    while (formatted.indexOf("  ") >= 0) {
        formatted.replace("  ", " ");
    }
    
    formatted.trim();
    return formatted;
}
