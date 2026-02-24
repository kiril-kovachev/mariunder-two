/***************************************************
 * EyeTransition.h - Smooth transitions between eye presets
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef _EYETRANSITION_h
#define _EYETRANSITION_h

#include <Arduino.h>
#include "Animations.h"
#include "EyeConfig.h"

class EyeTransition {
public:
    EyeTransition();

    EyeConfig* Origin;
    EyeConfig Destin;

    RampAnimation Animation;

    void Update();
    void Apply(float t);
};

#endif
