#ifndef INTEL_GLASSES_H
#define INTEL_GLASSES_H

#include <Arduino.h>
#include "intel_glasses_config.h"
#include "camera_manager.h"
#include "gsm_module.h"
#include "ai_processor.h"
#include "input_handler.h"
#include "display_handler.h"
#include "speech_recognition.h"

// System states
enum SystemState {
    STATE_INITIALIZING,
    STATE_CONNECTING,
    STATE_READY,
    STATE_PROCESSING,
    STATE_ERROR,
    STATE_SLEEPING
};

class IntelGlasses {
private:
    SystemState currentState;
    SystemState previousState;
    unsigned long lastStateChange;
    unsigned long lastHeartbeat;
    unsigned long lastStatusUpdate;
    bool systemReady;
    int bootAttempts;
    
    // Auto-capture timing
    unsigned long lastAutoCapture;
    bool autoCaptureMode;
    
    // Battery monitoring (if available)
    float batteryVoltage;
    int batteryPercentage;
    
    // Performance metrics
    int totalProcessedImages;
    int successfulProcessing;
    float averageProcessingTime;
    
public:
    IntelGlasses();
    
    // System lifecycle
    bool initialize();
    void run();
    void shutdown();
    void restart();
    
    // State management
    void setState(SystemState newState);
    SystemState getState();
    String getStateString();
    bool isSystemReady();
    
    // Operation control
    void captureAndProcess();
    void processManualCapture();
    void processAutoCapture();
    void processSpeechCommand(const SpeechResult& result);
    void toggleAutoCaptureMode();
    void emergencyAlert();
    
    // Mode and settings
    void handleModeChange();
    void handleSpeechModeChange(OperationMode newMode);
    void handleSettingsChange();
    void calibrateSystem();
    
    // Status and diagnostics
    void updateSystemStatus();
    void performSelfTest();
    String getSystemInfo();
    void logPerformanceMetrics();
    
    // Power management
    void enterSleepMode();
    void exitSleepMode();
    void updateBatteryStatus();
    
private:
    bool initializeSubsystems();
    bool testConnectivity();
    void handleSystemError(const String& error);
    void processUserInput();
    void updateSystemMetrics();
    void checkSystemHealth();
    void handleLowBattery();
    
    // Startup sequence
    bool initializeCamera();
    bool initializeGSM();
    bool initializeDisplay();
    bool initializeInput();
    bool initializeSpeechRecognition();
    bool testAllSystems();
    
    // Recovery procedures
    void recoverFromError();
    void resetToDefaults();
    bool attemptReconnection();
};

// Global Intel Glasses instance
extern IntelGlasses glasses;

#endif // INTEL_GLASSES_H
