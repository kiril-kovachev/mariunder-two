/***************************************************
 * EyeVariation.h - Small animated variations in eye shape
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef _EYEVARIATION_h
#define _EYEVARIATION_h

#include <Arduino.h>
#include "Animations.h"
#include "EyeConfig.h"

class EyeVariation {
public:
    EyeVariation();

    EyeConfig* Input;
    EyeConfig Output;

    TrapeziumPulseAnimation Animation;

    EyeConfig Values;
    
    void Clear();
    void Update();
    void Apply(float t);
};

#endif
