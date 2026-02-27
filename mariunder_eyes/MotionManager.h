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
        _shakeThreshold(1.5f),  // 1.5g
        _motionThreshold(0.3f), // 0.3g for general motion
        _accumulatedRotation(0.0f),
        _lastRotationUpdateTime(0),
        _lastLoggedRotation(0.0f),
        _lastTap(NONE),
        _packetSize(0),
        _fifoCount(0)
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

            // Enable tap detection
            _mpu.setTapDetectionOnXYZ(true, true, true);
            _mpu.setTapThreshold(3, 250);  // X/Y/Z axes, threshold

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

        // Read a packet from FIFO
        uint8_t fifoBuffer[64];
        _mpu.getFIFOBytes(fifoBuffer, _packetSize);

        // Track FIFO count in case there is > 1 packet available
        _fifoCount -= _packetSize;

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

        // Calculate motion magnitude (in raw units, ~8192 per g)
        float accelMagnitude = sqrt(
            aaWorld.x * aaWorld.x +
            aaWorld.y * aaWorld.y +
            aaWorld.z * aaWorld.z
        ) / 8192.0f; // Convert to g

        // Check for motion
        if (accelMagnitude > _motionThreshold) {
            _lastMotionTime = millis();
        }

        // Check for tap detection
        uint8_t tapStatus = _mpu.getTapDetectStatus();
        if (tapStatus & 0x01) { // X-axis tap
            _lastTap = SINGLE_TAP;
            Serial.println(F("Tap detected on X-axis"));
        }
        if (tapStatus & 0x02) { // Y-axis tap
            _lastTap = SINGLE_TAP;
            Serial.println(F("Tap detected on Y-axis"));
        }
        if (tapStatus & 0x04) { // Z-axis tap
            _lastTap = SINGLE_TAP;
            Serial.println(F("Tap detected on Z-axis"));
        }

        // Get gyro for rotation tracking
        VectorInt16 gyro;
        _mpu.dmpGetGyro(&gyro, fifoBuffer);
        
        // Store for rotation tracking
        _lastGyroZ = gyro.z / 131.0f; // Convert to degrees/second
    }

    bool isShaking() {
        if (!_isInitialized || !_dmpReady) return false;

        // Check if FIFO has data
        uint16_t fifoCount = _mpu.getFIFOCount();
        if (fifoCount < _packetSize) return false;

        // Read a packet from FIFO
        uint8_t fifoBuffer[64];
        _mpu.getFIFOBytes(fifoBuffer, _packetSize);

        // Get acceleration (gravity-compensated)
        Quaternion q;
        VectorInt16 aa;
        VectorFloat gravity;
        VectorInt16 aaReal;
        VectorInt16 aaWorld;
        
        _mpu.dmpGetQuaternion(&q, fifoBuffer);
        _mpu.dmpGetAccel(&aa, fifoBuffer);
        _mpu.dmpGetGravity(&gravity, &q);
        _mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        _mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);

        // Calculate acceleration magnitude (in g)
        float accelMagnitude = sqrt(
            aaWorld.x * aaWorld.x +
            aaWorld.y * aaWorld.y +
            aaWorld.z * aaWorld.z
        ) / 8192.0f; // Convert to g

        return accelMagnitude > _shakeThreshold;
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
        if (!_isInitialized || !_dmpReady) return;

        uint32_t now = millis();
        if (_lastRotationUpdateTime == 0) {
            _lastRotationUpdateTime = now;
            return;
        }

        float deltaTime = (now - _lastRotationUpdateTime) / 1000.0f;
        _lastRotationUpdateTime = now;

        if (deltaTime > 0.1f) return; // Skip large time jumps

        // Integrate Z-axis rotation over time to get angle
        // _lastGyroZ is in degrees/s, multiply by deltaTime to get degrees
        float rotationRate = _lastGyroZ * DEG_TO_RAD; // Convert to radians/s
        _accumulatedRotation += rotationRate * deltaTime;

        // Convert to degrees for easier threshold checking
        float degrees = _accumulatedRotation * RAD_TO_DEG;

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
    float _shakeThreshold;    // in g
    float _motionThreshold;   // in g

    // Rotation tracking
    float _accumulatedRotation;       // Accumulated rotation angle in radians
    uint32_t _lastRotationUpdateTime; // Last time rotation was updated
    float _lastLoggedRotation;        // Last logged rotation value to prevent spam
    float _lastGyroZ;                 // Last gyro Z reading (degrees/second)

    // Tap detection
    TapType _lastTap;

    // DMP variables
    uint16_t _packetSize;    // Expected DMP packet size (default is 42 bytes)
    uint16_t _fifoCount;     // Count of all bytes currently in FIFO
};

#endif // MOTION_MANAGER_H
