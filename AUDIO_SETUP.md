# Audio System Setup

The Intel Glasses system uses both local MP3 files and cloud-delivered audio for comprehensive user feedback.

## Required MP3 Files

Upload the following MP3 files to SPIFFS in the `/audio/` directory:

### System Audio Files
- `system_ready.mp3` - "Intel Glasses ready"
- `mode_change.mp3` - "Mode changed"
- `voice_command.mp3` - "Voice command recognized"
- `capture_complete.mp3` - "Photo captured"
- `processing.mp3` - "Processing image..."
- `error.mp3` - "System error"

### Mode Audio Files
- `hazard_mode.mp3` - "Hazard detection mode"
- `caption_mode.mp3` - "Visual caption mode"
- `sign_mode.mp3` - "Sign detection mode"
- `ocr_mode.mp3` - "OCR mode"

### Hazard Alert Audio Files
- `hazard_general.mp3` - "Hazard detected"
- `hazard_left.mp3` - "Obstacle on the left"
- `hazard_right.mp3` - "Obstacle on the right"
- `hazard_front.mp3` - "Obstacle ahead"
- `hazard_stairs.mp3` - "Stairs detected"
- `hazard_hole.mp3` - "Hole or gap detected"

## Cloud Audio Integration

For visual captioning and OCR, your cloud API should return JSON responses with audio URLs:

```json
{
  "success": true,
  "result": "A person walking down a street",
  "confidence": 0.95,
  "processing_time": 1.2,
  "hasAudio": true,
  "audioUrl": "https://your-api.com/audio/response_12345.mp3",
  "audioFormat": "mp3",
  "audioDuration": 3.5
}
```

## Audio File Requirements

- **Format**: MP3 (recommended) or WAV
- **Sample Rate**: 16kHz or 22kHz (for ESP32 compatibility)
- **Bitrate**: 64-128 kbps (balance quality vs. storage)
- **Duration**: Keep system sounds under 3 seconds for responsive UX
- **Volume**: Normalize all files to consistent levels

## SPIFFS Upload

Use PlatformIO's SPIFFS upload feature:

1. Create `data/audio/` directory in your project
2. Place all MP3 files in this directory
3. Run: `pio run --target uploadfs`

## Voice Quality Tips

- Use clear, concise language
- Consider text-to-speech services for consistency
- Test audio levels with actual ESP32 hardware
- Ensure accessibility-friendly voice patterns
- Consider multiple languages if needed

## Testing

The system will fall back to tone patterns if MP3 files are missing, but audio feedback greatly improves the user experience for visually impaired users.
