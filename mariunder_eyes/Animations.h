/***************************************************
 * Animations.h - Time-based animation classes for eye movements
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>

// Base animation class with time tracking
class AnimationBase {
public:
    unsigned long Interval;
    unsigned long StartTime;

    AnimationBase(unsigned long interval) : Interval(interval), StartTime(0) {}

    void Start() {
        StartTime = millis();
    }

    void Restart() {
        StartTime = millis();
    }

    unsigned long GetElapsed() {
        return millis() - StartTime;
    }

    float GetValue() {
        return Calculate(GetElapsed());
    }

    virtual float Calculate(unsigned long elapsedMillis) = 0;
};

// Ramp animation - linear 0 to 1
class RampAnimation : public AnimationBase {
public:
    bool IsActive = true;

    RampAnimation(unsigned long interval) : AnimationBase(interval) {}

    float Calculate(unsigned long elapsedMillis) override {
        if (elapsedMillis < Interval) {
            return static_cast<float>(elapsedMillis) / Interval;
        }
        return 1.0f;
    }
};

// Trapezium animation - ramp up, hold, ramp down
class TrapeziumAnimation : public AnimationBase {
public:
    unsigned long _t0;  // ramp up time
    unsigned long _t1;  // hold time
    unsigned long _t2;  // ramp down time

    TrapeziumAnimation(unsigned long t) : AnimationBase(t) {
        _t0 = t / 3;
        _t1 = _t0;
        _t2 = t - _t0 - _t1;
    }

    TrapeziumAnimation(unsigned long t0, unsigned long t1, unsigned long t2) 
        : AnimationBase(t0 + t1 + t2) {
        _t0 = t0;
        _t1 = t1;
        _t2 = t2;
    }

    float Calculate(unsigned long elapsedMillis) override {
        if (elapsedMillis > Interval) return 0.0;
        if (elapsedMillis < _t0) {
            return static_cast<float>(elapsedMillis) / _t0;
        }
        else if (elapsedMillis < _t0 + _t1) {
            return 1.0f;
        }
        else {
            return 1.0f - (static_cast<float>(elapsedMillis) - _t1 - _t0) / _t2;
        }
    }
};

// Trapezium pulse animation - repeating trapezium with pause
class TrapeziumPulseAnimation : public AnimationBase {
public:
    unsigned long _t0;  // initial delay
    unsigned long _t1;  // ramp up
    unsigned long _t2;  // hold
    unsigned long _t3;  // ramp down
    unsigned long _t4;  // end delay

    TrapeziumPulseAnimation(unsigned long t) : AnimationBase(t) {
        _t0 = 0;
        _t1 = t / 3;
        _t2 = t - _t0 - _t0;
        _t3 = _t1;
        _t4 = 0;
    }

    TrapeziumPulseAnimation(unsigned long t0, unsigned long t1, unsigned long t2, 
                          unsigned long t3, unsigned long t4) 
        : AnimationBase(t0 + t1 + t2 + t3 + t4) {
        _t0 = t0;
        _t1 = t1;
        _t2 = t2;
        _t3 = t3;
        _t4 = t4;
    }

    float Calculate(unsigned long elapsedMillis) override {
        unsigned long elapsed = elapsedMillis % Interval;

        if (elapsed < _t0) {
            return 0.0;
        }
        if (elapsed < _t0 + _t1) {
            return static_cast<float>(elapsed - _t0) / _t1;
        }
        else if (elapsed < _t0 + _t1 + _t2) {
            return 1.0f;
        }
        else if (elapsed < _t0 + _t1 + _t2 + _t3) {
            return 1.0f - (static_cast<float>(elapsed) - _t2 - _t1 - _t0) / _t3;
        }
        return 0.0;
    }

    void SetInterval(uint16_t t) {
        _t0 = 0;
        _t1 = t / 3;
        _t2 = t - _t0 - _t0;
        _t3 = _t1;
        _t4 = 0;
        Interval = _t0 + _t1 + _t2 + _t3 + _t4;
    }

    void SetTriangle(uint16_t t, uint16_t delay) {
        _t0 = 0;
        _t1 = t / 2;
        _t2 = 0;
        _t3 = _t1;
        _t4 = delay;
        Interval = _t0 + _t1 + _t2 + _t3 + _t4;
    }
};

#endif // ANIMATIONS_H
