#include "speech_recognition.h"

SpeechRecognition speechRecognizer;

SpeechRecognition::SpeechRecognition() {
    isInitialized = false;
    isListening = false;
    isProcessing = false;
    audioBuffer = nullptr;
    inferenceBuffer = nullptr;
    audioBufferIndex = 0;
    inferenceBufferIndex = 0;
    lastCommandTime = 0;
    lastCommand = CMD_NONE;
    lastConfidence = 0.0;
}

SpeechRecognition::~SpeechRecognition() {
    deinitialize();
}

bool SpeechRecognition::initialize() {
    Serial.println("Initializing speech recognition...");
    
    // Allocate audio buffers
    audioBuffer = (int16_t*)malloc(SAMPLE_BUFFER_SIZE * sizeof(int16_t));
    inferenceBuffer = (int16_t*)malloc(INFERENCE_BUFFER_SIZE * sizeof(int16_t));
    
    if (!audioBuffer || !inferenceBuffer) {
        Serial.println("Failed to allocate audio buffers");
        return false;
    }
    
    clearBuffers();
    
    // Initialize I2S for microphone input
    if (!initializeI2S()) {
        Serial.println("Failed to initialize I2S");
        return false;
    }
    
    // Setup Edge Impulse signal structure
    signal.total_length = INFERENCE_BUFFER_SIZE;
    signal.get_data = &audioSignalGetDataWrapper;
    signal.ctx = (void*)this;
    
    isInitialized = true;
    Serial.println("Speech recognition initialized successfully");
    
    // Test microphone
    if (!testMicrophone()) {
        Serial.println("Warning: Microphone test failed");
    }
    
    return true;
}

void SpeechRecognition::deinitialize() {
    stopListening();
    
    if (audioBuffer) {
        free(audioBuffer);
        audioBuffer = nullptr;
    }
    
    if (inferenceBuffer) {
        free(inferenceBuffer);
        inferenceBuffer = nullptr;
    }
    
    i2s_driver_uninstall(I2S_PORT);
    isInitialized = false;
    
    Serial.println("Speech recognition deinitialized");
}

bool SpeechRecognition::initializeI2S() {
    // Configure I2S for microphone input
    i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = SAMPLE_BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    // Pin configuration
    pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    // Install and start I2S driver
    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %d\n", err);
        return false;
    }
    
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %d\n", err);
        return false;
    }
    
    Serial.println("I2S microphone initialized");
    return true;
}

bool SpeechRecognition::startListening() {
    if (!isInitialized) {
        Serial.println("Speech recognition not initialized");
        return false;
    }
    
    isListening = true;
    clearBuffers();
    
    Serial.println("Started listening for speech commands");
    Serial.println("Supported commands:");
    Serial.println("- 'Hazard mode' - Switch to hazard detection");
    Serial.println("- 'Caption mode' - Switch to visual caption");
    Serial.println("- 'Sign mode' - Switch to sign detection");
    Serial.println("- 'Text mode' - Switch to OCR mode");
    Serial.println("- 'Auto mode' - Switch to automatic mode");
    Serial.println("- 'Capture' - Take a picture");
    Serial.println("- 'Emergency' - Emergency alert");
    Serial.println("- 'Status' - System status");
    
    return true;
}

void SpeechRecognition::stopListening() {
    isListening = false;
    Serial.println("Stopped listening for speech commands");
}

void SpeechRecognition::update() {
    if (!isListening || isProcessing) {
        return;
    }
    
    // Capture audio continuously
    if (captureAudio()) {
        // Check if we have enough audio for inference
        if (inferenceBufferIndex >= INFERENCE_BUFFER_SIZE) {
            // Process the audio buffer
            SpeechResult result = processAudio();
            
            if (result.isValid && result.confidence >= CONFIDENCE_THRESHOLD) {
                Serial.printf("Speech command detected: %s (confidence: %.2f)\n", 
                             result.commandText.c_str(), result.confidence);
                
                lastCommand = result.command;
                lastConfidence = result.confidence;
                lastCommandTime = millis();
                
                // Handle the command (this will be called by the main system)
                handleSpeechCommand(result);
            }
            
            // Reset buffer for next inference
            inferenceBufferIndex = 0;
        }
    }
}

bool SpeechRecognition::captureAudio() {
    size_t bytesRead = 0;
    
    // Read audio samples from I2S
    esp_err_t result = i2s_read(I2S_PORT, audioBuffer, 
                               SAMPLE_BUFFER_SIZE * sizeof(int16_t), 
                               &bytesRead, portMAX_DELAY);
    
    if (result != ESP_OK) {
        Serial.printf("I2S read error: %d\n", result);
        return false;
    }
    
    size_t samplesRead = bytesRead / sizeof(int16_t);
    
    // Copy samples to inference buffer
    for (size_t i = 0; i < samplesRead && inferenceBufferIndex < INFERENCE_BUFFER_SIZE; i++) {
        inferenceBuffer[inferenceBufferIndex++] = audioBuffer[i];
    }
    
    return samplesRead > 0;
}

SpeechResult SpeechRecognition::processAudio() {
    SpeechResult result;
    result.command = CMD_NONE;
    result.confidence = 0.0;
    result.commandText = "";
    result.timestamp = millis();
    result.isValid = false;
    
    isProcessing = true;
    
    // Preprocess audio (normalize, filter, etc.)
    preprocessAudio();
    
    // Check for voice activity
    if (!detectVoiceActivity()) {
        isProcessing = false;
        return result;
    }
    
    // Run Edge Impulse inference
    if (runInference()) {
        result.command = classifyResult();
        result.confidence = lastConfidence;
        result.commandText = getCommandText(result.command);
        result.isValid = (result.command != CMD_NONE);
    }
    
    isProcessing = false;
    return result;
}

bool SpeechRecognition::runInference() {
    // Run the Edge Impulse classifier
    EI_IMPULSE_ERROR res = run_classifier(&signal, &this->result, false);
    
    if (res != EI_IMPULSE_OK) {
        Serial.printf("Edge Impulse inference failed: %d\n", res);
        return false;
    }
    
    return true;
}

SpeechCommand SpeechRecognition::classifyResult() {
    if (this->result.classification_count == 0) {
        return CMD_NONE;
    }
    
    // Find the classification with highest confidence
    float maxConfidence = 0.0;
    int maxIndex = -1;
    
    for (size_t i = 0; i < this->result.classification_count; i++) {
        if (this->result.classification[i].value > maxConfidence) {
            maxConfidence = this->result.classification[i].value;
            maxIndex = i;
        }
    }
    
    if (maxIndex == -1 || maxConfidence < CONFIDENCE_THRESHOLD) {
        return CMD_NONE;
    }
    
    lastConfidence = maxConfidence;
    
    // Map classification label to command
    String label = String(this->result.classification[maxIndex].label);
    return getCommandFromText(label);
}

String SpeechRecognition::getCommandText(SpeechCommand cmd) {
    switch (cmd) {
        case CMD_HAZARD_MODE: return "Hazard mode";
        case CMD_CAPTION_MODE: return "Caption mode";
        case CMD_SIGN_MODE: return "Sign mode";
        case CMD_OCR_MODE: return "Text mode";
        case CMD_AUTO_MODE: return "Auto mode";
        case CMD_CAPTURE: return "Capture";
        case CMD_EMERGENCY: return "Emergency";
        case CMD_STATUS: return "Status";
        case CMD_SLEEP: return "Sleep";
        case CMD_WAKE_UP: return "Wake up";
        default: return "Unknown";
    }
}

SpeechCommand SpeechRecognition::getCommandFromText(const String& text) {
    String lowerText = text;
    lowerText.toLowerCase();
    
    if (lowerText.indexOf("hazard") >= 0) return CMD_HAZARD_MODE;
    if (lowerText.indexOf("caption") >= 0 || lowerText.indexOf("describe") >= 0) return CMD_CAPTION_MODE;
    if (lowerText.indexOf("sign") >= 0) return CMD_SIGN_MODE;
    if (lowerText.indexOf("text") >= 0 || lowerText.indexOf("ocr") >= 0 || lowerText.indexOf("read") >= 0) return CMD_OCR_MODE;
    if (lowerText.indexOf("auto") >= 0) return CMD_AUTO_MODE;
    if (lowerText.indexOf("capture") >= 0 || lowerText.indexOf("photo") >= 0 || lowerText.indexOf("picture") >= 0) return CMD_CAPTURE;
    if (lowerText.indexOf("emergency") >= 0 || lowerText.indexOf("help") >= 0) return CMD_EMERGENCY;
    if (lowerText.indexOf("status") >= 0 || lowerText.indexOf("info") >= 0) return CMD_STATUS;
    if (lowerText.indexOf("sleep") >= 0) return CMD_SLEEP;
    if (lowerText.indexOf("wake") >= 0) return CMD_WAKE_UP;
    
    return CMD_NONE;
}

bool SpeechRecognition::testMicrophone() {
    Serial.println("Testing microphone...");
    
    // Capture a small sample
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, audioBuffer, 
                               1024 * sizeof(int16_t), 
                               &bytesRead, 1000);
    
    if (result != ESP_OK || bytesRead == 0) {
        Serial.println("Microphone test failed - no audio data");
        return false;
    }
    
    // Calculate RMS to check for audio activity
    float rms = calculateRMS(audioBuffer, bytesRead / sizeof(int16_t));
    Serial.printf("Microphone test - RMS level: %.2f\n", rms);
    
    if (rms < 10.0) {
        Serial.println("Warning: Very low audio level detected");
        return false;
    }
    
    Serial.println("Microphone test passed");
    return true;
}

void SpeechRecognition::preprocessAudio() {
    // Apply basic preprocessing to inference buffer
    // Normalize audio levels
    int16_t maxVal = 0;
    for (size_t i = 0; i < inferenceBufferIndex; i++) {
        if (abs(inferenceBuffer[i]) > maxVal) {
            maxVal = abs(inferenceBuffer[i]);
        }
    }
    
    if (maxVal > 0) {
        float scale = 32767.0f / maxVal * 0.8f; // Leave some headroom
        for (size_t i = 0; i < inferenceBufferIndex; i++) {
            inferenceBuffer[i] = (int16_t)(inferenceBuffer[i] * scale);
        }
    }
}

bool SpeechRecognition::detectVoiceActivity() {
    // Simple voice activity detection based on RMS energy
    float rms = calculateRMS(inferenceBuffer, inferenceBufferIndex);
    
    // Threshold for voice activity (adjust based on environment)
    const float voiceThreshold = 500.0f;
    
    return rms > voiceThreshold;
}

float SpeechRecognition::calculateRMS(int16_t* buffer, size_t length) {
    if (length == 0) return 0.0f;
    
    float sum = 0.0f;
    for (size_t i = 0; i < length; i++) {
        sum += (float)buffer[i] * buffer[i];
    }
    
    return sqrt(sum / length);
}

void SpeechRecognition::clearBuffers() {
    audioBufferIndex = 0;
    inferenceBufferIndex = 0;
    
    if (audioBuffer) {
        memset(audioBuffer, 0, SAMPLE_BUFFER_SIZE * sizeof(int16_t));
    }
    
    if (inferenceBuffer) {
        memset(inferenceBuffer, 0, INFERENCE_BUFFER_SIZE * sizeof(int16_t));
    }
}

bool SpeechRecognition::isReady() {
    return isInitialized;
}

bool SpeechRecognition::isActivelyListening() {
    return isListening && !isProcessing;
}

void SpeechRecognition::adjustSensitivity(float threshold) {
    // This would be implemented to adjust the confidence threshold
    Serial.printf("Adjusting speech sensitivity to: %.2f\n", threshold);
}

void SpeechRecognition::printAudioStats() {
    if (inferenceBufferIndex > 0) {
        float rms = calculateRMS(inferenceBuffer, inferenceBufferIndex);
        Serial.printf("Audio Stats - RMS: %.2f, Buffer: %d/%d\n", 
                     rms, inferenceBufferIndex, INFERENCE_BUFFER_SIZE);
    }
}

// Edge Impulse callback function
int SpeechRecognition::audioSignalGetData(size_t offset, size_t length, float* out_ptr) {
    for (size_t i = 0; i < length; i++) {
        if (offset + i < inferenceBufferIndex) {
            out_ptr[i] = (float)inferenceBuffer[offset + i] / 32768.0f;
        } else {
            out_ptr[i] = 0.0f;
        }
    }
    return 0;
}

int SpeechRecognition::audioSignalGetDataWrapper(size_t offset, size_t length, float* out_ptr, void* ctx) {
    SpeechRecognition* sr = (SpeechRecognition*)ctx;
    return sr->audioSignalGetData(offset, length, out_ptr);
}

// Global function to handle speech commands (called from main system)
void handleSpeechCommand(const SpeechResult& result) {
    // This function will be implemented in the main system
    // It will interface with the Intel Glasses system to execute commands
    Serial.printf("Executing speech command: %s\n", result.commandText.c_str());
}
