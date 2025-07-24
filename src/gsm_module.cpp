#include "gsm_module.h"
#include <base64.h>
#include <StreamDebugger.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <HTTPClient.h>

// SIM card APN credentials (configure for your carrier)
const char* apn = "internet";      // Your APN
const char* gprsUser = "";         // GPRS User (leave empty if not required)
const char* gprsPass = "";         // GPRS Password (leave empty if not required)

// Create a debugging stream for GSM communication
StreamDebugger debugger(Serial2, Serial);
TinyGsm gsmModem(debugger);

GSMModule gsmModule;

GSMModule::GSMModule() {
    gsmSerial = &Serial2;
    modem = &gsmModem;
    client = nullptr;
    http = nullptr;
    isConnected = false;
}

GSMModule::~GSMModule() {
    if (client) delete client;
    if (http) delete http;
}

bool GSMModule::initialize() {
    Serial.println("Initializing GSM module...");
    
    // Initialize serial communication
    gsmSerial->begin(GSM_BAUD, SERIAL_8N1, GSM_PIN_RX, GSM_PIN_TX);
    
    // Power on the modem
    powerOn();
    delay(3000);
    
    // Initialize modem
    if (!modem->init()) {
        Serial.println("Failed to initialize modem");
        return false;
    }
    
    Serial.println("GSM module initialized successfully");
    Serial.print("Modem Name: ");
    Serial.println(modem->getModemName());
    Serial.print("Modem Info: ");
    Serial.println(modem->getModemInfo());
    
    return true;
}

bool GSMModule::connectToNetwork() {
    Serial.println("Connecting to cellular network...");
    
    // Restart modem
    if (!modem->restart()) {
        Serial.println("Failed to restart modem");
        return false;
    }
    
    // Wait for network registration
    Serial.print("Waiting for network...");
    if (!modem->waitForNetwork()) {
        Serial.println(" Failed");
        return false;
    }
    Serial.println(" Connected to network");
    
    // Connect to GPRS
    Serial.print("Connecting to ");
    Serial.print(apn);
    if (!modem->gprsConnect(apn, gprsUser, gprsPass)) {
        Serial.println(" Failed");
        return false;
    }
    Serial.println(" Connected to GPRS");
    
    // Create TinyGsmClient
    client = new TinyGsmClient(*modem);
    http = new HTTPClient();
    
    isConnected = true;
    
    Serial.print("Signal quality: ");
    Serial.println(getSignalQuality());
    Serial.print("Network info: ");
    Serial.println(getNetworkInfo());
    
    return true;
}

bool GSMModule::isNetworkConnected() {
    return isConnected && modem->isNetworkConnected() && modem->isGprsConnected();
}

void GSMModule::disconnect() {
    if (modem->isGprsConnected()) {
        modem->gprsDisconnect();
    }
    isConnected = false;
}

APIResponse GSMModule::sendImageForAnalysis(uint8_t* imageData, size_t imageSize, const String& endpoint, OperationMode mode) {
    APIResponse response;
    response.success = false;
    response.confidence = 0.0;
    response.processing_time = 0;
    
    if (!isNetworkConnected()) {
        response.error = "Network not connected";
        return response;
    }
    
    // Encode image to Base64
    String base64Image = encodeImageToBase64(imageData, imageSize);
    if (base64Image.length() == 0) {
        response.error = "Failed to encode image";
        return response;
    }
    
    // Create JSON payload
    JsonDocument doc;
    doc["image"] = base64Image;
    doc["api_key"] = CLOUD_API_KEY;
    doc["mode"] = (int)mode;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Setup HTTP request (simplified URL approach for TinyGsm compatibility)
    String url = "https://" + String(CLOUD_API_HOST) + ":" + String(CLOUD_API_PORT) + endpoint;
    http->begin(url);  // Use URL-only method for TinyGsm compatibility
    http->addHeader("Content-Type", "application/json");
    http->addHeader("Authorization", "Bearer " + String(CLOUD_API_KEY));
    http->setTimeout(CLOUD_API_TIMEOUT);
    
    // Send POST request
    Serial.println("Sending image to cloud API...");
    unsigned long startTime = millis();
    
    int httpResponseCode = http->POST(jsonString);
    
    if (httpResponseCode > 0) {
        String responsePayload = http->getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.println("Response: " + responsePayload);
        
        if (httpResponseCode == 200) {
            response = parseAPIResponse(responsePayload);
            response.processing_time = millis() - startTime;
        } else {
            response.error = "HTTP Error: " + String(httpResponseCode);
        }
    } else {
        response.error = "Connection failed: " + http->errorToString(httpResponseCode);
        Serial.println("Error: " + response.error);
    }
    
    http->end();
    return response;
}

APIResponse GSMModule::callHazardDetection(uint8_t* imageData, size_t imageSize) {
    return sendImageForAnalysis(imageData, imageSize, HAZARD_DETECTION_ENDPOINT, MODE_HAZARD_DETECTION);
}

APIResponse GSMModule::callVisualCaption(uint8_t* imageData, size_t imageSize) {
    return sendImageForAnalysis(imageData, imageSize, VISUAL_CAPTION_ENDPOINT, MODE_VISUAL_CAPTION);
}

APIResponse GSMModule::callSignDetection(uint8_t* imageData, size_t imageSize) {
    return sendImageForAnalysis(imageData, imageSize, SIGN_DETECTION_ENDPOINT, MODE_SIGN_DETECTION);
}

APIResponse GSMModule::callOCR(uint8_t* imageData, size_t imageSize) {
    return sendImageForAnalysis(imageData, imageSize, OCR_ENDPOINT, MODE_OCR);
}

String GSMModule::getSignalQuality() {
    int csq = modem->getSignalQuality();
    return String(csq) + " (RSSI: " + String(-113 + 2 * csq) + " dBm)";
}

String GSMModule::getNetworkInfo() {
    return "Operator: " + modem->getOperator() + 
           ", Network: " + (modem->isNetworkConnected() ? "Connected" : "Disconnected") +
           ", GPRS: " + (modem->isGprsConnected() ? "Connected" : "Disconnected");
}

void GSMModule::powerOn() {
    pinMode(GSM_PIN_PWR, OUTPUT);
    digitalWrite(GSM_PIN_PWR, HIGH);
    delay(1000);
    digitalWrite(GSM_PIN_PWR, LOW);
}

void GSMModule::powerOff() {
    pinMode(GSM_PIN_PWR, OUTPUT);
    digitalWrite(GSM_PIN_PWR, HIGH);
    delay(3000);
    digitalWrite(GSM_PIN_PWR, LOW);
}

void GSMModule::reset() {
    pinMode(GSM_PIN_RST, OUTPUT);
    digitalWrite(GSM_PIN_RST, LOW);
    delay(100);
    digitalWrite(GSM_PIN_RST, HIGH);
    delay(1000);
}

String GSMModule::encodeImageToBase64(uint8_t* imageData, size_t imageSize) {
    // Calculate Base64 encoded size
    size_t base64Size = ((imageSize + 2) / 3) * 4;
    
    // Check if we have enough memory
    if (base64Size > 100000) { // 100KB limit for Base64
        Serial.println("Image too large for Base64 encoding");
        return "";
    }
    
    // Encode to Base64
    String encoded = base64::encode(imageData, imageSize);
    return encoded;
}

APIResponse GSMModule::parseAPIResponse(String jsonResponse) {
    APIResponse response;
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error) {
        response.success = false;
        response.error = "Failed to parse JSON response";
        response.hasAudio = false;
        return response;
    }
    
    response.success = doc["success"] | false;
    response.result = doc["result"] | "";
    response.error = doc["error"] | "";
    response.confidence = doc["confidence"] | 0.0;
    
    // Parse audio response fields
    response.hasAudio = doc["has_audio"] | false;
    response.audioUrl = doc["audio_url"] | "";
    response.audioFormat = doc["audio_format"] | "mp3";
    response.audioSize = doc["audio_size"] | 0;
    
    return response;
}

bool GSMModule::waitForResponse(int timeout) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
        if (gsmSerial->available()) {
            return true;
        }
        delay(10);
    }
    return false;
}
