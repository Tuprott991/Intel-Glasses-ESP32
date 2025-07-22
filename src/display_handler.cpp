#include "display_handler.h"

DisplayHandler displayHandler;

DisplayHandler::DisplayHandler() {
    currentMode = DISPLAY_OFF;
    currentMessage = "";
    lastUpdate = 0;
    displayTimeout = 0;
    isBlinking = false;
    blinkState = false;
    lastBlink = 0;
    networkStatus = "Disconnected";
    processingStatus = "Ready";
    batteryStatus = "Unknown";
    operationMode = MODE_HAZARD_DETECTION;
    signalStrength = 0;
}

void DisplayHandler::initialize() {
    Serial.println("Initializing display handler...");
    
    // In a real implementation, this would initialize an actual display
    // For now, we'll use Serial output to simulate display functionality
    
    clearDisplay();
    showStatus();
    
    Serial.println("Display handler initialized");
}

void DisplayHandler::update() {
    unsigned long currentTime = millis();
    
    // Handle display timeout
    if (displayTimeout > 0 && (currentTime - lastUpdate) >= displayTimeout) {
        showStatus(); // Return to status display
        displayTimeout = 0;
    }
    
    // Handle blinking
    if (isBlinking && (currentTime - lastBlink) >= 500) { // 500ms blink interval
        blinkState = !blinkState;
        lastBlink = currentTime;
        
        // Update display with blink state
        switch (currentMode) {
            case DISPLAY_PROCESSING:
                displayText(blinkState ? processingStatus : "", true);
                break;
            case DISPLAY_STATUS:
                if (blinkState) {
                    displayStatusInfo();
                } else {
                    clearDisplay();
                }
                break;
            default:
                break;
        }
    }
}

void DisplayHandler::showStatus() {
    currentMode = DISPLAY_STATUS;
    displayTimeout = 0; // No timeout for status display
    isBlinking = false;
    lastUpdate = millis();
    
    displayStatusInfo();
}

void DisplayHandler::showProcessing(const String& message) {
    currentMode = DISPLAY_PROCESSING;
    currentMessage = message;
    processingStatus = message;
    displayTimeout = 0; // No timeout while processing
    isBlinking = true;
    lastUpdate = millis();
    
    displayText(message, true);
    Serial.println("DISPLAY: " + message);
}

void DisplayHandler::showResult(const String& result, int displayTime) {
    currentMode = DISPLAY_RESULT;
    currentMessage = result;
    displayTimeout = displayTime;
    isBlinking = false;
    lastUpdate = millis();
    
    displayText(result, true);
    Serial.println("DISPLAY RESULT: " + result);
}

void DisplayHandler::showError(const String& error, int displayTime) {
    currentMode = DISPLAY_ERROR;
    currentMessage = error;
    displayTimeout = displayTime;
    isBlinking = true;
    lastUpdate = millis();
    
    displayText("ERROR: " + error, true);
    Serial.println("DISPLAY ERROR: " + error);
}

void DisplayHandler::turnOff() {
    currentMode = DISPLAY_OFF;
    isBlinking = false;
    clearDisplay();
}

void DisplayHandler::setBrightness(int level) {
    // In a real implementation, this would control display brightness
    Serial.println("Display brightness set to: " + String(level));
}

void DisplayHandler::updateNetworkStatus(const String& status) {
    networkStatus = status;
    if (currentMode == DISPLAY_STATUS) {
        displayStatusInfo();
    }
}

void DisplayHandler::updateProcessingStatus(const String& status) {
    processingStatus = status;
}

void DisplayHandler::updateBatteryStatus(const String& status) {
    batteryStatus = status;
    if (currentMode == DISPLAY_STATUS) {
        displayStatusInfo();
    }
}

void DisplayHandler::updateOperationMode(OperationMode mode) {
    operationMode = mode;
    if (currentMode == DISPLAY_STATUS) {
        displayStatusInfo();
    }
}

void DisplayHandler::updateSignalStrength(int strength) {
    signalStrength = strength;
    if (currentMode == DISPLAY_STATUS) {
        displayStatusInfo();
    }
}

void DisplayHandler::setDisplayMode(DisplayMode mode) {
    currentMode = mode;
}

DisplayMode DisplayHandler::getDisplayMode() {
    return currentMode;
}

void DisplayHandler::enableBlinking(bool enable) {
    isBlinking = enable;
    blinkState = true;
    lastBlink = millis();
}

void DisplayHandler::displayText(const String& text, bool center) {
    // In a real implementation, this would display text on an actual screen
    // For simulation, we use Serial output with formatting
    
    if (center) {
        // Center the text (simulate)
        int padding = (20 - text.length()) / 2; // Assume 20 character width
        if (padding > 0) {
            String spaces = "";
            for (int i = 0; i < padding; i++) {
                spaces += " ";
            }
            Serial.println("[DISPLAY] " + spaces + text);
        } else {
            Serial.println("[DISPLAY] " + text);
        }
    } else {
        Serial.println("[DISPLAY] " + text);
    }
}

void DisplayHandler::displayStatusInfo() {
    // Create a status display showing key information
    String statusLine1 = formatModeString(operationMode);
    String statusLine2 = formatSignalBars(signalStrength) + " " + networkStatus.substring(0, 8);
    String statusLine3 = "Bat: " + batteryStatus;
    
    Serial.println("========== STATUS ==========");
    displayText(statusLine1, true);
    displayText(statusLine2, true);
    displayText(statusLine3, true);
    Serial.println("============================");
}

void DisplayHandler::clearDisplay() {
    // In a real implementation, this would clear the display
    Serial.println("[DISPLAY] Cleared");
}

String DisplayHandler::formatModeString(OperationMode mode) {
    switch (mode) {
        case MODE_HAZARD_DETECTION: return "HAZARD DETECT";
        case MODE_VISUAL_CAPTION: return "VISUAL DESC";
        case MODE_SIGN_DETECTION: return "SIGN DETECT";
        case MODE_OCR: return "TEXT SCAN";
        case MODE_AUTO_ALL: return "AUTO MODE";
        default: return "UNKNOWN";
    }
}

String DisplayHandler::formatSignalBars(int strength) {
    // Convert signal strength to visual bars
    if (strength >= 20) return "||||";      // Excellent
    else if (strength >= 15) return "|||.";  // Good
    else if (strength >= 10) return "||..";  // Fair
    else if (strength >= 5) return "|...";   // Poor
    else return "....";                      // No signal
}
