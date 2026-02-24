/***************************************************
 * InertiaAssistant.cpp
 ****************************************************/

#include "InertiaAssistant.h"
#include "Face.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

InertiaAssistant::InertiaAssistant(Face& face)
    : _face(face),
      _motionManager(nullptr),
      _velocityX(0),
      _velocityY(0),
      _positionX(0),
      _positionY(0),
      _lastGyroX(0),
      _lastGyroY(0),
      _lastGyroZ(0),
      _lastUpdateTime(0),
      _lastLoggedPositionMag(0) {}

void InertiaAssistant::SetMotionManager(MotionManager* motionManager) {
    _motionManager = motionManager;
}

void InertiaAssistant::Update() {
    if (!Enabled || _motionManager == nullptr) {
        return;
    }

    uint32_t currentTime = millis();

    // Initialize timing on first update
    if (_lastUpdateTime == 0) {
        _lastUpdateTime = currentTime;
        return;
    }

    // Calculate time delta in seconds
    float deltaTime = (currentTime - _lastUpdateTime) / 1000.0f;
    _lastUpdateTime = currentTime;

    // Clamp delta time to prevent large jumps
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    // Get gyroscope data
    sensors_event_t accel, gyro, temp;
    _motionManager->_mpu.getEvent(&accel, &gyro, &temp);

    // Apply rotation data to create inertia effect
    // Gyro gives rotation rate in rad/s
    // We'll use X and Y rotation to move the eyes
    float gyroX = gyro.gyro.x;  // Pitch (rotation around X-axis)
    float gyroY = gyro.gyro.y;  // Roll (rotation around Y-axis)

    // Convert rotation to acceleration on eye position
    // When device rotates right, eyes should lag left (inertia)
    float accelX = -gyroY * Sensitivity * 10.0f;  // Negate for natural feel
    float accelY = gyroX * Sensitivity * 10.0f;

    // Update velocity with acceleration and damping
    _velocityX += accelX * deltaTime;
    _velocityY += accelY * deltaTime;

    // Apply damping to velocity
    _velocityX *= Damping;
    _velocityY *= Damping;

    // Add return force towards center (spring-like behavior)
    _velocityX -= _positionX * ReturnSpeed;
    _velocityY -= _positionY * ReturnSpeed;

    // Update position
    _positionX += _velocityX * deltaTime;
    _positionY += _velocityY * deltaTime;

    // Clamp position to max displacement
    if (_positionX > MaxDisplacement) _positionX = MaxDisplacement;
    if (_positionX < -MaxDisplacement) _positionX = -MaxDisplacement;
    if (_positionY > MaxDisplacement) _positionY = MaxDisplacement;
    if (_positionY < -MaxDisplacement) _positionY = -MaxDisplacement;

    // Calculate movement magnitude to determine if inertia effect is significant
    float positionMagnitude = sqrt(_positionX * _positionX + _positionY * _positionY);
    float velocityMagnitude = sqrt(_velocityX * _velocityX + _velocityY * _velocityY);
    const float LOG_THRESHOLD = 0.5f;  // Only log when movement is significant (0.5 pixels)

    // Apply to eyes via InertiaTransformation
    Transformation t;
    t.MoveX = _positionX;
    t.MoveY = _positionY;
    t.ScaleX = 1.0f;
    t.ScaleY = 1.0f;

    // Log when inertia effect changes significantly (every 1.0 pixel change in position magnitude)
    // This prevents spam while still providing useful debug information
    if ((positionMagnitude > LOG_THRESHOLD || velocityMagnitude > 0.1f) &&
        abs(positionMagnitude - _lastLoggedPositionMag) >= 1.0f) {
        Serial.print("[Inertia] Effect triggered - Pos: (");
        Serial.print(_positionX);
        Serial.print(", ");
        Serial.print(_positionY);
        Serial.print(") Vel: (");
        Serial.print(_velocityX);
        Serial.print(", ");
        Serial.print(_velocityY);
        Serial.print(") Gyro: (");
        Serial.print(gyroX);
        Serial.print(", ");
        Serial.print(gyroY);
        Serial.println(")");
        _lastLoggedPositionMag = positionMagnitude;
    }

    // Update destination - SetDestin updates Origin to Current, so animation smoothly interpolates
    // Don't restart every frame - let the animation interpolate continuously
    // This creates the lag effect: eyes smoothly follow the physics position
    
    // Check if destination changed significantly (more than 0.5 pixels)
    float deltaX = t.MoveX - _face.LeftEye.InertiaTransformation.Destin.MoveX;
    float deltaY = t.MoveY - _face.LeftEye.InertiaTransformation.Destin.MoveY;
    float deltaMag = sqrt(deltaX * deltaX + deltaY * deltaY);
    
    _face.LeftEye.InertiaTransformation.SetDestin(t);
    _face.RightEye.InertiaTransformation.SetDestin(t);
    
    // Only restart animation if destination changed significantly or animation completed
    // This ensures smooth interpolation while allowing the animation to progress
    if (deltaMag > 0.5f || _face.LeftEye.InertiaTransformation.Animation.GetValue() >= 1.0f) {
        _face.LeftEye.InertiaTransformation.Animation.Restart();
        _face.RightEye.InertiaTransformation.Animation.Restart();
    }
}

void InertiaAssistant::Reset() {
    _velocityX = 0;
    _velocityY = 0;
    _positionX = 0;
    _positionY = 0;
    _lastGyroX = 0;
    _lastGyroY = 0;
    _lastGyroZ = 0;
    _lastUpdateTime = 0;
    _lastLoggedPositionMag = 0;
}
