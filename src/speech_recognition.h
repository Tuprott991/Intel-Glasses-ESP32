#ifndef SPEECH_RECOGNITION_H
#define SPEECH_RECOGNITION_H

#include <Arduino.h>
#include "intel_glasses_config.h"
#include <driver/i2s.h>

// Edge Impulse includes (adjust based on your model)
// These will be generated when you export your model
extern "C" {
    #include "edge-impulse-sdk/classifier/ei_run_classifier.h"
    #include "edge-impulse-sdk/dsp/numpy.hpp"
}

// Speech recognition configuration
#define SAMPLE_RATE         16000    // Sample rate in Hz
#define SAMPLE_BITS         16       // Bits per sample
#define SAMPLE_BUFFER_SIZE  2048     // Buffer size for audio samples
#define INFERENCE_BUFFER_SIZE 16000  // Buffer for inference (1 second at 16kHz)
#define CONFIDENCE_THRESHOLD 0.8     // Minimum confidence for command recognition

// I2S microphone configuration
#define I2S_WS_PIN         15        // Word Select (LRC) pin
#define I2S_SCK_PIN        14        // Serial Clock (BCLK) pin  
#define I2S_SD_PIN         32        // Serial Data (DIN) pin
#define I2S_PORT           I2S_NUM_0 // I2S port number

// Speech commands enum
enum SpeechCommand {
    CMD_NONE,
    CMD_HAZARD_MODE,
    CMD_CAPTION_MODE, 
    CMD_SIGN_MODE,
    CMD_OCR_MODE,
    CMD_AUTO_MODE,
    CMD_CAPTURE,
    CMD_EMERGENCY,
    CMD_STATUS,
    CMD_SLEEP,
    CMD_WAKE_UP
};

// Speech recognition result structure
struct SpeechResult {
    SpeechCommand command;
    float confidence;
    String commandText;
    unsigned long timestamp;
    bool isValid;
};

class SpeechRecognition {
private:
    bool isInitialized;
    bool isListening;
    bool isProcessing;
    
    // Audio buffers
    int16_t* audioBuffer;
    int16_t* inferenceBuffer;
    size_t audioBufferIndex;
    size_t inferenceBufferIndex;
    
    // I2S configuration
    i2s_config_t i2s_config;
    i2s_pin_config_t pin_config;
    
    // Edge Impulse classifier
    signal_t signal;
    ei_impulse_result_t result;
    
    // Command recognition
    unsigned long lastCommandTime;
    SpeechCommand lastCommand;
    float lastConfidence;
    
public:
    SpeechRecognition();
    ~SpeechRecognition();
    
    // Initialization and configuration
    bool initialize();
    void deinitialize();
    bool loadModel(const uint8_t* modelData, size_t modelSize);
    
    // Audio capture and processing
    bool startListening();
    void stopListening();
    void update();
    bool captureAudio();
    
    // Speech recognition
    SpeechResult processAudio();
    bool runInference();
    SpeechCommand classifyResult();
    
    // Status and control
    bool isReady();
    bool isActivelyListening();
    void setListeningMode(bool continuous);
    void adjustSensitivity(float threshold);
    
    // Command mapping
    String getCommandText(SpeechCommand cmd);
    SpeechCommand getCommandFromText(const String& text);
    
    // Calibration and testing
    void calibrateMicrophone();
    bool testMicrophone();
    void printAudioStats();
    
private:
    bool initializeI2S();
    void configureI2S();
    bool readAudioSamples(int16_t* buffer, size_t samples);
    void preprocessAudio();
    float calculateRMS(int16_t* buffer, size_t length);
    bool detectVoiceActivity();
    void clearBuffers();
    
    // Edge Impulse integration
    int audioSignalGetData(size_t offset, size_t length, float* out_ptr);
    static int audioSignalGetDataWrapper(size_t offset, size_t length, float* out_ptr, void* ctx);
};

// Global speech recognition instance
extern SpeechRecognition speechRecognizer;

#endif // SPEECH_RECOGNITION_H
