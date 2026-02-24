/***************************************************
 * EyeBlink.h - Eye blinking transformation
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef _EYEBLINK_h
#define _EYEBLINK_h

#include <Arduino.h>
#include "Animations.h"
#include "EyeConfig.h"

class EyeBlink {
public:
    EyeBlink();

    EyeConfig* Input;
    EyeConfig Output;

    TrapeziumAnimation Animation;

    int32_t BlinkWidth = 60;
    int32_t BlinkHeight = 2;

    void Update();
    void Apply(float t);
};

#endif
