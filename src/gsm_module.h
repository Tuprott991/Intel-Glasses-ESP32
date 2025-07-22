#ifndef GSM_MODULE_H
#define GSM_MODULE_H

#include <TinyGsmClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "intel_glasses_config.h"

// SIM card APN credentials (configure for your carrier)
const char* apn = "internet";      // Your APN
const char* gprsUser = "";         // GPRS User (leave empty if not required)
const char* gprsPass = "";         // GPRS Password (leave empty if not required)

class GSMModule {
private:
    TinyGsm* modem;
    TinyGsmClient* client;
    HTTPClient* http;
    HardwareSerial* gsmSerial;
    bool isConnected;
    
public:
    GSMModule();
    ~GSMModule();
    
    bool initialize();
    bool connectToNetwork();
    bool isNetworkConnected();
    void disconnect();
    
    // Image upload and API call methods
    APIResponse sendImageForAnalysis(uint8_t* imageData, size_t imageSize, const String& endpoint, OperationMode mode);
    APIResponse callHazardDetection(uint8_t* imageData, size_t imageSize);
    APIResponse callVisualCaption(uint8_t* imageData, size_t imageSize);
    APIResponse callSignDetection(uint8_t* imageData, size_t imageSize);
    APIResponse callOCR(uint8_t* imageData, size_t imageSize);
    
    // Utility methods
    String getSignalQuality();
    String getNetworkInfo();
    void powerOn();
    void powerOff();
    void reset();
    
private:
    String encodeImageToBase64(uint8_t* imageData, size_t imageSize);
    APIResponse parseAPIResponse(String jsonResponse);
    bool waitForResponse(int timeout = 30000);
};

// Global GSM module instance
extern GSMModule gsmModule;

#endif // GSM_MODULE_H
