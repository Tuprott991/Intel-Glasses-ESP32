#include "input_handler.h"

InputHandler inputHandler;

InputHandler::InputHandler() {
    lastCaptureButtonPress = 0;
    lastModeButtonPress = 0;
    captureButtonState = false;
    modeButtonState = false;
    lastCaptureButtonState = false;
    lastModeButtonState = false;
    captureButtonPressStart = 0;
    modeButtonPressStart = 0;
    captureButtonLongPressed = false;
    modeButtonLongPressed = false;
    captureButtonPressCount = 0;
    modeButtonPressCount = 0;
    lastCapturePressTime = 0;
    lastModePressTime = 0;
}

void InputHandler::initialize() {
    Serial.println("Initializing input handler...");
    
    // Configure button pins with internal pull-up resistors
    pinMode(CAPTURE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    
    // Read initial states
    lastCaptureButtonState = readCaptureButton();
    lastModeButtonState = readModeButton();
    
    Serial.println("Input handler initialized");
    Serial.println("Capture button: Pin " + String(CAPTURE_BUTTON_PIN));
    Serial.println("Mode button: Pin " + String(MODE_BUTTON_PIN));
}

void InputHandler::update() {
    processCaptureButton();
    processModeButton();
}

bool InputHandler::readCaptureButton() {
    // Button is active LOW (pressed = LOW due to pull-up)
    return !digitalRead(CAPTURE_BUTTON_PIN);
}

bool InputHandler::readModeButton() {
    // Button is active LOW (pressed = LOW due to pull-up)
    return !digitalRead(MODE_BUTTON_PIN);
}

void InputHandler::processCaptureButton() {
    bool currentState = readCaptureButton();
    unsigned long currentTime = millis();
    
    // Debounce
    if (currentState != lastCaptureButtonState) {
        lastCaptureButtonPress = currentTime;
    }
    
    if ((currentTime - lastCaptureButtonPress) > DEBOUNCE_DELAY) {
        // Button state has been stable for debounce period
        if (currentState != captureButtonState) {
            captureButtonState = currentState;
            
            if (captureButtonState) {
                // Button just pressed
                captureButtonPressStart = currentTime;
                captureButtonLongPressed = false;
                
                // Check for double click
                if ((currentTime - lastCapturePressTime) < DOUBLE_PRESS_DELAY) {
                    captureButtonPressCount = 2; // Double click
                } else {
                    captureButtonPressCount = 1; // Single click
                }
                lastCapturePressTime = currentTime;
                
            } else {
                // Button just released
                if ((currentTime - captureButtonPressStart) >= LONG_PRESS_DELAY && 
                    !captureButtonLongPressed) {
                    captureButtonLongPressed = true;
                }
            }
        } else if (captureButtonState && !captureButtonLongPressed) {
            // Check for long press while button is held
            if ((currentTime - captureButtonPressStart) >= LONG_PRESS_DELAY) {
                captureButtonLongPressed = true;
            }
        }
    }
    
    lastCaptureButtonState = currentState;
}

void InputHandler::processModeButton() {
    bool currentState = readModeButton();
    unsigned long currentTime = millis();
    
    // Debounce
    if (currentState != lastModeButtonState) {
        lastModeButtonPress = currentTime;
    }
    
    if ((currentTime - lastModeButtonPress) > DEBOUNCE_DELAY) {
        // Button state has been stable for debounce period
        if (currentState != modeButtonState) {
            modeButtonState = currentState;
            
            if (modeButtonState) {
                // Button just pressed
                modeButtonPressStart = currentTime;
                modeButtonLongPressed = false;
                
                // Check for double click
                if ((currentTime - lastModePressTime) < DOUBLE_PRESS_DELAY) {
                    modeButtonPressCount = 2; // Double click
                } else {
                    modeButtonPressCount = 1; // Single click
                }
                lastModePressTime = currentTime;
                
            } else {
                // Button just released
                if ((currentTime - modeButtonPressStart) >= LONG_PRESS_DELAY && 
                    !modeButtonLongPressed) {
                    modeButtonLongPressed = true;
                }
            }
        } else if (modeButtonState && !modeButtonLongPressed) {
            // Check for long press while button is held
            if ((currentTime - modeButtonPressStart) >= LONG_PRESS_DELAY) {
                modeButtonLongPressed = true;
            }
        }
    }
    
    lastModeButtonState = currentState;
}

bool InputHandler::isCaptureButtonPressed() {
    return captureButtonState;
}

bool InputHandler::isModeButtonPressed() {
    return modeButtonState;
}

bool InputHandler::wasCaptureButtonClicked() {
    if (captureButtonPressCount == 1 && !captureButtonState && !captureButtonLongPressed) {
        captureButtonPressCount = 0;
        return true;
    }
    return false;
}

bool InputHandler::wasCaptureButtonLongPressed() {
    if (captureButtonLongPressed && captureButtonState) {
        captureButtonLongPressed = false; // Reset after reading
        return true;
    }
    return false;
}

bool InputHandler::wasCaptureButtonDoubleClicked() {
    if (captureButtonPressCount == 2 && !captureButtonState) {
        captureButtonPressCount = 0;
        return true;
    }
    return false;
}

bool InputHandler::wasModeButtonClicked() {
    if (modeButtonPressCount == 1 && !modeButtonState && !modeButtonLongPressed) {
        modeButtonPressCount = 0;
        return true;
    }
    return false;
}

bool InputHandler::wasModeButtonLongPressed() {
    if (modeButtonLongPressed && modeButtonState) {
        modeButtonLongPressed = false; // Reset after reading
        return true;
    }
    return false;
}

bool InputHandler::wasModeButtonDoubleClicked() {
    if (modeButtonPressCount == 2 && !modeButtonState) {
        modeButtonPressCount = 0;
        return true;
    }
    return false;
}

void InputHandler::resetButtonStates() {
    captureButtonPressCount = 0;
    modeButtonPressCount = 0;
    captureButtonLongPressed = false;
    modeButtonLongPressed = false;
}
