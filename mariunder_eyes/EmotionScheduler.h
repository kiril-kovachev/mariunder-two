#ifndef EMOTION_SCHEDULER_H
#define EMOTION_SCHEDULER_H

#include <Arduino.h>
#include "Face.h"
#include "AudioManager.h"
#include "PowerManager.h"
#include "AsyncTimer.h"
#include "Animations.h"
#include "FaceEmotions.hpp"

// Manages emotion changes and audio synchronization
class EmotionScheduler {
public:
    enum RotateMode {
        ROTATE_MODE_NONE = 0,
        ROTATE_MODE_1 = 1,     // Rotate changes MP3 file
        ROTATE_MODE_2 = 2      // Rotate changes volume
    };

    EmotionScheduler() :
        _face(nullptr),
        _audioManager(nullptr),
        _powerManager(nullptr),
        _currentEmotion(NORMAL),
        _inTapMode(false),
        _rotateMode(ROTATE_MODE_NONE),
        _randomChangeInterval(15000),  // 15 seconds
        _tapModeStartTime(0),
        _lastMotionTime(0),
        _motionIdleThreshold(5000),  // 5 seconds of no motion
        _readyForMotionTrigger(false),
        _wasMoving(false),
        _currentFolderFile(1),
        _lastRotationCheck(0),
        _rotateModeStartTime(0),
        _lastRotationTime(0),
        _rotateModeTimeout(45000),   // 45 seconds timeout
        _enterRotateModeAnimation(200),  // 200ms animation
        _exitRotateModeAnimation(200),
        _pulsePhase(0.0f),
        _canSelectNextMp3(false),
        _lastShakeTime(0),
        _shakeCooldown(10000),       // 10s cooldown between shake reactions
        _isInShakenMode(false),
        _rotateMode1Playing(false),
        _wavePhase(0.0f)
    {}

    void begin(Face* face, AudioManager* audio, PowerManager* power) {
        _face = face;
        _audioManager = audio;
        _powerManager = power;

        // Set singleton instance for static callbacks
        _instance = this;

        // Set up overlay callback
        _face->OverlayCallback = nullptr;  // Will be set when entering rotate modes

        // Start random emotion timer
        _randomChangeTimer.setInterval(_randomChangeInterval);
        _randomChangeTimer.start();

        // Initialize motion tracking
        _lastMotionTime = millis();
        _readyForMotionTrigger = false;
        _wasMoving = false;

        // Initialize ADC for audio waveform display (DAC_L from DFPlayer via AC-coupling)
        // analogReadResolution and default 11dB attenuation cover the full 0-3.3V range
        analogReadResolution(12);
        pinMode(AUDIO_ADC_PIN, INPUT);
        // Warm up ADC — first reads after power-on can be inaccurate
        for (int i = 0; i < 8; i++) { analogRead(AUDIO_ADC_PIN); }
        Serial.printf("Audio ADC initialized on GPIO%d\n", AUDIO_ADC_PIN);

        Serial.println("EmotionScheduler initialized");
    }

    void update() {
        if (_face == nullptr || _powerManager == nullptr) return;

        // Check rotate mode timeout (45 seconds of no rotation)
        if (_rotateMode != ROTATE_MODE_NONE) {
            // In rotate mode 1 playing state: check if playback has ended
            if (_rotateMode == ROTATE_MODE_1 && _rotateMode1Playing) {
                if (!_audioManager->isPlaying()) {
                    // Track finished - exit back to eyes mode
                    Serial.println("Rotate mode 1 playback finished - exiting to eyes");
                    exitRotateMode();
                }
                return;  // Don't time out while playing
            }
            uint32_t now = millis();
            if ((now - _lastRotationTime) >= _rotateModeTimeout) {
                Serial.println("Rotate mode timeout (5s no rotation) - exiting");
                exitRotateMode();
            }
            return;  // Don't do other updates in rotate mode
        }

        // While in shaken mode: hold angry expression until MP3 finishes, then resume normal
        if (_isInShakenMode) {
            if (!_audioManager->isPlaying()) {
                // Audio finished - clear buzz and exit shaken mode
                _face->RightEye.Variation2.Clear();
                _face->LeftEye.Variation2.Clear();
                _isInShakenMode = false;
                Serial.println("Shaken mode ended - audio finished");
                _randomChangeTimer.restart();
            }
            return;  // Stay in angry, don't allow any other emotion changes
        }

        // Check power state - force SLEEPY emotion if in sleepy mode OR deep sleep mode
        // BUT only if audio playback has completed
        if (_powerManager->getState() == POWER_SLEEPY || _powerManager->getState() == POWER_DEEP_SLEEP) {
            if (_currentEmotion != SLEEPY) {
                // Check if audio is still playing
                if (_audioManager != nullptr && _audioManager->isPlaying()) {
                    Serial.println("PowerManager wants sleepy mode, but audio is still playing - waiting...");
                    // Don't enter sleepy mode yet, but don't do random changes either
                    return;
                }

                Serial.println("PowerManager in POWER_SLEEPY/POWER_DEEP_SLEEP state - forcing sleepy emotion");
                changeEmotionTo(SLEEPY);
                _audioManager->playSleepSound();
            }
            return;  // Don't do random changes in sleepy mode
        }

        // Don't do random changes in tap mode or rotate modes
        if (_inTapMode || _rotateMode != ROTATE_MODE_NONE) {
            return;
        }

        // Check for random emotion change
        if (_randomChangeTimer.isExpired()) {
            changeEmotionRandom();
            _randomChangeTimer.restart();
        }
    }

    void handleMotion(MotionManager* motionManager) {
        if (motionManager == nullptr) return;

        uint32_t now = millis();
        bool isCurrentlyMoving = motionManager->isMoving();

        if (isCurrentlyMoving) {
            // Motion detected - check if we're ready to trigger emotion change
            if (_readyForMotionTrigger && !_wasMoving) {
                // Motion just started after idle period - trigger emotion change
                Serial.println("Motion detected after 5s idle - changing face emotion");
                changeEmotionRandom();
                _readyForMotionTrigger = false;  // Reset - wait for next 5s idle period
            }
            // Update last motion time when motion is detected
            _lastMotionTime = now;
            _wasMoving = true;
        } else {
            // No motion - check if we've been idle for 5 seconds
            if (_wasMoving) {
                // Motion just stopped - reset idle timer
                _lastMotionTime = now;
            }
            
            uint32_t idleTime = now - _lastMotionTime;
            if (idleTime >= _motionIdleThreshold && !_readyForMotionTrigger) {
                Serial.println("5 seconds of no motion - ready to trigger emotion on next motion");
                _readyForMotionTrigger = true;
            }
            _wasMoving = false;
        }
    }

    void handleShake() {
        uint32_t now = millis();
        if (now - _lastShakeTime < _shakeCooldown) {
            return;
        }
        _lastShakeTime = now;

        Serial.println("Shake detected! Getting angry!");

        // Pick random angry emotion
        FaceEmotions shakeEmotions[] = {ANGRY, FURIOUS};
        FaceEmotions emotion = shakeEmotions[random(0, 2)];

        changeEmotionTo(emotion);
        _audioManager->playFolderFile(3, 5);  // Dedicated shake sound (03/005.mp3), distinct from folder 01 emotion sounds

        // Enter shaken mode: hold angry and apply buzzing until the MP3 finishes
        _isInShakenMode = true;

        // Apply rapid horizontal vibration via Variation2 (80ms period, ±2px)
        _face->RightEye.Variation2.Values.OffsetX = 2;
        _face->RightEye.Variation2.Animation.SetTriangle(80, 0);
        _face->RightEye.Variation2.Animation.Restart();
        _face->LeftEye.Variation2.Values.OffsetX = 2;
        _face->LeftEye.Variation2.Animation.SetTriangle(80, 0);
        _face->LeftEye.Variation2.Animation.Restart();

        // Reset random timer to prevent immediate change
        _randomChangeTimer.restart();
    }

    void handleTap(int tap) {
        // Accept tap as int for compatibility with both TouchSensorManager and MotionManager
        // 0 = NONE, 1 = SINGLE_TAP, 2 = LONG_PRESS, 3 = VERY_LONG_PRESS
        if (tap == 0) return;  // NONE

        if (tap == 3) {  // VERY_LONG_PRESS (4s) - manual deep sleep trigger
            Serial.println("Very long press (4s) - triggering sleep sequence");

            // Clear shaken mode if active
            if (_isInShakenMode) {
                _face->RightEye.Variation2.Clear();
                _face->LeftEye.Variation2.Clear();
                _isInShakenMode = false;
            }

            // Exit any rotate mode cleanly
            if (_rotateMode != ROTATE_MODE_NONE) {
                _rotateMode = ROTATE_MODE_NONE;
                _face->HideEyes = false;
                _face->OverlayCallback = nullptr;
            }

            // Stop any playing audio
            if (_audioManager->isPlaying()) {
                _audioManager->stopPlayback();
            }

            // Request deep sleep - EmotionScheduler::update() will handle
            // the sleepy eyes + sleep sound before the main loop executes sleep
            _powerManager->requestSleep();
            return;
        }

        // Trigger hit-on-head visual effect for any other tap
        _face->HitOnHead();

        if (tap == 2) {  // LONG_PRESS
            // Long press: enter rotate mode 2 (volume control) OR stop playback if not in rotate mode
            if (_rotateMode == ROTATE_MODE_NONE) {
                // Not in rotate mode - stop any playing audio first, then enter rotate mode 2
                if (_audioManager->isPlaying()) {
                    Serial.println("Long press - stopping playback");
                    _audioManager->stopPlayback();
                }
                Serial.println("Long press - entering rotate mode 2 (volume control)");
                enterRotateMode2();
            } else {
                // Already in a rotate mode - ignore long press
                Serial.println("Long press ignored - already in rotate mode");
            }
            return;
        }

        // Single tap behavior depends on current state
        if (_rotateMode == ROTATE_MODE_NONE) {
            // Not in rotate mode - stop any playing audio first, then enter rotate mode 1
            if (_audioManager->isPlaying()) {
                Serial.println("Single tap - stopping playback");
                _audioManager->stopPlayback();
            }
            Serial.println("Single tap - entering rotate mode 1 (MP3 file change)");
            enterRotateMode1();
        } else if (_rotateMode == ROTATE_MODE_1 && _rotateMode1Playing) {
            // Single tap during playback: cancel track and return to selection state
            Serial.println("Single tap - cancelling playback, returning to MP3 selection");
            _audioManager->stopPlayback();
            _rotateMode1Playing = false;
            _face->HideEyes = true;
            _face->OverlayCallback = staticDrawRotateMode1Overlay;
            _lastRotationTime = millis();
            _canSelectNextMp3 = false;
        } else {
            // In rotate mode (idle) - exit it
            Serial.println("Single tap - exiting rotate mode");
            exitRotateMode();
        }

        _randomChangeTimer.restart();
    }

    void setTapMode(bool enabled) {
        _inTapMode = enabled;
        if (!enabled) {
            Serial.println("Exiting tap mode");
            _randomChangeTimer.restart();
        }
    }

    bool inTapMode() const {
        return _inTapMode;
    }

    RotateMode getRotateMode() const {
        return _rotateMode;
    }

    // Handle rotation in rotate modes
    void handleRotation(MotionManager* motionManager) {
        if (_rotateMode == ROTATE_MODE_NONE || motionManager == nullptr) {
            return;
        }

        // Update rotation tracking
        motionManager->updateRotation();

        // Check rotation threshold (15 degrees)
        float accumulatedRotation = motionManager->getAccumulatedRotation();
        float degrees = accumulatedRotation * 180.0f / PI;

        const float ROTATION_THRESHOLD = 15.0f;

        if (_rotateMode == ROTATE_MODE_1) {
            // Block track changes while a track is playing
            if (_rotateMode1Playing) {
                // Keep resetting the timeout timer so mode doesn't expire during playback
                _lastRotationTime = millis();
                return;
            }

            // Update visual feedback - can select next MP3 when rotation is close to threshold
            _canSelectNextMp3 = abs(degrees) >= (ROTATION_THRESHOLD * 0.7f);  // Visual feedback at 70% threshold

            // Rotate mode 1: Change MP3 file on 15-degree rotation
            if (abs(degrees) >= ROTATION_THRESHOLD) {
                // Update last rotation time (for timeout tracking)
                _lastRotationTime = millis();

                if (degrees > 0) {
                    // Rotate right - next file
                    Serial.println("Rotate right - next MP3 file");
                    _currentFolderFile++;
                    if (_currentFolderFile > 10) _currentFolderFile = 1;  // Assuming 10 files in folder 02
                } else {
                    // Rotate left - previous file
                    Serial.println("Rotate left - previous MP3 file");
                    _currentFolderFile--;
                    if (_currentFolderFile < 1) _currentFolderFile = 10;
                }

                // Play the file from folder 02
                Serial.print("Playing folder 02, file ");
                Serial.println(_currentFolderFile);
                _audioManager->stopPlayback();
                delay(50);
                _audioManager->playFolderFile(2, _currentFolderFile);

                // Reset rotation accumulator and visual feedback
                motionManager->resetAccumulatedRotation();
                _canSelectNextMp3 = false;

                // Transition to playing state: show eyes with wave overlay
                _rotateMode1Playing = true;
                _wavePhase = 0.0f;
                _face->HideEyes = false;
                _face->OverlayCallback = staticDrawPlaybackOverlay;
                Serial.println("Playback started - showing eyes with wave animation");
            }
        } else if (_rotateMode == ROTATE_MODE_2) {
            // Rotate mode 2: Change volume on 15-degree rotation
            if (abs(degrees) >= ROTATION_THRESHOLD) {
                // Update last rotation time (for timeout tracking)
                _lastRotationTime = millis();

                if (degrees > 0) {
                    // Rotate right - volume up
                    Serial.println("Rotate right - volume up");
                    _audioManager->increaseVolume();
                } else {
                    // Rotate left - volume down
                    Serial.println("Rotate left - volume down");
                    _audioManager->decreaseVolume();
                }

                // Play test sound at the new volume so the user can judge the level
                _audioManager->stopPlayback();
                delay(50);
                _audioManager->playFolderFile(3, 4);

                // Reset rotation accumulator
                motionManager->resetAccumulatedRotation();
            }
        }
    }

private:
    void enterRotateMode1() {
        _rotateMode = ROTATE_MODE_1;
        _currentFolderFile = 1;
        _rotateModeStartTime = millis();
        _lastRotationTime = millis();  // Initialize rotation timer
        _pulsePhase = 0.0f;            // Reset pulse animation
        _canSelectNextMp3 = false;
        _rotateMode1Playing = false;   // Start in selection (idle) state
        _wavePhase = 0.0f;

        // Hide eyes and set overlay callback
        _face->HideEyes = true;
        _face->OverlayCallback = staticDrawRotateMode1Overlay;

        // Play animation: eyes look slightly left and right quickly
        // (Eyes won't be visible, but the Look position will be set)
        _enterRotateModeAnimation.Restart();
        _face->LookLeft();
        delay(100);
        _face->LookRight();
        delay(100);
        _face->LookFront();

        // Play mode-entry sound from folder 03, file 002
        _audioManager->playFolderFile(3, 2);

        // Don't start playing MP3 yet - wait for rotation
        Serial.println("Entering rotate mode 1 - rotate 15 degrees to play MP3 from folder 02");
        Serial.println("Rotate mode 1 active - will timeout after 45s of no rotation");
    }

    void enterRotateMode2() {
        _rotateMode = ROTATE_MODE_2;
        _rotateModeStartTime = millis();
        _lastRotationTime = millis();  // Initialize rotation timer

        // Hide eyes and set overlay callback
        _face->HideEyes = true;
        _face->OverlayCallback = staticDrawRotateMode2Overlay;

        // Play different animation: eyes look up and down
        // (Eyes won't be visible, but the Look position will be set)
        _enterRotateModeAnimation.Restart();
        _face->LookTop();
        delay(100);
        _face->LookBottom();
        delay(100);
        _face->LookFront();

        // Play mode-entry sound from folder 03, file 003
        _audioManager->playFolderFile(3, 3);

        Serial.print("Rotate mode 2 active - rotate 15 degrees to change volume (current: ");
        Serial.print(_audioManager->getCurrentVolume());
        Serial.println(")");
        Serial.println("Will timeout after 45s of no rotation");
    }

    void exitRotateMode() {
        RotateMode previousMode = _rotateMode;
        _rotateMode = ROTATE_MODE_NONE;

        // Restore eyes and clear overlay callback
        _face->HideEyes = false;
        _face->OverlayCallback = nullptr;

        // Play exit animation based on which mode we're exiting
        _exitRotateModeAnimation.Restart();

        if (previousMode == ROTATE_MODE_1) {
            // Rotate mode 1 exit: blink quickly
            _face->DoBlink();
            delay(150);
            _face->DoBlink();

            // Don't stop playback - let it continue playing
            Serial.println("Exiting rotate mode 1 - playback continues");
        // Play exit sound
        _audioManager->playFolderFile(3, 6);
    } else if (previousMode == ROTATE_MODE_2) {
        // Rotate mode 2 exit: eyes do a circle motion
        _face->LookLeft();
        delay(80);
        _face->LookTop();
        delay(80);
        _face->LookRight();
        delay(80);
        _face->LookBottom();
        delay(80);
        _face->LookFront();

        // Play exit sound
        _audioManager->playFolderFile(3, 6);
        }

        Serial.println("Rotate mode exited - normal behavior resumed");
    }
    void changeEmotionRandom() {
        // Exclude SLEEPY and strong negative emotions from random changes
        FaceEmotions exclude[] = {SLEEPY, ANGRY, FURIOUS};
        FaceEmotions newEmotion = _face->Behavior.GetRandomEmotionExcluding(exclude, 3);

        // Don't repeat the same emotion
        if (newEmotion == _currentEmotion) {
            // Try one more time
            newEmotion = _face->Behavior.GetRandomEmotionExcluding(exclude, 3);
        }

        changeEmotionTo(newEmotion);
        _audioManager->playEmotion(newEmotion);
    }

    void changeEmotionTo(FaceEmotions emotion) {
        Serial.print("Changing emotion to: ");
        Serial.println(getEmotionName(emotion));

        _currentEmotion = emotion;
        
        // Use GoTo_* methods based on emotion
        switch(emotion) {
            case NORMAL: _face->Expression.GoTo_Normal(); break;
            case HAPPY: _face->Expression.GoTo_Happy(); break;
            case ANGRY: _face->Expression.GoTo_Angry(); break;
            case SAD: _face->Expression.GoTo_Sad(); break;
            case SURPRISED: _face->Expression.GoTo_Surprised(); break;
            case SLEEPY: _face->Expression.GoTo_Sleepy(); break;
            case SCARED: _face->Expression.GoTo_Scared(); break;
            case FURIOUS: _face->Expression.GoTo_Furious(); break;
            case EXCITED: _face->Expression.GoTo_Excited(); break;
            case DISAPPOINTED: _face->Expression.GoTo_Disappointed(); break;
            case CONFUSED: _face->Expression.GoTo_Confused(); break;
            case CURIOUS: _face->Expression.GoTo_Curious(); break;
            case BORED: _face->Expression.GoTo_Bored(); break;
            case WORRIED: _face->Expression.GoTo_Worried(); break;
            case ANNOYED: _face->Expression.GoTo_Annoyed(); break;
            case SUSPICIOUS: _face->Expression.GoTo_Suspicious(); break;
            case SKEPTICAL: _face->Expression.GoTo_Skeptical(); break;
            case FRUSTRATED: _face->Expression.GoTo_Frustrated(); break;
        }
    }

    Face* _face;
    AudioManager* _audioManager;
    PowerManager* _powerManager;

    FaceEmotions _currentEmotion;
    bool _inTapMode;
    RotateMode _rotateMode;
    AsyncTimer _randomChangeTimer;
    uint32_t _randomChangeInterval;
    uint32_t _tapModeStartTime;
    uint32_t _lastMotionTime;
    uint32_t _motionIdleThreshold;
    bool _readyForMotionTrigger;
    bool _wasMoving;

    // Rotate mode state
    int _currentFolderFile;
    uint32_t _lastRotationCheck;
    uint32_t _rotateModeStartTime;      // When rotate mode was entered
    uint32_t _lastRotationTime;         // Last time rotation was detected
    uint32_t _rotateModeTimeout;        // Timeout for rotate mode (45s)
    RampAnimation _enterRotateModeAnimation;
    RampAnimation _exitRotateModeAnimation;

    // Overlay state for rotate modes
    float _pulsePhase;                  // Phase for pulsating circle animation (0-TWO_PI)
    bool _canSelectNextMp3;             // True when ready to select next MP3

    // Shake handling
    uint32_t _lastShakeTime;            // Last time a shake was handled
    uint32_t _shakeCooldown;            // Minimum ms between shake reactions
    bool _isInShakenMode;               // True while holding angry + buzzing during MP3 playback

    // Rotate mode 1 playback state
    bool _rotateMode1Playing;           // True while a folder-02 track is playing
    float _wavePhase;                   // Phase for scrolling wave animation during playback

    // Static overlay rendering methods (to be used as callbacks)
    static EmotionScheduler* _instance;  // Singleton instance for static callbacks

    void drawRotateMode1Overlay() {
        // Draw pulsating filled circle for rotate mode 1 (centered)
        extern U8G2* u8g2;

        // Pulse faster when at threshold to signal readiness
        float pulseSpeed = _canSelectNextMp3 ? 0.35f : 0.15f;
        _pulsePhase += pulseSpeed;
        if (_pulsePhase > TWO_PI) _pulsePhase -= TWO_PI;

        // Calculate radius based on whether we can select next MP3
        float baseRadius = _canSelectNextMp3 ? 20.0f : 12.0f;  // Bigger when at threshold
        float pulse = sin(_pulsePhase) * 0.5f + 0.5f;  // 0.0 to 1.0
        int radius = (int)(baseRadius + pulse * 6.0f);  // Pulsate ±6 pixels

        // Draw filled circle in center of screen
        int centerX = 64;  // Center X (128 / 2)
        int centerY = 32;  // Center Y (64 / 2)

        u8g2->drawDisc(centerX, centerY, radius);

        // When at threshold, draw an outer ring to clearly signal track-select is ready
        if (_canSelectNextMp3) {
            int ringRadius = radius + 5;
            u8g2->drawCircle(centerX, centerY, ringRadius);
            u8g2->drawCircle(centerX, centerY, ringRadius + 1);  // Double ring for emphasis
        }
    }

    void drawRotateMode2Overlay() {
        // Draw volume indicator for rotate mode 2
        extern U8G2* u8g2;

        // Get current volume (0-30) and convert to 1-10 scale
        uint8_t volume30 = _audioManager->getCurrentVolume();
        int volumeDisplay = (volume30 * 10) / 30 + 1;  // Convert 0-30 to 1-10
        if (volumeDisplay > 10) volumeDisplay = 10;
        if (volumeDisplay < 1) volumeDisplay = 1;

        // Draw volume number in center of screen
        char volumeStr[4];
        sprintf(volumeStr, "%d", volumeDisplay);

        u8g2->setFont(u8g2_font_inb30_mn);  // Large font for numbers

        // Calculate text width for centering
        int textWidth = u8g2->getStrWidth(volumeStr);
        int x = (128 - textWidth) / 2;
        int y = 48;  // Centered vertically (baseline)

        u8g2->drawStr(x, y, volumeStr);

        // Restore default font
        u8g2->setFont(u8g2_font_ncenB08_tr);
    }

    void drawPlaybackOverlay() {
        // Show real-time audio waveform from GPIO5 (DFPlayer DAC_L, AC-coupled to ~1.65V bias)
        extern U8G2* u8g2;

        // --- 1. Capture 128 ADC samples as fast as possible for a coherent snapshot ---
        int16_t samples[128];
        for (int i = 0; i < 128; i++) {
            samples[i] = (int16_t)analogRead(AUDIO_ADC_PIN);
        }

        // --- 2. Compute mean to dynamically remove DC bias ---
        int32_t sum = 0;
        for (int i = 0; i < 128; i++) sum += samples[i];
        int16_t mean = (int16_t)(sum / 128);

        // --- 3. Find peak deviation from mean for auto-scaling ---
        int16_t peak = 0;
        for (int i = 0; i < 128; i++) {
            int16_t dev = abs((int16_t)(samples[i] - mean));
            if (dev > peak) peak = dev;
        }
        // Noise floor: below 30 ADC counts (~25mV) treat as silence and show flat line
        if (peak < 30) peak = 0;
        // Scale divisor: map ±peak to ±amplitude pixels (avoid divide-by-zero)
        int16_t scale = (peak < 1) ? 1 : peak;

        const int waveY = 64;   // Baseline row — below the eye area (shifted 4px down)
        const int amplitude = 4; // ±4 pixel max deflection (rows 56–64)

        for (int x = 0; x < 128; x++) {
            int centered = (int)(samples[x] - mean);
            int yOffset = (peak == 0) ? 0 : ((centered * amplitude) / scale);
            if (yOffset >  amplitude) yOffset =  amplitude;
            if (yOffset < -amplitude) yOffset = -amplitude;
            int y = waveY - yOffset;  // positive offset → draw above baseline
            if (y < 0) y = 0;
            // Fill from the waveform point down to the bottom of the display
            u8g2->drawVLine(x, y, 64 - y);
        }
    }

    // Static wrappers for callbacks
    static void staticDrawRotateMode1Overlay() {
        if (_instance) _instance->drawRotateMode1Overlay();
    }

    static void staticDrawRotateMode2Overlay() {
        if (_instance) _instance->drawRotateMode2Overlay();
    }

    static void staticDrawPlaybackOverlay() {
        if (_instance) _instance->drawPlaybackOverlay();
    }
};

#endif // EMOTION_SCHEDULER_H
