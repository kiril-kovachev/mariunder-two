#ifndef TOUCH_SENSOR_MANAGER_H
#define TOUCH_SENSOR_MANAGER_H

#include <Arduino.h>
#include "Common.h"

// Manages TTP223 capacitive touch sensor for tap detection
class TouchSensorManager {
public:
    // Tap detection types (same as MotionManager for compatibility)
    enum TapType {
        NONE = 0,
        SINGLE_TAP = 1,
        DOUBLE_TAP = 2
    };

    TouchSensorManager() :
        _isInitialized(false),
        _tapEvent(NONE),
        _lastTouchState(false),
        _touchStartTime(0),
        _lastTapTime(0),
        _tapCount(0),
        _touchDebounceTime(50),      // 50ms debounce
        _doubleTapWindow(600),       // 600ms window for double tap
        _minTapInterval(100)         // Minimum 100ms between taps
    {}

    bool begin() {
        // Initialize touch sensor pin
        pinMode(TOUCH_SENSOR_PIN, INPUT);
        
        _isInitialized = true;
        _lastTouchState = digitalRead(TOUCH_SENSOR_PIN);
        
        Serial.println("TTP223 touch sensor initialized on GPIO7");
        return true;
    }

    void update() {
        if (!_isInitialized) return;

        uint32_t now = millis();
        bool currentState = digitalRead(TOUCH_SENSOR_PIN);

        // Detect state change (with debounce)
        if (currentState != _lastTouchState) {
            if ((now - _touchStartTime) > _touchDebounceTime) {
                if (currentState == HIGH) {
                    // Touch detected (rising edge)
                    handleTouchDetected(now);
                }
                _lastTouchState = currentState;
                _touchStartTime = now;
            }
        }

        // Check if single tap window expired
        if (_tapCount == 1 && (now - _lastTapTime) >= _doubleTapWindow) {
            if (_tapEvent == NONE) {  // Don't override double tap
                _tapEvent = SINGLE_TAP;
                Serial.println("Single tap detected!");
            }
            _tapCount = 0;
        }
    }

    TapType getTapEvent() {
        TapType event = _tapEvent;
        _tapEvent = NONE;  // Clear event after reading
        return event;
    }

    bool isTouched() {
        if (!_isInitialized) return false;
        return digitalRead(TOUCH_SENSOR_PIN) == HIGH;
    }

private:
    void handleTouchDetected(uint32_t now) {
        // Check if enough time passed since last tap
        if ((now - _lastTapTime) < _minTapInterval) {
            return; // Too soon, ignore
        }

        // Check if this could be part of a double tap
        if ((now - _lastTapTime) < _doubleTapWindow && _tapCount == 1) {
            // Second tap detected - it's a double tap!
            _tapEvent = DOUBLE_TAP;
            _tapCount = 0;  // Reset
            Serial.println("Double tap detected!");
        } else {
            // First tap detected
            _tapCount = 1;
            _lastTapTime = now;
            // Wait to see if there's a second tap
        }
    }

    bool _isInitialized;
    TapType _tapEvent;
    bool _lastTouchState;
    uint32_t _touchStartTime;
    uint32_t _lastTapTime;
    int _tapCount;
    uint32_t _touchDebounceTime;
    uint32_t _doubleTapWindow;
    uint32_t _minTapInterval;
};

#endif // TOUCH_SENSOR_MANAGER_H
