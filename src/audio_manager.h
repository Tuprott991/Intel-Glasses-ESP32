#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "intel_glasses_config.h"

// ESP32 native audio support using I2S
// Will use ESP32's built-in I2S capabilities for audio output
#include "driver/i2s.h"
#include "esp_system.h"

// Audio file types
enum AudioType {
    AUDIO_LOCAL_MP3,      // Local MP3 files stored in SPIFFS/SD
    AUDIO_CLOUD_STREAM,   // Audio stream from cloud API
    AUDIO_SIMPLE_TONE     // Simple buzzer tones
};

// Audio feedback categories
enum AudioCategory {
    AUDIO_SYSTEM,         // System sounds (beeps, confirmations)
    AUDIO_HAZARD,         // Hazard detection audio
    AUDIO_CAPTION,        // Visual captioning audio from cloud
    AUDIO_OCR,            // OCR text audio from cloud
    AUDIO_SIGN,           // Sign detection audio
    AUDIO_STATUS,         // System status audio
    AUDIO_ERROR           // Error notifications
};

// Audio playback structure
struct AudioPlayback {
    AudioType type;
    AudioCategory category;
    String filename;      // For local MP3 files
    String url;          // For cloud audio streams
    uint8_t* audioData;  // For in-memory audio data
    size_t dataSize;     // Size of audio data
    bool isPlaying;
    bool priority;       // High priority audio interrupts lower priority
    unsigned long startTime;
    int volume;          // 0-100
};

class AudioManager {
private:
    // Simplified implementation - will be replaced with full audio support later
    bool isInitialized;
    bool isPlaying;
    bool muteState;
    int globalVolume;
    
    // Current playback
    AudioPlayback currentPlayback;
    
    // Audio queue for managing multiple audio requests
    static const int AUDIO_QUEUE_SIZE = 5;
    AudioPlayback audioQueue[AUDIO_QUEUE_SIZE];
    int queueHead;
    int queueTail;
    int queueCount;
    
    // I2S audio output pins
    int i2s_bclk_pin;
    int i2s_lrc_pin;
    int i2s_dout_pin;
    
public:
    AudioManager();
    ~AudioManager();
    
    // Initialization and configuration
    bool initialize();
    void deinitialize();
    bool setupI2SAudio();
    
    // Local MP3 file playback
    bool playLocalMP3(const String& filename, AudioCategory category, bool priority = false);
    bool loadLocalAudioFiles();
    
    // Cloud audio stream playback
    bool playCloudAudio(const String& audioUrl, AudioCategory category, bool priority = false);
    bool playAudioData(uint8_t* audioData, size_t dataSize, AudioCategory category, bool priority = false);
    
    // System audio feedback
    void playSystemAudio(const String& audioName);  // Generic system audio method
    void playHazardAlert(const String& hazardType, const String& direction = "");
    void playModeChangeConfirmation(const String& modeName);
    void playSystemStatus(const String& statusMessage);
    void playErrorSound(const String& errorType);
    void playSuccessSound();
    void playProcessingSound();
    
    // Playback control
    void update();
    void stopCurrentAudio();
    void stopAllAudio();
    void pauseAudio();
    void resumeAudio();
    
    // Volume and settings
    void setGlobalVolume(int volume);     // 0-100
    void setMute(bool mute);
    int getGlobalVolume();
    bool isMuted();
    
    // Queue management
    bool queueAudio(const AudioPlayback& playback);
    void clearQueue();
    bool hasQueuedAudio();
    
    // Status
    bool isCurrentlyPlaying();
    AudioCategory getCurrentCategory();
    String getCurrentAudioInfo();
    
    // Audio file management
    bool checkLocalAudioFile(const String& filename);
    void listAvailableAudioFiles();
    
private:
    bool playAudioFromQueue();
    bool addToQueue(const AudioPlayback& playback);
    AudioPlayback getNextFromQueue();
    void removeFromQueue();
    
    String getAudioFilePath(const String& filename);
    String getHazardAudioFile(const String& hazardType);
    String getModeAudioFile(const String& modeName);
    
    bool downloadAndPlayCloudAudio(const String& audioUrl);
    void handleAudioFinished();
    
    // Audio processing callbacks
    static void audioEndCallback(void* userData);
};

// Global audio manager instance
extern AudioManager audioManager;

#endif // AUDIO_MANAGER_H
