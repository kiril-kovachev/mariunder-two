#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include "FaceEmotions.hpp"

// Manages MP3-TF-16P audio playback
class AudioManager {
public:
    AudioManager() :
        _serial(nullptr),
        _isInitialized(false),
        _isPlaying(false),
        _playbackStartTime(0),
        _currentVolume(25)  // Default volume
    {}

    bool begin(HardwareSerial* serial) {
        _serial = serial;

        // Initialize serial for DFPlayer
        _serial->begin(9600, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN);
        
        delay(500);       // MP3-TF16F boot time

        if (!_dfPlayer.begin(*_serial)) {
            Serial.println("Failed to initialize DFPlayer!");
            Serial.println("Check connections:");
            Serial.println("- RX (module) -> GPIO7 (ESP32)");
            Serial.println("- TX (module) -> GPIO6 (ESP32)");
            return false;
        }

        Serial.println("DFPlayer initialized");

        // Configure DFPlayer
        _dfPlayer.setTimeOut(500);        // Timeout 500ms
        _dfPlayer.volume(25);             // Volume 0-30 (start at moderate level)
        _dfPlayer.EQ(DFPLAYER_EQ_NORMAL); // Normal EQ

        delay(200);  // Allow settings to take effect

        _isInitialized = true;
        return true;
    }

    void update() {
        if (!_isInitialized) return;

        if (_isPlaying) {
            // Primary: listen for the DFPlayer "play finished" notification
            if (_dfPlayer.available()) {
                uint8_t type = _dfPlayer.readType();
                if (type == DFPlayerPlayFinished) {
                    _isPlaying = false;
                    return;
                }
            }
            // Fallback: hard timeout (10s) in case the notification is missed
            if (millis() - _playbackStartTime > 10000) {
                _isPlaying = false;
            }
        }
    }

    void playEmotion(FaceEmotions emotion) {
        if (!_isInitialized) return;

        // Map emotion to file number in folder 01
        // Files are 001.mp3 to 018.mp3 in folder 01
        int fileNumber = (int)emotion + 1;  // +1 because files start at 001

        Serial.print("Playing emotion: ");
        Serial.print(getEmotionName(emotion));
        Serial.print(" (Folder 01, File ");
        Serial.print(fileNumber);
        Serial.println(")");

        _dfPlayer.playFolder(1, fileNumber);
        _isPlaying = true;
        _playbackStartTime = millis();

        delay(50);  // Small delay to allow command processing
    }

    // Start playing from folder 02 (plays 001.mp3)
    void playTapFolderStart() {
        if (!_isInitialized) return;

        Serial.println("Playing tap folder: 02/001");
        _dfPlayer.playFolder(2, 1);
        _isPlaying = true;
        _playbackStartTime = millis();
        delay(50);
    }

    // Play next file in current folder (DFPlayer handles cycling automatically)
    void playTapNext() {
        if (!_isInitialized) return;

        Serial.println("Playing next (tap folder)");
        _dfPlayer.next();
        _isPlaying = true;
        _playbackStartTime = millis();
        delay(50);
    }

    // Play specific file from folder
    void playFolderFile(uint8_t folder, uint8_t fileNumber) {
        if (!_isInitialized) return;

        Serial.print("Playing folder ");
        Serial.print(folder);
        Serial.print(", file ");
        Serial.println(fileNumber);

        _dfPlayer.playFolder(folder, fileNumber);
        _isPlaying = true;
        _playbackStartTime = millis();
        delay(50);
    }

    void playSleepSound() {
        if (!_isInitialized) return;

        Serial.println("Playing sleep transition sound");

        // Sleep sound is in folder 03, file 001
        _dfPlayer.playFolder(3, 1);
        _isPlaying = true;
        _playbackStartTime = millis();

        delay(50);
    }

    void stopPlayback() {
        if (!_isInitialized) return;

        Serial.println("Stopping audio playback");
        _dfPlayer.stop();
        _isPlaying = false;

        delay(50);
    }

    bool isPlaying() {
        return _isPlaying;
    }

    void setVolume(uint8_t volume) {
        if (!_isInitialized) return;

        // Volume range: 0-30
        volume = constrain(volume, 0, 30);
        _dfPlayer.volume(volume);
        _currentVolume = volume;

        Serial.print("Volume set to: ");
        Serial.println(volume);

        delay(50);
    }

    void increaseVolume() {
        if (!_isInitialized) return;

        uint8_t newVolume = _currentVolume + 2;  // Increment by 2
        if (newVolume > 30) newVolume = 30;
        setVolume(newVolume);
    }

    void decreaseVolume() {
        if (!_isInitialized) return;

        int newVolume = _currentVolume - 2;  // Decrement by 2
        if (newVolume < 0) newVolume = 0;
        setVolume((uint8_t)newVolume);
    }

    uint8_t getCurrentVolume() {
        return _currentVolume;
    }

private:
    HardwareSerial* _serial;
    DFRobotDFPlayerMini _dfPlayer;
    bool _isInitialized;
    bool _isPlaying;
    uint32_t _playbackStartTime;
    uint8_t _currentVolume;
};

#endif // AUDIO_MANAGER_H
