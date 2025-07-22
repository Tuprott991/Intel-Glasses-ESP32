#ifndef AI_PROCESSOR_H
#define AI_PROCESSOR_H

#include <Arduino.h>
#include "intel_glasses_config.h"
#include "gsm_module.h"
#include "audio_manager.h"

class AIProcessor {
private:
    OperationMode currentMode;
    bool isProcessing;
    unsigned long lastProcessTime;
    int consecutiveFailures;
    
public:
    AIProcessor();
    
    // Core processing methods
    bool processImage(uint8_t* imageData, size_t imageSize);
    bool processHazardDetection(uint8_t* imageData, size_t imageSize);
    bool processVisualCaption(uint8_t* imageData, size_t imageSize);
    bool processSignDetection(uint8_t* imageData, size_t imageSize);
    bool processOCR(uint8_t* imageData, size_t imageSize);
    bool processAutoMode(uint8_t* imageData, size_t imageSize);
    
    // Mode management
    void setOperationMode(OperationMode mode);
    OperationMode getOperationMode();
    void cycleMode();
    
    // Status methods
    bool getProcessingStatus();
    String getCurrentModeString();
    int getConsecutiveFailures();
    void resetFailureCount();
    
    // Feedback methods
    void provideAudioFeedback(const String& message, bool isHazard = false);
    void provideCloudAudioFeedback(const APIResponse& response);
    void provideHapticFeedback(int pattern);
    void updateStatusLEDs(bool processing, bool hazard, bool success);
    
private:
    void handleHazardResponse(const APIResponse& response);
    void handleVisualCaptionResponse(const APIResponse& response);
    void handleSignDetectionResponse(const APIResponse& response);
    void handleOCRResponse(const APIResponse& response);
    
    void speakText(const String& text);
    void playTone(int frequency, int duration);
    void vibrate(int duration, int pattern = 1);
    
    bool isHighConfidence(float confidence);
    bool isHazardDetected(const String& result);
    String formatResultForSpeech(const String& result);
};

// Global AI processor instance
extern AIProcessor aiProcessor;

#endif // AI_PROCESSOR_H
