/*
 * Intel Glasses - AI-Powered Smart Glasses
 * Features: Hazard Detection, Visual Captioning, Sign Recognition, OCR
 * Communication: 4G Module for Cloud AI Processing
 * 
 * Hardware: ESP32-S3-CAM with 4G module
 * 
 * Controls:
 * - Capture Button: Single click = Manual capture, Long press = Toggle auto mode, Double click = Emergency alert
 * - Mode Button: Single click = Cycle modes, Long press = System info, Double click = Calibrate
 * 
 * LED Indicators:
 * - Status LED: System status (on = ready, blinking = processing)
 * - Hazard LED: Hazard detection alert
 * - Processing LED: AI processing in progress
 * 
 * Operating Modes:
 * 1. Hazard Detection - Identifies dangers and safety hazards
 * 2. Visual Caption - Describes what the camera sees
 * 3. Sign Detection - Recognizes and reads signs
 * 4. OCR - Optical Character Recognition for text
 * 5. Auto Mode - Runs all features automatically
 */

#include <Arduino.h>
#include "intel_glasses.h"

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  Serial.println("========================================");
  Serial.println("       INTEL AI GLASSES SYSTEM         ");
  Serial.println("========================================");
  Serial.println("Initializing smart glasses...");
  Serial.println();
  
  // Initialize the Intel Glasses system
  if (!glasses.initialize()) {
    Serial.println("FATAL ERROR: Failed to initialize glasses system");
    Serial.println("Please check hardware connections and restart");
    while(1) {
      delay(1000);
      Serial.print(".");
    }
  }
  
  Serial.println("========================================");
  Serial.println("         SYSTEM READY FOR USE          ");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Usage Instructions:");
  Serial.println("• Voice Commands - 'Hazard mode', 'Caption mode', 'Sign mode', 'Text mode', 'Auto mode'");
  Serial.println("• Voice Actions - 'Capture', 'Emergency', 'Status', 'Sleep', 'Wake up'");
  Serial.println("• Capture Button - Manual scan/Auto toggle/Emergency (backup control)");
  Serial.println("• Mode Button - Change modes/Info/Calibrate (backup control)");
  Serial.println("• Auto-capture mode scans environment every 5 seconds");
  Serial.println();
}
void loop() {
  // Run the main Intel Glasses system loop
  glasses.run();
  
  // Small delay to prevent watchdog timeout
  delay(10);
}