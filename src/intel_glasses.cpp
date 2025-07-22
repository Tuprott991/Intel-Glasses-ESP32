#include "intel_glasses.h"

IntelGlasses glasses;

IntelGlasses::IntelGlasses() {
    currentState = STATE_INITIALIZING;
    previousState = STATE_INITIALIZING;
    lastStateChange = 0;
    lastHeartbeat = 0;
    lastStatusUpdate = 0;
    systemReady = false;
    bootAttempts = 0;
    lastAutoCapture = 0;
    autoCaptureMode = true; // Start with auto-capture enabled for glasses
    batteryVoltage = 0.0;
    batteryPercentage = 100;
    totalProcessedImages = 0;
    successfulProcessing = 0;
    averageProcessingTime = 0.0;
}

bool IntelGlasses::initialize() {
    Serial.println("=== INTEL GLASSES INITIALIZING ===");
    Serial.println("Boot attempt: " + String(++bootAttempts));
    
    setState(STATE_INITIALIZING);
    
    // Show boot message
    Serial.println("Intel AI Glasses v1.0");
    Serial.println("Hazard Detection | Visual Caption | Sign Recognition | OCR");
    
    // Initialize all subsystems
    if (!initializeSubsystems()) {
        handleSystemError("Subsystem initialization failed");
        return false;
    }
    
    // Test system connectivity
    setState(STATE_CONNECTING);
    if (!testConnectivity()) {
        handleSystemError("Connectivity test failed");
        return false;
    }
    
    // Perform self-test
    if (!testAllSystems()) {
        handleSystemError("System self-test failed");
        return false;
    }
    
    setState(STATE_READY);
    systemReady = true;
    
    Serial.println("=== INTEL GLASSES READY ===");
    Serial.println("System initialized successfully!");
    Serial.println("Mode: " + aiProcessor.getCurrentModeString());
    Serial.println("Auto-capture: " + String(autoCaptureMode ? "ENABLED" : "DISABLED"));
    
    // Initial status update
    updateSystemStatus();
    
    // Provide startup feedback
    audioManager.playSystemAudio("system_ready");
    delay(800); // Allow ready message to play
    audioManager.playSystemAudio("hazard_mode"); // Default mode is hazard detection
    aiProcessor.updateStatusLEDs(false, false, true);
    
    return true;
}

void IntelGlasses::run() {
    // Main system loop
    if (!systemReady && currentState != STATE_INITIALIZING) {
        if (bootAttempts < MAX_RETRIES) {
            Serial.println("Attempting system recovery...");
            if (initialize()) {
                return;
            }
        } else {
            handleSystemError("Maximum boot attempts exceeded");
            return;
        }
    }
    
    // Process user input (buttons and speech)
    processUserInput();
    
    // Update subsystems
    inputHandler.update();
    displayHandler.update();
    speechRecognizer.update();  // Process speech recognition
    
    // Handle auto-capture mode
    if (autoCaptureMode && currentState == STATE_READY) {
        processAutoCapture();
    }
    
    // Periodic status updates
    unsigned long currentTime = millis();
    if (currentTime - lastStatusUpdate >= 10000) { // Every 10 seconds
        updateSystemStatus();
        lastStatusUpdate = currentTime;
    }
    
    // Heartbeat
    if (currentTime - lastHeartbeat >= 30000) { // Every 30 seconds
        Serial.println("Heartbeat - System operational");
        checkSystemHealth();
        lastHeartbeat = currentTime;
    }
    
    // Battery monitoring
    if (currentTime % 60000 == 0) { // Every minute
        updateBatteryStatus();
    }
}

void IntelGlasses::processUserInput() {
    // Handle capture button
    if (inputHandler.wasCaptureButtonClicked()) {
        Serial.println("Capture button clicked");
        processManualCapture();
    } else if (inputHandler.wasCaptureButtonLongPressed()) {
        Serial.println("Capture button long pressed - toggling auto mode");
        toggleAutoCaptureMode();
    } else if (inputHandler.wasCaptureButtonDoubleClicked()) {
        Serial.println("Capture button double clicked - emergency alert");
        emergencyAlert();
    }
    
    // Handle mode button
    if (inputHandler.wasModeButtonClicked()) {
        Serial.println("Mode button clicked");
        handleModeChange();
    } else if (inputHandler.wasModeButtonLongPressed()) {
        Serial.println("Mode button long pressed - system info");
        Serial.println(getSystemInfo());
        aiProcessor.provideAudioFeedback("System status: " + String(successfulProcessing) + " successful scans", false);
    } else if (inputHandler.wasModeButtonDoubleClicked()) {
        Serial.println("Mode button double clicked - calibration");
        calibrateSystem();
    }
}

void IntelGlasses::processManualCapture() {
    if (currentState == STATE_PROCESSING) {
        Serial.println("Already processing, ignoring manual capture");
        return;
    }
    
    captureAndProcess();
}

void IntelGlasses::processAutoCapture() {
    if (cameraManager.shouldAutoCapture() && currentState == STATE_READY) {
        captureAndProcess();
    }
}

void IntelGlasses::captureAndProcess() {
    if (currentState != STATE_READY) {
        Serial.println("System not ready for capture");
        return;
    }
    
    setState(STATE_PROCESSING);
    displayHandler.showProcessing("Capturing...");
    
    unsigned long processingStart = millis();
    
    // Capture image
    uint8_t* imageData = nullptr;
    size_t imageSize = 0;
    
    if (!cameraManager.captureToBuffer(&imageData, &imageSize)) {
        Serial.println("Failed to capture image");
        displayHandler.showError("Capture failed", 2000);
        setState(STATE_READY);
        return;
    }
    
    Serial.printf("Image captured: %d bytes\n", imageSize);
    displayHandler.showProcessing("Processing with AI...");
    
    // Process with AI
    bool success = aiProcessor.processImage(imageData, imageSize);
    
    // Update metrics
    totalProcessedImages++;
    if (success) {
        successfulProcessing++;
        unsigned long processingTime = millis() - processingStart;
        averageProcessingTime = (averageProcessingTime * (successfulProcessing - 1) + processingTime) / successfulProcessing;
        
        displayHandler.showResult("Analysis complete", 3000);
    } else {
        displayHandler.showError("Analysis failed", 2000);
    }
    
    // Clean up
    if (imageData) {
        free(imageData);
    }
    
    setState(STATE_READY);
    
    Serial.printf("Processing complete. Success: %s, Time: %lu ms\n", 
                  success ? "YES" : "NO", millis() - processingStart);
}

void IntelGlasses::handleModeChange() {
    aiProcessor.cycleMode();
    displayHandler.updateOperationMode(aiProcessor.getOperationMode());
    displayHandler.showResult("Mode: " + aiProcessor.getCurrentModeString(), 2000);
    
    // Play mode change audio notification
    audioManager.playSystemAudio("mode_change");
    delay(300); // Brief pause
    
    // Play mode-specific audio
    OperationMode currentMode = aiProcessor.getOperationMode();
    switch(currentMode) {
        case OP_MODE_HAZARD_DETECTION:
            audioManager.playSystemAudio("hazard_mode");
            break;
        case OP_MODE_VISUAL_CAPTION:
            audioManager.playSystemAudio("caption_mode");
            break;
        case OP_MODE_SIGN_DETECTION:
            audioManager.playSystemAudio("sign_mode");
            break;
        case OP_MODE_OCR:
            audioManager.playSystemAudio("ocr_mode");
            break;
    }
}

void IntelGlasses::handleSpeechModeChange(OperationMode newMode) {
    aiProcessor.setOperationMode(newMode);
    displayHandler.updateOperationMode(aiProcessor.getOperationMode());
    displayHandler.showResult("Voice: " + aiProcessor.getCurrentModeString(), 2000);
    
    // Play voice command confirmation
    audioManager.playSystemAudio("voice_command");
    delay(300); // Brief pause
    
    // Play mode-specific audio
    switch(newMode) {
        case OP_MODE_HAZARD_DETECTION:
            audioManager.playSystemAudio("hazard_mode");
            break;
        case OP_MODE_VISUAL_CAPTION:
            audioManager.playSystemAudio("caption_mode");
            break;
        case OP_MODE_SIGN_DETECTION:
            audioManager.playSystemAudio("sign_mode");
            break;
        case OP_MODE_OCR:
            audioManager.playSystemAudio("ocr_mode");
            break;
    }
}

void IntelGlasses::processSpeechCommand(const SpeechResult& result) {
    if (!result.isValid || currentState == STATE_ERROR) {
        return;
    }
    
    Serial.printf("Processing speech command: %s (%.2f confidence)\n", 
                  result.commandText.c_str(), result.confidence);
    
    switch (result.command) {
        case CMD_HAZARD_MODE:
            handleSpeechModeChange(MODE_HAZARD_DETECTION);
            break;
            
        case CMD_CAPTION_MODE:
            handleSpeechModeChange(MODE_VISUAL_CAPTION);
            break;
            
        case CMD_SIGN_MODE:
            handleSpeechModeChange(MODE_SIGN_DETECTION);
            break;
            
        case CMD_OCR_MODE:
            handleSpeechModeChange(MODE_OCR);
            break;
            
        case CMD_AUTO_MODE:
            handleSpeechModeChange(MODE_AUTO_ALL);
            break;
            
        case CMD_CAPTURE:
            Serial.println("Voice command: Manual capture");
            processManualCapture();
            break;
            
        case CMD_EMERGENCY:
            Serial.println("Voice command: Emergency alert");
            emergencyAlert();
            break;
            
        case CMD_STATUS:
            Serial.println("Voice command: System status");
            String statusMsg = "System status: " + String(successfulProcessing) + " successful scans, " +
                              String(batteryPercentage) + "% battery, " +
                              (gsmModule.isNetworkConnected() ? "connected" : "disconnected");
            aiProcessor.provideAudioFeedback(statusMsg, false);
            displayHandler.showResult("Status reported", 2000);
            break;
            
        case CMD_SLEEP:
            Serial.println("Voice command: Sleep mode");
            enterSleepMode();
            break;
            
        case CMD_WAKE_UP:
            Serial.println("Voice command: Wake up");
            exitSleepMode();
            break;
            
        default:
            Serial.println("Unknown speech command");
            break;
    }
}

void IntelGlasses::toggleAutoCaptureMode() {
    autoCaptureMode = !autoCaptureMode;
    cameraManager.enableAutoCaptureMode(autoCaptureMode);
    
    String message = "Auto capture " + String(autoCaptureMode ? "enabled" : "disabled");
    Serial.println(message);
    aiProcessor.provideAudioFeedback(message, false);
    displayHandler.showResult(message, 2000);
}

void IntelGlasses::emergencyAlert() {
    Serial.println("EMERGENCY ALERT ACTIVATED!");
    
    // Flash hazard LED
    for (int i = 0; i < 10; i++) {
        aiProcessor.updateStatusLEDs(false, true, false);
        delay(100);
        aiProcessor.updateStatusLEDs(false, false, false);
        delay(100);
    }
    
    // Provide strong feedback
    aiProcessor.provideAudioFeedback("Emergency alert activated", true);
    aiProcessor.provideHapticFeedback(3);
    displayHandler.showError("EMERGENCY ALERT", 5000);
    
    // TODO: Send emergency message via 4G
    // This would send GPS location and emergency status to a monitoring service
}

void IntelGlasses::calibrateSystem() {
    Serial.println("Starting system calibration...");
    displayHandler.showProcessing("Calibrating...");
    
    // Calibrate camera settings
    cameraManager.setupDefaultSettings();
    
    // Test network connection
    if (!gsmModule.isNetworkConnected()) {
        gsmModule.connectToNetwork();
    }
    
    // Reset AI processor
    aiProcessor.resetFailureCount();
    
    displayHandler.showResult("Calibration complete", 2000);
    aiProcessor.provideAudioFeedback("System calibrated", false);
}

void IntelGlasses::updateSystemStatus() {
    // Update display with current status
    String networkStatus = gsmModule.isNetworkConnected() ? "Connected" : "Disconnected";
    int signalStrength = 0; // Get from GSM module
    
    displayHandler.updateNetworkStatus(networkStatus);
    displayHandler.updateSignalStrength(signalStrength);
    displayHandler.updateBatteryStatus(String(batteryPercentage) + "%");
    displayHandler.updateOperationMode(aiProcessor.getOperationMode());
    
    if (displayHandler.getDisplayMode() == DISPLAY_STATUS) {
        displayHandler.showStatus();
    }
}

void IntelGlasses::setState(SystemState newState) {
    if (newState != currentState) {
        previousState = currentState;
        currentState = newState;
        lastStateChange = millis();
        
        Serial.println("State changed: " + getStateString());
    }
}

SystemState IntelGlasses::getState() {
    return currentState;
}

String IntelGlasses::getStateString() {
    switch (currentState) {
        case STATE_INITIALIZING: return "INITIALIZING";
        case STATE_CONNECTING: return "CONNECTING";
        case STATE_READY: return "READY";
        case STATE_PROCESSING: return "PROCESSING";
        case STATE_ERROR: return "ERROR";
        case STATE_SLEEPING: return "SLEEPING";
        default: return "UNKNOWN";
    }
}

bool IntelGlasses::isSystemReady() {
    return systemReady && currentState == STATE_READY;
}

String IntelGlasses::getSystemInfo() {
    String info = "=== INTEL GLASSES SYSTEM INFO ===\n";
    info += "State: " + getStateString() + "\n";
    info += "Mode: " + aiProcessor.getCurrentModeString() + "\n";
    info += "Auto-capture: " + String(autoCaptureMode ? "ON" : "OFF") + "\n";
    info += "Images processed: " + String(totalProcessedImages) + "\n";
    info += "Success rate: " + String((float)successfulProcessing / totalProcessedImages * 100) + "%\n";
    info += "Avg processing time: " + String(averageProcessingTime) + "ms\n";
    info += "Battery: " + String(batteryPercentage) + "%\n";
    info += "Network: " + String(gsmModule.isNetworkConnected() ? "Connected" : "Disconnected") + "\n";
    info += "Camera captures: " + String(cameraManager.getCaptureCount()) + "\n";
    info += "Boot attempts: " + String(bootAttempts) + "\n";
    info += "=================================";
    return info;
}

bool IntelGlasses::initializeSubsystems() {
    Serial.println("Initializing subsystems...");
    
    // Initialize input handler first
    if (!initializeInput()) return false;
    
    // Initialize display
    if (!initializeDisplay()) return false;
    
    // Initialize speech recognition
    if (!initializeSpeechRecognition()) return false;
    
    // Initialize camera
    if (!initializeCamera()) return false;
    
    // Initialize GSM module
    if (!initializeGSM()) return false;
    
    Serial.println("All subsystems initialized");
    return true;
}

bool IntelGlasses::initializeInput() {
    inputHandler.initialize();
    delay(100);
    return true; // Input handler doesn't fail
}

bool IntelGlasses::initializeDisplay() {
    displayHandler.initialize();
    displayHandler.showProcessing("Booting...");
    delay(500);
    return true; // Display handler doesn't fail
}

bool IntelGlasses::initializeCamera() {
    displayHandler.showProcessing("Init Camera...");
    bool success = cameraManager.initialize();
    if (!success) {
        Serial.println("Camera initialization failed");
        return false;
    }
    delay(500);
    return true;
}

bool IntelGlasses::initializeSpeechRecognition() {
    displayHandler.showProcessing("Init Speech...");
    
    if (!speechRecognizer.initialize()) {
        Serial.println("Speech recognition initialization failed");
        return false;
    }
    
    // Start listening for commands
    speechRecognizer.startListening();
    
    delay(500);
    return true;
}

bool IntelGlasses::initializeGSM() {
    displayHandler.showProcessing("Init Network...");
    
    if (!gsmModule.initialize()) {
        Serial.println("GSM module initialization failed");
        return false;
    }
    
    displayHandler.showProcessing("Connecting...");
    if (!gsmModule.connectToNetwork()) {
        Serial.println("Network connection failed");
        return false;
    }
    
    delay(1000);
    return true;
}

bool IntelGlasses::testConnectivity() {
    Serial.println("Testing connectivity...");
    displayHandler.showProcessing("Testing Network...");
    
    return gsmModule.isNetworkConnected();
}

bool IntelGlasses::testAllSystems() {
    Serial.println("Performing system self-test...");
    displayHandler.showProcessing("Self Test...");
    
    // Test camera
    if (!cameraManager.isReady()) {
        Serial.println("Camera self-test failed");
        return false;
    }
    
    // Test network
    if (!gsmModule.isNetworkConnected()) {
        Serial.println("Network self-test failed");
        return false;
    }
    
    // Test a small image capture
    camera_fb_t* testFrame = cameraManager.captureImage();
    if (!testFrame) {
        Serial.println("Test image capture failed");
        return false;
    }
    cameraManager.releaseFrameBuffer(testFrame);
    
    Serial.println("All systems passed self-test");
    return true;
}

void IntelGlasses::handleSystemError(const String& error) {
    setState(STATE_ERROR);
    Serial.println("SYSTEM ERROR: " + error);
    
    displayHandler.showError(error, 5000);
    aiProcessor.provideAudioFeedback("System error", true);
    aiProcessor.updateStatusLEDs(false, true, false);
    
    systemReady = false;
    
    // Attempt recovery after delay
    delay(2000);
    recoverFromError();
}

void IntelGlasses::recoverFromError() {
    Serial.println("Attempting error recovery...");
    
    // Try to reinitialize failed components
    if (!cameraManager.isReady()) {
        cameraManager.initialize();
    }
    
    if (!gsmModule.isNetworkConnected()) {
        gsmModule.connectToNetwork();
    }
    
    // If recovery successful, return to ready state
    if (cameraManager.isReady() && gsmModule.isNetworkConnected()) {
        setState(STATE_READY);
        systemReady = true;
        Serial.println("Error recovery successful");
    } else {
        Serial.println("Error recovery failed");
    }
}

void IntelGlasses::checkSystemHealth() {
    // Monitor system health and performance
    if (aiProcessor.getConsecutiveFailures() >= MAX_RETRIES) {
        handleSystemError("Too many consecutive AI processing failures");
    }
    
    if (batteryPercentage < 10) {
        handleLowBattery();
    }
    
    // Check memory usage
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    if (ESP.getFreeHeap() < 10000) { // Less than 10KB free
        Serial.println("Warning: Low memory");
    }
}

void IntelGlasses::updateBatteryStatus() {
    // TODO: Implement actual battery voltage reading
    // For simulation, slowly decrease battery
    static int counter = 0;
    counter++;
    if (counter % 60 == 0) { // Every hour (approximately)
        batteryPercentage = max(0, batteryPercentage - 1);
    }
}

void IntelGlasses::handleLowBattery() {
    Serial.println("WARNING: Low battery!");
    aiProcessor.provideAudioFeedback("Low battery warning", true);
    displayHandler.showError("Low Battery: " + String(batteryPercentage) + "%", 3000);
    
    // Reduce power consumption
    if (batteryPercentage < 5) {
        enterSleepMode();
    }
}

void IntelGlasses::enterSleepMode() {
    setState(STATE_SLEEPING);
    Serial.println("Entering sleep mode to conserve battery");
    
    // Turn off non-essential systems
    displayHandler.turnOff();
    aiProcessor.updateStatusLEDs(false, false, false);
    cameraManager.enableAutoCaptureMode(false);
    
    // TODO: Implement deep sleep mode
}

void IntelGlasses::exitSleepMode() {
    Serial.println("Exiting sleep mode");
    setState(STATE_READY);
    
    // Re-enable systems
    displayHandler.showStatus();
    cameraManager.enableAutoCaptureMode(autoCaptureMode);
}

void IntelGlasses::shutdown() {
    Serial.println("Shutting down Intel Glasses...");
    
    setState(STATE_INITIALIZING);
    systemReady = false;
    
    // Shutdown all subsystems
    displayHandler.showProcessing("Shutting down...");
    delay(1000);
    
    cameraManager.deinitialize();
    gsmModule.disconnect();
    displayHandler.turnOff();
    aiProcessor.updateStatusLEDs(false, false, false);
    
    Serial.println("Shutdown complete");
}

void IntelGlasses::restart() {
    Serial.println("Restarting system...");
    shutdown();
    delay(2000);
    ESP.restart();
}

// Global function to handle speech commands (called from speech recognition module)
void handleSpeechCommand(const SpeechResult& result) {
    glasses.processSpeechCommand(result);
}
