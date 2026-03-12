#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include "Kalman.h"
#include "Common.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// Manages MPU6050 motion sensor using Kalman filter fusion
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
        _dmpReady(false),
        _shakeThreshold(0.8f),  // delta g above baseline for shake
        _motionThreshold(0.3f), // 0.3g for general motion
        _accumulatedRotation(0.0f),
        _lastRotationUpdateTime(0),
        _lastLoggedRotation(0.0f),
        _kalAngleX(0.0f),
        _kalAngleY(0.0f),
        _lastAngleForRotate(0.0f),
        _lastGyroX(0.0f),
        _lastGyroY(0.0f),
        _lastGyroZ(0.0f),
        _gyroOffsetX(0.0f),
        _gyroOffsetY(0.0f),
        _gyroOffsetZ(0.0f),
        _gyroCalibrated(false),
        _lastTap(NONE),
        _packetSize(0),
        _fifoCount(0),
        _lastAccelMagnitude(0.0f),
        _smoothedAccel(0.0f),
        _lastYaw(0.0f),
        _lastRotationYaw(0.0f),
        _lastUpdateTime(0)
    {}

    bool begin() {
        // Initialize I2C
        #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
            Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
            Wire.setClock(400000); // 400kHz I2C clock
        #endif

        // Initialize MPU6050
        Serial.println(F("Initializing MPU6050..."));
        _mpu.initialize();
        
        // Verify connection
        Serial.println(F("Testing MPU6050 connection..."));
        if (!_mpu.testConnection()) {
            Serial.println(F("MPU6050 connection failed!"));
            return false;
        }
        Serial.println(F("MPU6050 connection successful"));

        // No DMP usage here; we work directly with raw data + Kalman filter

        // Gyro bias calibration (2 seconds, device must be stationary)
        Serial.println(F("Starting 2s gyro calibration (keep device still)..."));
        _gyroOffsetX = _gyroOffsetY = _gyroOffsetZ = 0.0f;
        _gyroCalibrated = false;

        int16_t ax, ay, az, gx, gy, gz;
        uint32_t calibStart = millis();
        uint32_t samples = 0;

        while (millis() - calibStart < 2000) {
            _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

            // Convert raw gyro to deg/s (assuming default 250deg/s range -> 131 LSB/deg/s)
            float rawGyroX = gx / 131.0f;
            float rawGyroY = gy / 131.0f;
            float rawGyroZ = gz / 131.0f;

            _gyroOffsetX += rawGyroX;
            _gyroOffsetY += rawGyroY;
            _gyroOffsetZ += rawGyroZ;
            samples++;

            delay(5);
        }

        if (samples > 0) {
            _gyroOffsetX /= samples;
            _gyroOffsetY /= samples;
            _gyroOffsetZ /= samples;
        }
        _gyroCalibrated = true;
        Serial.println(F("Gyro calibration complete."));

        // Initialize Kalman filter angles from accelerometer so they start aligned
        _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        float accX = ax / 16384.0f;
        float accY = ay / 16384.0f;
        float accZ = az / 16384.0f;

        float accAngleX = atan2(accY, accZ) * RAD_TO_DEG;
        float accAngleY = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;

        _kalAngleX = accAngleX;
        _kalAngleY = accAngleY;
        _kalmanX.setAngle(_kalAngleX);
        _kalmanY.setAngle(_kalAngleY);
        _lastAngleForRotate = _kalAngleX;

        _lastMotionTime = millis();
        _lastUpdateTime = millis();
        _isInitialized = true;

        Serial.println(F("MotionManager initialized with Kalman filter."));
        return true;
    }

    void update() {
        if (!_isInitialized || !_gyroCalibrated) return;

        _lastTap = NONE; // Reset tap detection

        int16_t ax, ay, az, gx, gy, gz;
        _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        uint32_t now = millis();
        float dt = (now - _lastUpdateTime) / 1000.0f;
        if (dt <= 0.0f || dt > 0.1f) {
            dt = 0.01f;
        }
        _lastUpdateTime = now;

        // Convert accelerometer to g and compute magnitude
        float accX = ax / 16384.0f;
        float accY = ay / 16384.0f;
        float accZ = az / 16384.0f;

        float accelMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);
        _lastAccelMagnitude = accelMagnitude;
        _smoothedAccel = 0.9f * _smoothedAccel + 0.1f * accelMagnitude;

        // Detect general motion
        if (accelMagnitude > _motionThreshold) {
            _lastMotionTime = now;
        }

        // Convert gyro to deg/s, apply calibration and deadband
        float rawGyroX = gx / 131.0f - _gyroOffsetX;
        float rawGyroY = gy / 131.0f - _gyroOffsetY;
        float rawGyroZ = gz / 131.0f - _gyroOffsetZ;

        _lastGyroX = (fabs(rawGyroX) < 0.5f) ? 0.0f : rawGyroX;
        _lastGyroY = (fabs(rawGyroY) < 0.5f) ? 0.0f : rawGyroY;
        _lastGyroZ = (fabs(rawGyroZ) < 0.5f) ? 0.0f : rawGyroZ;

        // Compute accelerometer-only angles (degrees)
        float accAngleX = atan2(accY, accZ) * RAD_TO_DEG;
        float accAngleY = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;

        // Kalman filter fusion (TKJElectronics pattern)
        float gyroXrate = _lastGyroX; // deg/s
        float gyroYrate = _lastGyroY;

        _kalAngleX = _kalmanX.getAngle(accAngleX, gyroXrate, dt);
        _kalAngleY = _kalmanY.getAngle(accAngleY, gyroYrate, dt);
    }

    bool isShaking() {
        if (!_isInitialized || !_gyroCalibrated) return false;

        // Require general motion as well to avoid false shakes when stationary
        if (!isMoving()) return false;

        // Use a high-pass style check: spike above smoothed baseline
        float delta = _lastAccelMagnitude - _smoothedAccel;

        // Ignore very small overall accelerations
        if (_lastAccelMagnitude < 0.6f) {
            return false;
        }

        return delta > _shakeThreshold;
    }

    bool isMoving() {
        if (!_isInitialized || !_gyroCalibrated) return false;

        // Check if any motion detected recently (within last 500ms)
        return (millis() - _lastMotionTime) < 500;
    }

    uint32_t getLastMotionTime() {
        return _lastMotionTime;
    }

    // Rotation tracking for rotate modes (based on Kalman X angle)
    float getRotationZ() {
        if (!_isInitialized || !_gyroCalibrated) return 0.0f;
        return _lastGyroZ;
    }

    // Latest gyro readings (degrees/second) for inertia
    float getGyroX() const { return _lastGyroX; }
    float getGyroY() const { return _lastGyroY; }
    float getGyroZ() const { return _lastGyroZ; }

    // Get accumulated rotation angle (integrated over time)
    float getAccumulatedRotation() {
        return _accumulatedRotation;
    }

    void resetAccumulatedRotation() {
        _accumulatedRotation = 0.0f;
        _lastRotationUpdateTime = millis();
        _lastLoggedRotation = 0.0f;
        _lastAngleForRotate = _kalAngleX;
    }

    void updateRotation() {
        if (!_isInitialized || !_gyroCalibrated) return;

        // Only react to rotation when accelerometer-based motion is detected
        if (!isMoving()) {
            // When not moving, slowly decay accumulated rotation back to zero
            _accumulatedRotation *= 0.95f;
            return;
        }

        uint32_t now = millis();
        if (_lastRotationUpdateTime == 0) {
            _lastRotationUpdateTime = now;
            _lastAngleForRotate = _kalAngleX;
            return;
        }

        float deltaTime = (now - _lastRotationUpdateTime) / 1000.0f;
        _lastRotationUpdateTime = now;
        if (deltaTime > 0.2f) return; // Skip large time jumps

        // Use change in Kalman X angle (roll) as rotation measure
        float currentAngle = _kalAngleX; // degrees
        float deltaDeg = currentAngle - _lastAngleForRotate;

        // Ignore very small changes to avoid drift
        if (fabs(deltaDeg) < 1.0f) {
            return;
        }

        _lastAngleForRotate = currentAngle;
        _accumulatedRotation += deltaDeg * DEG_TO_RAD; // store in radians

        // Debug log only when rotation changes significantly (every 5 degrees)
        float degrees = _accumulatedRotation * RAD_TO_DEG;
        if (fabs(degrees - _lastLoggedRotation) >= 5.0f) {
            Serial.print("Rotation accumulated (Kalman): ");
            Serial.print(degrees);
            Serial.println(" degrees");
            _lastLoggedRotation = degrees;
        }
    }

    void enableWakeInterrupt() {
        // Configure for wake-from-sleep
        if (!_isInitialized) return;

        // Enable motion detection interrupt
        _mpu.setIntEnabled(0x40); // Enable motion detection interrupt
    }

    // Get tap event (for compatibility with TouchSensorManager interface)
    TapType getTapEvent() {
        TapType result = _lastTap;
        _lastTap = NONE; // Clear after reading
        return result;
    }

    // Public access to MPU6050 for advanced features (like inertia)
    MPU6050 _mpu;

private:
    uint32_t _lastMotionTime;
    bool _isInitialized;
    bool _dmpReady;
    float _shakeThreshold;    // delta in g above smoothed baseline required for shake
    float _motionThreshold;   // in g

    // Rotation tracking
    float _accumulatedRotation;       // Accumulated rotation angle in radians
    uint32_t _lastRotationUpdateTime; // Last time rotation was updated
    float _lastLoggedRotation;        // Last logged rotation value to prevent spam

    // Kalman filters for tilt angles
    Kalman _kalmanX;
    Kalman _kalmanY;
    float _kalAngleX;          // degrees
    float _kalAngleY;          // degrees
    float _lastAngleForRotate; // degrees

    // Latest gyro readings (degrees/second) after calibration and deadband
    float _lastGyroX;
    float _lastGyroY;
    float _lastGyroZ;

    // Gyro calibration
    float _gyroOffsetX;
    float _gyroOffsetY;
    float _gyroOffsetZ;
    bool  _gyroCalibrated;

    // Tap detection
    TapType _lastTap;

    // DMP variables (no longer used for MotionManager logic, kept for compatibility)
    uint16_t _packetSize;    // Unused
    uint16_t _fifoCount;     // Unused

    // Latest acceleration magnitude (g) for shake detection
    float _lastAccelMagnitude;
    float _smoothedAccel;

    // Latest yaw angle from DMP (unused in Kalman-based logic, kept for compatibility)
    float _lastYaw;
    float _lastRotationYaw;

    // Timing
    uint32_t _lastUpdateTime;
};

#endif // MOTION_MANAGER_H
