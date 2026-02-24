/***************************************************
 * LookAssistant.h - Gaze direction management
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef LOOK_ASSISTANT_H
#define LOOK_ASSISTANT_H

#include "AsyncTimer.h"
#include "EyeTransformation.h"

// Forward declaration
class Face;

// Manages gaze direction changes
class LookAssistant {
public:
    LookAssistant(Face& face);

    AsyncTimer Timer;

    void Update();
    void LookAt(float x, float y);

protected:
    Face& _face;
};

#endif // LOOK_ASSISTANT_H
