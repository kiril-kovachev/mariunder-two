/***************************************************
 * InertiaAssistant.cpp
 ****************************************************/

#include "InertiaAssistant.h"
#include "Face.h"

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

    // Get gyroscope data from MPU6050 directly (raw readings)
    // This is more efficient than processing FIFO for inertia
    int16_t gx, gy, gz;
    _motionManager->_mpu.getRotation(&gx, &gy, &gz);

    // Convert raw gyro readings to rad/s (gyro sensitivity at 500 deg/s is 65.5 LSB/(deg/s))
    // At 500 deg/s range: 1 deg/s = 65.5 LSB
    float gyroX = (gx / 65.5f) * (PI / 180.0f);  // Convert to rad/s
    float gyroY = (gy / 65.5f) * (PI / 180.0f);

    // Apply rotation data to create inertia effect
    // Gyro gives rotation rate in rad/s
    // We'll use X and Y rotation to move the eyes
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
