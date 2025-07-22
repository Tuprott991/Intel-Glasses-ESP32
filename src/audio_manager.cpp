#include "audio_manager.h"
#include "gsm_module.h"
#include <SPIFFS.h>

AudioManager audioManager;

AudioManager::AudioManager() {
    audio = nullptr;
    isInitialized = false;
    isPlaying = false;
    isMuted = false;
    globalVolume = 70; // Default volume 70%
    
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    
    // Default I2S pins for audio output (MAX98357A or similar I2S amplifier)
    i2s_bclk_pin = 26;  // Bit clock
    i2s_lrc_pin = 25;   // Left/Right clock (Word Select)
    i2s_dout_pin = 22;  // Data out
    
    // Initialize current playback
    currentPlayback.isPlaying = false;
    currentPlayback.priority = false;
    currentPlayback.volume = globalVolume;
}

AudioManager::~AudioManager() {
    deinitialize();
}

bool AudioManager::initialize() {
    Serial.println("Initializing audio manager...");
    
    // Initialize SPIFFS for local MP3 storage
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to initialize SPIFFS");
        return false;
    }
    
    // Create Audio object
    audio = new Audio();
    if (!audio) {
        Serial.println("Failed to create Audio object");
        return false;
    }
    
    // Setup I2S audio output
    if (!setupI2SAudio()) {
        Serial.println("Failed to setup I2S audio");
        return false;
    }
    
    // Load and verify local audio files
    loadLocalAudioFiles();
    
    // Set initial volume
    audio->setVolume(globalVolume);
    
    isInitialized = true;
    Serial.println("Audio manager initialized successfully");
    
    // Play startup sound
    playSuccessSound();
    
    return true;
}

void AudioManager::deinitialize() {
    stopAllAudio();
    
    if (audio) {
        delete audio;
        audio = nullptr;
    }
    
    SPIFFS.end();
    isInitialized = false;
    Serial.println("Audio manager deinitialized");
}

bool AudioManager::setupI2SAudio() {
    // Setup I2S pins for audio output
    audio->setPinout(i2s_bclk_pin, i2s_lrc_pin, i2s_dout_pin);
    
    Serial.printf("I2S Audio setup - BCLK: %d, LRC: %d, DOUT: %d\n", 
                  i2s_bclk_pin, i2s_lrc_pin, i2s_dout_pin);
    
    return true;
}

bool AudioManager::playLocalMP3(const String& filename, AudioCategory category, bool priority) {
    if (!isInitialized) {
        Serial.println("Audio manager not initialized");
        return false;
    }
    
    String fullPath = getAudioFilePath(filename);
    
    // Check if file exists
    if (!SPIFFS.exists(fullPath)) {
        Serial.println("Audio file not found: " + fullPath);
        return false;
    }
    
    // Stop current audio if this is high priority
    if (priority && isPlaying) {
        stopCurrentAudio();
    }
    
    // If already playing and not priority, queue it
    if (isPlaying && !priority) {
        AudioPlayback queuedAudio;
        queuedAudio.type = AUDIO_LOCAL_MP3;
        queuedAudio.category = category;
        queuedAudio.filename = filename;
        queuedAudio.priority = priority;
        queuedAudio.volume = globalVolume;
        return queueAudio(queuedAudio);
    }
    
    // Play the audio file
    Serial.println("Playing audio: " + fullPath);
    
    if (audio->connecttoFS(SPIFFS, fullPath.c_str())) {
        currentPlayback.type = AUDIO_LOCAL_MP3;
        currentPlayback.category = category;
        currentPlayback.filename = filename;
        currentPlayback.isPlaying = true;
        currentPlayback.priority = priority;
        currentPlayback.startTime = millis();
        currentPlayback.volume = globalVolume;
        
        isPlaying = true;
        return true;
    } else {
        Serial.println("Failed to play audio file: " + fullPath);
        return false;
    }
}

bool AudioManager::playCloudAudio(const String& audioUrl, AudioCategory category, bool priority) {
    if (!isInitialized) {
        Serial.println("Audio manager not initialized");
        return false;
    }
    
    Serial.println("Playing cloud audio: " + audioUrl);
    
    // Stop current audio if this is high priority
    if (priority && isPlaying) {
        stopCurrentAudio();
    }
    
    // If already playing and not priority, queue it
    if (isPlaying && !priority) {
        AudioPlayback queuedAudio;
        queuedAudio.type = AUDIO_CLOUD_STREAM;
        queuedAudio.category = category;
        queuedAudio.url = audioUrl;
        queuedAudio.priority = priority;
        queuedAudio.volume = globalVolume;
        return queueAudio(queuedAudio);
    }
    
    // Connect to cloud audio stream
    if (audio->connecttohost(audioUrl.c_str())) {
        currentPlayback.type = AUDIO_CLOUD_STREAM;
        currentPlayback.category = category;
        currentPlayback.url = audioUrl;
        currentPlayback.isPlaying = true;
        currentPlayback.priority = priority;
        currentPlayback.startTime = millis();
        currentPlayback.volume = globalVolume;
        
        isPlaying = true;
        return true;
    } else {
        Serial.println("Failed to connect to cloud audio: " + audioUrl);
        return false;
    }
}

void AudioManager::playHazardAlert(const String& hazardType, const String& direction) {
    String hazardFile = getHazardAudioFile(hazardType);
    
    if (direction.length() > 0) {
        // If we have direction info, try to find specific directional audio
        String directionalFile = hazardType + "_" + direction + ".mp3";
        if (checkLocalAudioFile(directionalFile)) {
            hazardFile = directionalFile;
        }
    }
    
    Serial.printf("Playing hazard alert: %s (direction: %s)\n", hazardType.c_str(), direction.c_str());
    
    // Hazard alerts are always high priority
    playLocalMP3(hazardFile, AUDIO_HAZARD, true);
}

void AudioManager::playModeChangeConfirmation(const String& modeName) {
    String modeFile = getModeAudioFile(modeName);
    playLocalMP3(modeFile, AUDIO_SYSTEM, false);
}

void AudioManager::playSystemStatus(const String& statusMessage) {
    // For system status, we might use TTS or pre-recorded messages
    // For now, play a generic status sound
    playLocalMP3("status.mp3", AUDIO_STATUS, false);
}

void AudioManager::playErrorSound(const String& errorType) {
    playLocalMP3("error.mp3", AUDIO_ERROR, true);
}

void AudioManager::playSuccessSound() {
    playLocalMP3("success.mp3", AUDIO_SYSTEM, false);
}

void AudioManager::playProcessingSound() {
    playLocalMP3("processing.mp3", AUDIO_SYSTEM, false);
}

void AudioManager::update() {
    if (!isInitialized || !audio) {
        return;
    }
    
    // Update audio processing
    audio->loop();
    
    // Check if current audio finished
    if (isPlaying && !audio->isRunning()) {
        handleAudioFinished();
    }
    
    // Play next audio from queue if available
    if (!isPlaying && hasQueuedAudio()) {
        playAudioFromQueue();
    }
}

bool AudioManager::loadLocalAudioFiles() {
    Serial.println("Loading local audio files...");
    
    // List of required audio files
    const char* requiredFiles[] = {
        "startup.mp3",
        "success.mp3", 
        "error.mp3",
        "processing.mp3",
        "status.mp3",
        "hazard_general.mp3",
        "hazard_obstacle.mp3",
        "hazard_fire.mp3",
        "hazard_warning.mp3",
        "mode_hazard.mp3",
        "mode_caption.mp3",
        "mode_sign.mp3",
        "mode_ocr.mp3",
        "mode_auto.mp3"
    };
    
    int foundFiles = 0;
    int totalFiles = sizeof(requiredFiles) / sizeof(requiredFiles[0]);
    
    for (int i = 0; i < totalFiles; i++) {
        String filepath = getAudioFilePath(requiredFiles[i]);
        if (SPIFFS.exists(filepath)) {
            foundFiles++;
            Serial.println("Found: " + String(requiredFiles[i]));
        } else {
            Serial.println("Missing: " + String(requiredFiles[i]));
        }
    }
    
    Serial.printf("Audio files: %d/%d found\n", foundFiles, totalFiles);
    return foundFiles > 0;
}

String AudioManager::getAudioFilePath(const String& filename) {
    return "/audio/" + filename;
}

String AudioManager::getHazardAudioFile(const String& hazardType) {
    // Map hazard types to audio files
    if (hazardType.indexOf("obstacle") >= 0) return "hazard_obstacle.mp3";
    if (hazardType.indexOf("fire") >= 0) return "hazard_fire.mp3";
    if (hazardType.indexOf("warning") >= 0) return "hazard_warning.mp3";
    
    // Default hazard sound
    return "hazard_general.mp3";
}

String AudioManager::getModeAudioFile(const String& modeName) {
    // Map mode names to audio files
    if (modeName.indexOf("Hazard") >= 0) return "mode_hazard.mp3";
    if (modeName.indexOf("Caption") >= 0) return "mode_caption.mp3";
    if (modeName.indexOf("Sign") >= 0) return "mode_sign.mp3";
    if (modeName.indexOf("Text") >= 0 || modeName.indexOf("OCR") >= 0) return "mode_ocr.mp3";
    if (modeName.indexOf("Auto") >= 0) return "mode_auto.mp3";
    
    return "mode_general.mp3";
}

void AudioManager::stopCurrentAudio() {
    if (isPlaying && audio) {
        audio->stopSong();
        currentPlayback.isPlaying = false;
        isPlaying = false;
        Serial.println("Stopped current audio");
    }
}

void AudioManager::stopAllAudio() {
    stopCurrentAudio();
    clearQueue();
}

void AudioManager::setGlobalVolume(int volume) {
    globalVolume = constrain(volume, 0, 100);
    if (audio) {
        audio->setVolume(globalVolume);
    }
    Serial.printf("Audio volume set to: %d%%\n", globalVolume);
}

void AudioManager::setMute(bool mute) {
    isMuted = mute;
    if (audio) {
        if (mute) {
            audio->setVolume(0);
        } else {
            audio->setVolume(globalVolume);
        }
    }
    Serial.println("Audio " + String(mute ? "muted" : "unmuted"));
}

bool AudioManager::queueAudio(const AudioPlayback& playback) {
    if (queueCount >= AUDIO_QUEUE_SIZE) {
        Serial.println("Audio queue full");
        return false;
    }
    
    audioQueue[queueTail] = playback;
    queueTail = (queueTail + 1) % AUDIO_QUEUE_SIZE;
    queueCount++;
    
    Serial.println("Audio queued: " + playback.filename + playback.url);
    return true;
}

bool AudioManager::playAudioFromQueue() {
    if (queueCount == 0) {
        return false;
    }
    
    AudioPlayback nextAudio = audioQueue[queueHead];
    queueHead = (queueHead + 1) % AUDIO_QUEUE_SIZE;
    queueCount--;
    
    // Play the queued audio
    if (nextAudio.type == AUDIO_LOCAL_MP3) {
        return playLocalMP3(nextAudio.filename, nextAudio.category, nextAudio.priority);
    } else if (nextAudio.type == AUDIO_CLOUD_STREAM) {
        return playCloudAudio(nextAudio.url, nextAudio.category, nextAudio.priority);
    }
    
    return false;
}

void AudioManager::clearQueue() {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    Serial.println("Audio queue cleared");
}

bool AudioManager::hasQueuedAudio() {
    return queueCount > 0;
}

bool AudioManager::isCurrentlyPlaying() {
    return isPlaying && audio && audio->isRunning();
}

void AudioManager::handleAudioFinished() {
    Serial.println("Audio playback finished");
    currentPlayback.isPlaying = false;
    isPlaying = false;
}

bool AudioManager::checkLocalAudioFile(const String& filename) {
    String fullPath = getAudioFilePath(filename);
    return SPIFFS.exists(fullPath);
}

void AudioManager::listAvailableAudioFiles() {
    Serial.println("=== Available Audio Files ===");
    
    File root = SPIFFS.open("/audio");
    if (!root || !root.isDirectory()) {
        Serial.println("No audio directory found");
        return;
    }
    
    File file = root.openNextFile();
    int fileCount = 0;
    
    while (file) {
        if (!file.isDirectory()) {
            Serial.printf("%d. %s (%d bytes)\n", ++fileCount, file.name(), file.size());
        }
        file = root.openNextFile();
    }
    
    if (fileCount == 0) {
        Serial.println("No audio files found");
    } else {
        Serial.printf("Total: %d audio files\n", fileCount);
    }
    
    Serial.println("==============================");
}
