/***************************************************
 * EyeConfig.h - Eye configuration structure
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef EYE_CONFIG_H
#define EYE_CONFIG_H

#include <Arduino.h>

// Eye configuration - defines the shape of the sclera (white part)
struct EyeConfig {
    // Position offset from center
    int16_t OffsetX = 0;
    int16_t OffsetY = 0;

    // Eye dimensions
    int16_t Height = 40;
    int16_t Width = 40;

    // Slope of top and bottom edges (creates slanted look)
    float Slope_Top = 0;
    float Slope_Bottom = 0;

    // Corner rounding radius
    int16_t Radius_Top = 8;
    int16_t Radius_Bottom = 8;

    // Inverse radius for special eye shapes (cutouts)
    int16_t Inverse_Radius_Top = 0;
    int16_t Inverse_Radius_Bottom = 0;
    int16_t Inverse_Offset_Top = 0;
    int16_t Inverse_Offset_Bottom = 0;
};

#endif // EYE_CONFIG_H
