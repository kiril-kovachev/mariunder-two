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
        LONG_PRESS = 2
    };

    TouchSensorManager() :
        _isInitialized(false),
        _tapEvent(NONE),
        _lastTouchState(false),
        _touchStartTime(0),
        _pressStartTime(0),
        _longPressTriggered(false),
        _touchDebounceTime(50),      // 50ms debounce
        _longPressThreshold(2000)    // 2s continuous press = long press
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
                    // Press started - record start time
                    _pressStartTime = now;
                    _longPressTriggered = false;
                } else {
                    // Press released
                    if (_pressStartTime > 0 && !_longPressTriggered) {
                        // Released before long press threshold -> single tap
                        _tapEvent = SINGLE_TAP;
                        Serial.println("Single tap detected!");
                    }
                    _pressStartTime = 0;
                }
                _lastTouchState = currentState;
                _touchStartTime = now;
            }
        }

        // Check for long press while key is still held
        if (_pressStartTime > 0 && !_longPressTriggered) {
            if ((now - _pressStartTime) >= _longPressThreshold) {
                _tapEvent = LONG_PRESS;
                _longPressTriggered = true;
                Serial.println("Long press detected!");
            }
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
    bool _isInitialized;
    TapType _tapEvent;
    bool _lastTouchState;
    uint32_t _touchStartTime;       // Last debounce edge time
    uint32_t _pressStartTime;       // When the current press started (0 = not pressed)
    bool _longPressTriggered;       // True if long press already fired for this press
    uint32_t _touchDebounceTime;
    uint32_t _longPressThreshold;   // Duration for long press (2000ms)
};

#endif // TOUCH_SENSOR_MANAGER_H
