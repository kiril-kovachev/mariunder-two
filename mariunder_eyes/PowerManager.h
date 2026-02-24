#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include "MotionManager.h"
#include "FaceEmotions.hpp"

// Power state enumeration
enum PowerState {
    POWER_ACTIVE,       // Normal operation
    POWER_SLEEPY,       // Preparing for sleep (30s idle)
    POWER_DEEP_SLEEP    // Will enter deep sleep mode (60s idle)
};

// Manages power states and idle detection
class PowerManager {
public:
    PowerManager() :
        _motionManager(nullptr),
        _currentState(POWER_ACTIVE),
        _lastActivityTime(0),
        _sleepyTimeout(30000),     // 30 seconds
        _deepSleepTimeout(60000)   // 60 seconds total
    {}

    void begin(MotionManager* motion) {
        _motionManager = motion;
        _lastActivityTime = millis();
        _currentState = POWER_ACTIVE;

        Serial.println("PowerManager initialized");
    }

    void update() {
        if (_motionManager == nullptr) return;

        // Check for motion/activity
        if (_motionManager->isMoving()) {
            // Activity detected - reset to active state
            if (_currentState != POWER_ACTIVE) {
                Serial.println("Activity detected - returning to POWER_ACTIVE state");
                _currentState = POWER_ACTIVE;
            }
            _lastActivityTime = millis();
            return;
        }

        // Calculate idle time
        uint32_t idleTime = millis() - _lastActivityTime;

        // State transitions based on idle time
        switch (_currentState) {
            case POWER_ACTIVE:
                if (idleTime >= _sleepyTimeout) {
                    enterSleepyMode();
                }
                break;

            case POWER_SLEEPY:
                if (idleTime >= _deepSleepTimeout) {
                    // Enter deep sleep
                    enterDeepSleep();
                }
                break;

            case POWER_DEEP_SLEEP:
                // Should not reach here (ESP32 will be sleeping)
                break;
        }
    }

    PowerState getState() const {
        return _currentState;
    }

    bool canShowEmotion(FaceEmotions emotion) const {
        // In POWER_SLEEPY mode, only allow SLEEPY emotion
        if (_currentState == POWER_SLEEPY) {
            return emotion == SLEEPY;
        }
        return true;
    }

    void resetActivity() {
        // Manual activity reset
        _lastActivityTime = millis();
        if (_currentState != POWER_ACTIVE) {
            _currentState = POWER_ACTIVE;
        }
    }

    uint32_t getIdleTime() const {
        return millis() - _lastActivityTime;
    }

    // Execute deep sleep after sleepy eyes are displayed
    void executeDeepSleep() {
        // This is called after sleepy eyes are displayed
        Serial.println("Configuring wake-on-motion...");

        // Configure MPU6050 for wake interrupt
        _motionManager->enableWakeInterrupt();

        // Give serial time to finish
        Serial.flush();
        delay(100);

        // Configure ESP32-C6 wake sources
        // ext1 wake on multiple pins going LOW
        // Bit mask: GPIO2 (MPU6050) and GPIO7 (TTP223)
        uint64_t wake_mask = (1ULL << MPU_INT_PIN) | (1ULL << TOUCH_SENSOR_PIN);
        esp_sleep_enable_ext1_wakeup(wake_mask, ESP_EXT1_WAKEUP_ANY_LOW);

        Serial.println("Going to sleep... Shake or touch to wake!");
        Serial.flush();
        delay(100);

        // Enter deep sleep
        esp_deep_sleep_start();

        // Code will not reach here - ESP32 will restart on wake
    }

private:
    void enterSleepyMode() {
        Serial.println("Entering POWER_SLEEPY mode (30s idle)");
        _currentState = POWER_SLEEPY;
        // The EmotionScheduler will handle switching to SLEEPY emotion
    }

    void enterDeepSleep() {
        Serial.println("Entering POWER_DEEP_SLEEP mode");

        // Set state but don't actually sleep yet
        // This allows the main loop to display sleepy eyes first
        _currentState = POWER_DEEP_SLEEP;
    }

    MotionManager* _motionManager;
    PowerState _currentState;
    uint32_t _lastActivityTime;
    uint32_t _sleepyTimeout;
    uint32_t _deepSleepTimeout;
};

#endif // POWER_MANAGER_H
