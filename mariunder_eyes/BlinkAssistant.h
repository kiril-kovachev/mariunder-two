/***************************************************
 * BlinkAssistant.h - Automatic blinking management
 * Based on esp32-eyes by LuisLlamas and Alastair Aitchison
 ****************************************************/

#ifndef BLINK_ASSISTANT_H
#define BLINK_ASSISTANT_H

#include "AsyncTimer.h"

// Forward declaration
class Face;

// Manages automatic blinking behavior
class BlinkAssistant {
public:
    BlinkAssistant(Face& face);

    AsyncTimer Timer;

    void Update();
    void Blink();

protected:
    Face& _face;
};

#endif // BLINK_ASSISTANT_H
