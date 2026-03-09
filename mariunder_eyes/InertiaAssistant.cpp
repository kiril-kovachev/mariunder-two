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

    // Get latest gyroscope data from MotionManager (degrees/second)
    float gyroDegX = _motionManager->getGyroX();
    float gyroDegY = _motionManager->getGyroY();

    // If there is essentially no rotation, slowly return to center and skip
    float gyroMag = sqrt(gyroDegX * gyroDegX + gyroDegY * gyroDegY);
    if (gyroMag < 5.0f) { // deadband ~5 deg/s
        // Apply only return-to-center when no significant rotation
        _velocityX -= _positionX * ReturnSpeed;
        _velocityY -= _positionY * ReturnSpeed;

        _velocityX *= Damping;
        _velocityY *= Damping;

        _positionX += _velocityX * deltaTime;
        _positionY += _velocityY * deltaTime;

        // Clamp position
        if (_positionX > MaxDisplacement) _positionX = MaxDisplacement;
        if (_positionX < -MaxDisplacement) _positionX = -MaxDisplacement;
        if (_positionY > MaxDisplacement) _positionY = MaxDisplacement;
        if (_positionY < -MaxDisplacement) _positionY = -MaxDisplacement;

        // Continue to apply transformation with updated position
    } else {
        // Convert to rad/s
        float gyroX = gyroDegX * (PI / 180.0f);
        float gyroY = gyroDegY * (PI / 180.0f);

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
    }

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
        Serial.print(") GyroDeg: (");
        Serial.print(gyroDegX);
        Serial.print(", ");
        Serial.print(gyroDegY);
        Serial.println(")");
        _lastLoggedPositionMag = positionMagnitude;
    }

    // Directly apply the physics position to the transformation.
    // The physics simulation above already produces smooth, lag-based movement.
    // The previous SetDestin+Restart approach reset the animation to t=0 every frame,
    // which caused Eye::Update() to evaluate Current = Origin (old position) each time,
    // effectively freezing the eyes and preventing any visible inertia effect.
    _face.LeftEye.InertiaTransformation.Origin  = t;
    _face.LeftEye.InertiaTransformation.Destin  = t;
    _face.LeftEye.InertiaTransformation.Current = t;

    _face.RightEye.InertiaTransformation.Origin  = t;
    _face.RightEye.InertiaTransformation.Destin  = t;
    _face.RightEye.InertiaTransformation.Current = t;
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
