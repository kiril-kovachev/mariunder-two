/***************************************************
 * Face.cpp
 ****************************************************/

#include "Face.h"

// Global display pointer
U8G2* u8g2 = nullptr;

Face::Face(U8G2* display)
    : LeftEye(*this), RightEye(*this), Blink(*this), Look(*this), Inertia(*this), Behavior(), Expression(*this),
      _hitOnHeadRestoreTimer(100) {

    // Set global display reference
    u8g2 = display;

    Width = DISPLAY_WIDTH;
    Height = DISPLAY_HEIGHT;
    EyeSize = 32;

    CenterX = Width / 2;
    CenterY = Height / 2;

    LeftEye.IsMirrored = true;
}

void Face::LookFront() {
    Look.LookAt(0.0, 0.0);
}

void Face::LookRight() {
    Look.LookAt(-1.0, 0.0);
}

void Face::LookLeft() {
    Look.LookAt(1.0, 0.0);
}

void Face::LookTop() {
    Look.LookAt(0.0, 1.0);
}

void Face::LookBottom() {
    Look.LookAt(0.0, -1.0);
}

void Face::HitOnHead() {
    // Instant eyes move down effect (like hit on head)
    // Move eyes down instantly, then bounce back up
    
    // First: move down instantly (very short animation for instant effect)
    Transformation tDown;
    tDown.MoveX = 0.0f;
    tDown.MoveY = -10.0f;  // Move down 10 pixels (negative Y = down)
    tDown.ScaleX = 1.0f;
    tDown.ScaleY = 1.0f;

    // Set very short animation duration for near-instant movement
    LeftEye.Transformation.Animation.Interval = 30;  // 30ms for instant down
    RightEye.Transformation.Animation.Interval = 30;

    LeftEye.Transformation.SetDestin(tDown);
    LeftEye.Transformation.Animation.Restart();

    RightEye.Transformation.SetDestin(tDown);
    RightEye.Transformation.Animation.Restart();

    // Start timer to restore eyes after down animation completes
    _hitOnHeadRestoreTimer.setInterval(100);  // 100ms total (30ms down + 70ms buffer)
    _hitOnHeadRestoreTimer.restart();
}

void Face::Update() {
    if(RandomLook) Look.Update();
    if(RandomBlink) Blink.Update();
    Inertia.Update();  // Always update inertia (can be disabled via Inertia.Enabled)
    
    // Restore eyes after hit-on-head effect
    if (_hitOnHeadRestoreTimer.isExpired() && _hitOnHeadRestoreTimer.isRunning()) {
        // Restore animation interval to default
        LeftEye.Transformation.Animation.Interval = 200;
        RightEye.Transformation.Animation.Interval = 200;
        
        // Restore eyes to center (LookAssistant will handle this naturally, but we can force it)
        Look.LookAt(0.0, 0.0);
        
        _hitOnHeadRestoreTimer.stop();
    }
    
    Draw();
}

void Face::Draw() {
    // Clear the display
    u8g2->clearBuffer();

    // Draw eyes only if not hidden
    if (!HideEyes) {
        // Draw left eye
        LeftEye.CenterX = CenterX - EyeSize / 2 - EyeInterDistance;
        LeftEye.CenterY = CenterY;
        LeftEye.Draw();

        // Draw right eye
        RightEye.CenterX = CenterX + EyeSize / 2 + EyeInterDistance;
        RightEye.CenterY = CenterY;
        RightEye.Draw();
    }

    // Draw overlay if callback is set
    if (OverlayCallback != nullptr) {
        OverlayCallback();
    }

    // Transfer the redrawn buffer to the display
    u8g2->sendBuffer();
}

void Face::DoBlink() {
    // Placeholder implementation for blinking behavior
    Serial.println("Blinking");
    // Add actual blinking logic here, e.g., updating eye states
}
