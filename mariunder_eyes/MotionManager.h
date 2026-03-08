#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include "Common.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// Manages MPU6050 motion sensor with DMP
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
        _shakeThreshold(0.8f),  // Threshold on acceleration spike above baseline (g)
        _motionThreshold(0.3f), // 0.3g for general motion
        _accumulatedRotation(0.0f),
        _lastRotationUpdateTime(0),
        _lastLoggedRotation(0.0f),
        _lastGyroX(0.0f),
        _lastGyroY(0.0f),
        _lastGyroZ(0.0f),
        _lastTap(NONE),
        _packetSize(0),
        _fifoCount(0),
        _lastAccelMagnitude(0.0f),
        _smoothedAccel(0.0f),
        _lastYaw(0.0f),
        _lastRotationYaw(0.0f)
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

        // Initialize and configure DMP
        Serial.println(F("Initializing DMP..."));
        uint8_t devStatus = _mpu.dmpInitialize();

        // Supply your own gyro offsets here, scaled for min sensitivity
        _mpu.setXGyroOffset(51);
        _mpu.setYGyroOffset(8);
        _mpu.setZGyroOffset(21);
        _mpu.setXAccelOffset(1150);
        _mpu.setYAccelOffset(-50);
        _mpu.setZAccelOffset(1060);

        // Make sure it worked (returns 0 if so)
        if (devStatus == 0) {
            // Turn on the DMP, now that it's ready
            Serial.println(F("Enabling DMP..."));
            _mpu.setDMPEnabled(true);

            Serial.println(F("DMP ready! Waiting for first data..."));
            _dmpReady = true;

            // Get expected DMP packet size for later comparison
            _packetSize = _mpu.dmpGetFIFOPacketSize();

            _isInitialized = true;
            _lastMotionTime = millis();
            return true;
        } else {
            // ERROR!
            // 1 = initial memory load failed
            // 2 = DMP configuration updates failed
            Serial.print(F("DMP Initialization failed (code "));
            Serial.print(devStatus);
            Serial.println(F(")"));
            return false;
        }
    }

    void update() {
        if (!_isInitialized || !_dmpReady) return;

        _lastTap = NONE; // Reset tap detection

        // Read FIFO and process DMP data
        _fifoCount = _mpu.getFIFOCount();

        // Check for overflow
        if (_fifoCount >= 1024) {
            _mpu.resetFIFO();
            Serial.println(F("FIFO overflow!"));
            return;
        }

        // Wait for correct available data length
        if (_fifoCount < _packetSize) return;

        // Drain FIFO, keeping only the most recent packet for processing
        uint8_t fifoBuffer[64];
        while (_fifoCount >= _packetSize) {
            _mpu.getFIFOBytes(fifoBuffer, _packetSize);
            _fifoCount -= _packetSize;
        }

        // Get quaternion for orientation
        Quaternion q;
        _mpu.dmpGetQuaternion(&q, fifoBuffer);

        // Get acceleration (gravity-compensated)
        VectorInt16 aa;
        VectorFloat gravity;
        VectorInt16 aaReal;
        VectorInt16 aaWorld;
        
        _mpu.dmpGetAccel(&aa, fifoBuffer);
        _mpu.dmpGetGravity(&gravity, &q);
        _mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        _mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);

        // Get yaw from DMP (radians)
        float ypr[3];
        _mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        _lastYaw = ypr[0];

        // Calculate motion magnitude (in raw units, ~8192 per g)
        float accelMagnitude = sqrt(
            aaWorld.x * aaWorld.x +
            aaWorld.y * aaWorld.y +
            aaWorld.z * aaWorld.z
        ) / 8192.0f; // Convert to g

        // Store for shake detection (with smoothing for baseline)
        _lastAccelMagnitude = accelMagnitude;
        _smoothedAccel = 0.9f * _smoothedAccel + 0.1f * accelMagnitude;

        // Check for motion
        if (accelMagnitude > _motionThreshold) {
            _lastMotionTime = millis();
        }

        // Get gyro for rotation and inertia tracking
        VectorInt16 gyro;
        _mpu.dmpGetGyro(&gyro, fifoBuffer);
        
        // Store for rotation tracking (degrees/second)
        _lastGyroX = gyro.x / 131.0f;
        _lastGyroY = gyro.y / 131.0f;
        _lastGyroZ = gyro.z / 131.0f;
    }

    bool isShaking() {
        if (!_isInitialized || !_dmpReady) return false;

        // Use a high-pass style check: spike above smoothed baseline
        float delta = _lastAccelMagnitude - _smoothedAccel;
        if (_lastAccelMagnitude < 0.4f) {
            return false; // Ignore very small motions entirely
        }
        return delta > _shakeThreshold;
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
        if (!_isInitialized || !_dmpReady) return 0.0f;
        // Return last gyro Z reading (degrees/second)
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
        _lastRotationYaw = _lastYaw;
    }

    void updateRotation() {
        if (!_isInitialized || !_dmpReady) return;

        uint32_t now = millis();
        if (_lastRotationUpdateTime == 0) {
            _lastRotationUpdateTime = now;
            _lastRotationYaw = _lastYaw;
            return;
        }

        float deltaTime = (now - _lastRotationUpdateTime) / 1000.0f;
        _lastRotationUpdateTime = now;

        if (deltaTime > 0.2f) return; // Skip large time jumps

        // Use change in yaw from DMP to accumulate rotation (radians)
        float deltaYaw = _lastYaw - _lastRotationYaw;

        // Ignore very small changes to avoid drift
        if (fabs(deltaYaw) < 0.01f) { // ~0.57 degrees
            return;
        }

        _lastRotationYaw = _lastYaw;
        _accumulatedRotation += deltaYaw;

        // Convert to degrees for easier threshold checking
        float degrees = _accumulatedRotation * RAD_TO_DEG;

        // Debug log only when rotation changes significantly (every 5 degrees)
        if (fabs(degrees - _lastLoggedRotation) >= 5.0f) {
            Serial.print("Rotation accumulated: ");
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
    float _lastGyroX;                 // Last gyro X reading (degrees/second)
    float _lastGyroY;                 // Last gyro Y reading (degrees/second)
    float _lastGyroZ;                 // Last gyro Z reading (degrees/second)

    // Tap detection
    TapType _lastTap;

    // DMP variables
    uint16_t _packetSize;    // Expected DMP packet size (default is 42 bytes)
    uint16_t _fifoCount;     // Count of all bytes currently in FIFO

    // Latest acceleration magnitude (g) for shake detection
    float _lastAccelMagnitude;
    float _smoothedAccel;

    // Latest yaw angle from DMP (radians) for rotate mode
    float _lastYaw;
    float _lastRotationYaw;
};

#endif // MOTION_MANAGER_H
