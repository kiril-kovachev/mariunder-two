#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Common.h"

// Manages MPU6050 motion sensor
class MotionManager {
public:
    // Tap detection types
    enum TapType {
        NONE = 0,
        SINGLE_TAP = 1,
        DOUBLE_TAP = 2
    };

    MotionManager() :
        _lastMotionTime(0),
        _isInitialized(false),
        _shakeThreshold(1.5f),  // 1.5g
        _motionThreshold(0.3f), // 0.3g for general motion
        _accumulatedRotation(0.0f),
        _lastRotationUpdateTime(0),
        _lastLoggedRotation(0.0f)
    {}

    bool begin() {
        if (!_mpu.begin()) {
            Serial.println("Failed to initialize MPU6050!");
            return false;
        }

        Serial.println("MPU6050 initialized");

        // Configure MPU6050
        _mpu.setAccelerometerRange(MPU6050_RANGE_4_G);   // ±4g range
        _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

        // Configure motion interrupt (for wake-on-motion)
        _mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
        _mpu.setMotionDetectionThreshold(20);  // 20 * 2mg = 40mg threshold
        _mpu.setMotionDetectionDuration(20);   // 20ms duration
        _mpu.setInterruptPinLatch(true);
        _mpu.setInterruptPinPolarity(false);   // Active LOW
        _mpu.setMotionInterrupt(true);

        _isInitialized = true;
        _lastMotionTime = millis();

        return true;
    }

    void update() {
        if (!_isInitialized) return;

        // Read sensor data
        sensors_event_t accel, gyro, temp;
        _mpu.getEvent(&accel, &gyro, &temp);

        // Calculate total acceleration magnitude
        float accelMagnitude = sqrt(
            accel.acceleration.x * accel.acceleration.x +
            accel.acceleration.y * accel.acceleration.y +
            accel.acceleration.z * accel.acceleration.z
        );

        // Remove gravity (9.8 m/s²) to get actual motion
        float motionMagnitude = abs(accelMagnitude - 9.8f);

        // Check for motion
        if (motionMagnitude > _motionThreshold) {
            _lastMotionTime = millis();
        }
    }

    bool isShaking() {
        if (!_isInitialized) return false;

        sensors_event_t accel, gyro, temp;
        _mpu.getEvent(&accel, &gyro, &temp);

        // Calculate acceleration magnitude
        float accelMagnitude = sqrt(
            accel.acceleration.x * accel.acceleration.x +
            accel.acceleration.y * accel.acceleration.y +
            accel.acceleration.z * accel.acceleration.z
        );

        // Check if magnitude exceeds shake threshold
        float motionMagnitude = abs(accelMagnitude - 9.8f);
        return motionMagnitude > _shakeThreshold * 9.8f;  // Convert g to m/s²
    }

    bool isMoving() {
        if (!_isInitialized) return false;

        // Check if any motion detected recently (within last 500ms)
        return (millis() - _lastMotionTime) < 500;
    }

    uint32_t getLastMotionTime() {
        return _lastMotionTime;
    }

    // Rotation tracking for rotate modes
    float getRotationZ() {
        if (!_isInitialized) return 0.0f;

        sensors_event_t accel, gyro, temp;
        _mpu.getEvent(&accel, &gyro, &temp);

        // Return Z-axis gyroscope reading (rotation around vertical axis)
        return gyro.gyro.z;  // rad/s
    }

    // Get accumulated rotation angle (integrated over time)
    float getAccumulatedRotation() {
        return _accumulatedRotation;
    }

    void resetAccumulatedRotation() {
        _accumulatedRotation = 0.0f;
        _lastRotationUpdateTime = millis();
        _lastLoggedRotation = 0.0f;
    }

    void updateRotation() {
        if (!_isInitialized) return;

        uint32_t now = millis();
        if (_lastRotationUpdateTime == 0) {
            _lastRotationUpdateTime = now;
            return;
        }

        float deltaTime = (now - _lastRotationUpdateTime) / 1000.0f;
        _lastRotationUpdateTime = now;

        if (deltaTime > 0.1f) return; // Skip large time jumps

        sensors_event_t accel, gyro, temp;
        _mpu.getEvent(&accel, &gyro, &temp);

        // Integrate Z-axis rotation over time to get angle
        // gyro.z is in rad/s, multiply by deltaTime to get radians
        float rotationRate = gyro.gyro.z;
        _accumulatedRotation += rotationRate * deltaTime;

        // Convert to degrees for easier threshold checking
        float degrees = _accumulatedRotation * 180.0f / PI;

        // Debug log only when rotation changes significantly (every 5 degrees)
        if (abs(degrees - _lastLoggedRotation) >= 5.0f) {
            Serial.print("Rotation accumulated: ");
            Serial.print(degrees);
            Serial.println(" degrees");
            _lastLoggedRotation = degrees;
        }
    }

    void enableWakeInterrupt() {
        // Configure for wake-from-sleep
        // This will be called before entering deep sleep
        if (!_isInitialized) return;

        // MPU6050 motion interrupt is already configured
        // Just ensure it's enabled
        _mpu.setMotionInterrupt(true);
    }

    // Public access to MPU6050 for advanced features (like inertia)
    Adafruit_MPU6050 _mpu;

private:
    uint32_t _lastMotionTime;
    bool _isInitialized;
    float _shakeThreshold;    // in g
    float _motionThreshold;   // in g

    // Rotation tracking
    float _accumulatedRotation;       // Accumulated rotation angle in radians
    uint32_t _lastRotationUpdateTime; // Last time rotation was updated
    float _lastLoggedRotation;        // Last logged rotation value to prevent spam
};

#endif // MOTION_MANAGER_H
