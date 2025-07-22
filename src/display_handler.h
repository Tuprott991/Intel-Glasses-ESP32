#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include "intel_glasses_config.h"

// Status display modes
enum DisplayMode {
    DISPLAY_OFF,
    DISPLAY_STATUS,
    DISPLAY_PROCESSING,
    DISPLAY_RESULT,
    DISPLAY_ERROR
};

class DisplayHandler {
private:
    DisplayMode currentMode;
    String currentMessage;
    unsigned long lastUpdate;
    unsigned long displayTimeout;
    bool isBlinking;
    bool blinkState;
    unsigned long lastBlink;
    
    // Status information
    String networkStatus;
    String processingStatus;
    String batteryStatus;
    OperationMode operationMode;
    int signalStrength;
    
public:
    DisplayHandler();
    
    void initialize();
    void update();
    
    // Display control
    void showStatus();
    void showProcessing(const String& message = "Processing...");
    void showResult(const String& result, int displayTime = 5000);
    void showError(const String& error, int displayTime = 3000);
    void turnOff();
    void setBrightness(int level); // 0-255
    
    // Status updates
    void updateNetworkStatus(const String& status);
    void updateProcessingStatus(const String& status);
    void updateBatteryStatus(const String& status);
    void updateOperationMode(OperationMode mode);
    void updateSignalStrength(int strength);
    
    // Display modes
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode();
    void enableBlinking(bool enable);
    
private:
    void displayText(const String& text, bool center = true);
    void displayStatusInfo();
    void clearDisplay();
    String formatModeString(OperationMode mode);
    String formatSignalBars(int strength);
};

// Global display handler instance
extern DisplayHandler displayHandler;

#endif // DISPLAY_HANDLER_H
