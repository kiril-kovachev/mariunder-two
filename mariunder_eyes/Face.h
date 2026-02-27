/***************************************************
 * Face.h - Main face controller managing both eyes
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef FACE_H
#define FACE_H

#include <U8g2lib.h>
#include "Eye.h"
#include "FaceExpression.h"
#include "FaceBehavior.h"
#include "BlinkAssistant.h"
#include "LookAssistant.h"
#include "InertiaAssistant.h"
#include "AsyncTimer.h"
#include "Common.h"

// Forward declare global display instance
extern U8G2* u8g2;

// Main face controller
class Face {
public:
    Face(U8G2* display);

    uint16_t Width;
    uint16_t Height;
    uint16_t CenterX;
    uint16_t CenterY;
    uint16_t EyeSize;
    uint16_t EyeInterDistance = 8;

    Eye LeftEye;
    Eye RightEye;
    BlinkAssistant Blink;
    LookAssistant Look;
    InertiaAssistant Inertia;
    FaceBehavior Behavior;
    FaceExpression Expression;

    AsyncTimer _hitOnHeadRestoreTimer;

    // Overlay callback - set this to draw custom overlays on top of eyes
    void (*OverlayCallback)() = nullptr;

    // Flag to hide eyes (useful for rotate modes)
    bool HideEyes = false;

    void Update();
    void DoBlink();

    bool RandomBehavior = true;
    bool RandomLook = true;
    bool RandomBlink = true;

    void LookLeft();
    void LookRight();
    void LookFront();
    void LookTop();
    void LookBottom();
    void HitOnHead();  // Instant eyes move down effect (like hit on head)

protected:
    void Draw();
};

#endif // FACE_H
