/***************************************************
 * InertiaAssistant.h - Eye inertia effects from device rotation
 * Applies physics-based eye movement based on MPU6050 gyroscope
 ****************************************************/

#ifndef INERTIA_ASSISTANT_H
#define INERTIA_ASSISTANT_H

#include <Arduino.h>
#include "EyeTransformation.h"
#include "MotionManager.h"

// Forward declaration
class Face;

// Manages inertia-based eye movement from device rotation
class InertiaAssistant {
public:
    InertiaAssistant(Face& face);

    // Configuration parameters
    bool Enabled = true;                    // Enable/disable inertia effect
    float Sensitivity = 1.0f;               // Multiplier for rotation influence (0.1 - 5.0)
    float Damping = 0.85f;                  // Velocity damping factor (0.0 - 1.0, higher = more sluggish)
    float MaxDisplacement = 8.0f;           // Maximum eye displacement in pixels
    float ReturnSpeed = 0.05f;              // Speed of returning to center when no rotation (0.01 - 0.2)

    void SetMotionManager(MotionManager* motionManager);
    void Update();
    void Reset();

protected:
    Face& _face;
    MotionManager* _motionManager;

    // Inertia state
    float _velocityX;
    float _velocityY;
    float _positionX;
    float _positionY;

    // Last gyro readings for calculating delta
    float _lastGyroX;
    float _lastGyroY;
    float _lastGyroZ;
    uint32_t _lastUpdateTime;

    // For preventing log spam
    float _lastLoggedPositionMag;  // Last logged position magnitude

    void ApplyInertia(float gyroX, float gyroY, float gyroZ);
};

#endif // INERTIA_ASSISTANT_H
