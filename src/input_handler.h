#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "intel_glasses_config.h"

class InputHandler {
private:
    unsigned long lastCaptureButtonPress;
    unsigned long lastModeButtonPress;
    bool captureButtonState;
    bool modeButtonState;
    bool lastCaptureButtonState;
    bool lastModeButtonState;
    
    // Debounce timing
    static const unsigned long DEBOUNCE_DELAY = 50;
    static const unsigned long LONG_PRESS_DELAY = 1000;
    static const unsigned long DOUBLE_PRESS_DELAY = 300;
    
    // Button press tracking
    unsigned long captureButtonPressStart;
    unsigned long modeButtonPressStart;
    bool captureButtonLongPressed;
    bool modeButtonLongPressed;
    int captureButtonPressCount;
    int modeButtonPressCount;
    unsigned long lastCapturePressTime;
    unsigned long lastModePressTime;
    
public:
    InputHandler();
    
    void initialize();
    void update();
    
    // Button state queries
    bool isCaptureButtonPressed();
    bool isModeButtonPressed();
    bool wasCaptureButtonClicked();
    bool wasCaptureButtonLongPressed();
    bool wasCaptureButtonDoubleClicked();
    bool wasModeButtonClicked();
    bool wasModeButtonLongPressed();
    bool wasModeButtonDoubleClicked();
    
    // Reset button states
    void resetButtonStates();
    
private:
    bool readCaptureButton();
    bool readModeButton();
    void processCaptureButton();
    void processModeButton();
};

// Global input handler instance
extern InputHandler inputHandler;

#endif // INPUT_HANDLER_H
