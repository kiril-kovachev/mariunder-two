/***************************************************
 * EyePresets.h - Predefined eye configurations for emotions
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef EYE_PRESETS_H
#define EYE_PRESETS_H

#include "EyeConfig.h"
#include "FaceEmotions.hpp"

// Predefined eye configurations for different emotions
static const EyeConfig Preset_Normal = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 40,
    .Width = 40,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 8,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Happy = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 10,
    .Width = 40,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 10,
    .Radius_Bottom = 0,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 5,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Sad = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 15,
    .Width = 40,
    .Slope_Top = -0.5,
    .Slope_Bottom = 0,
    .Radius_Top = 1,
    .Radius_Bottom = 10,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Angry = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 25,
    .Width = 40,
    .Slope_Top = -0.5,
    .Slope_Bottom = 0,
    .Radius_Top = 3,
    .Radius_Bottom = 1,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Annoyed = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 12,
    .Width = 40,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 0,
    .Radius_Bottom = 10,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Surprised = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 35,
    .Width = 42,
    .Slope_Top = -0.1,
    .Slope_Bottom = 0.1,
    .Radius_Top = 12,
    .Radius_Bottom = 12,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Sleepy = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 5,
    .Width = 40,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 3,
    .Radius_Bottom = 3,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Furious = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 30,
    .Width = 40,
    .Slope_Top = -0.8,
    .Slope_Bottom = 0,
    .Radius_Top = 3,
    .Radius_Bottom = 1,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Scared = {
    .OffsetX = -3,
    .OffsetY = 0,
    .Height = 40,
    .Width = 40,
    .Slope_Top = -0.1,
    .Slope_Bottom = 0,
    .Radius_Top = 12,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Excited = {
    .OffsetX = 2,
    .OffsetY = 0,
    .Height = 35,
    .Width = 45,
    .Slope_Top = -0.1,
    .Slope_Bottom = 0.1,
    .Radius_Top = 12,
    .Radius_Bottom = 12,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Worried = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 25,
    .Width = 40,
    .Slope_Top = -0.1,
    .Slope_Bottom = 0,
    .Radius_Top = 3,
    .Radius_Bottom = 10,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Suspicious = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 10,
    .Width = 40,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 0,
    .Radius_Bottom = 10,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Squint = {
    .OffsetX = -10,
    .OffsetY = -3,
    .Height = 35,
    .Width = 35,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 8,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Disappointed = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 20,
    .Width = 40,
    .Slope_Top = -0.6,
    .Slope_Bottom = 0,
    .Radius_Top = 1,
    .Radius_Bottom = 10,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Confused = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 30,
    .Width = 40,
    .Slope_Top = 0.3,
    .Slope_Bottom = 0,
    .Radius_Top = 5,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Bored = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 20,
    .Width = 40,
    .Slope_Top = 0,
    .Slope_Bottom = 0,
    .Radius_Top = 5,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Curious = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 35,
    .Width = 40,
    .Slope_Top = 0.2,
    .Slope_Bottom = 0,
    .Radius_Top = 8,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Skeptical = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 15,
    .Width = 40,
    .Slope_Top = -0.2,
    .Slope_Bottom = 0,
    .Radius_Top = 2,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

static const EyeConfig Preset_Frustrated = {
    .OffsetX = 0,
    .OffsetY = 0,
    .Height = 22,
    .Width = 40,
    .Slope_Top = -0.6,
    .Slope_Bottom = 0,
    .Radius_Top = 2,
    .Radius_Bottom = 8,
    .Inverse_Radius_Top = 0,
    .Inverse_Radius_Bottom = 0,
    .Inverse_Offset_Top = 0,
    .Inverse_Offset_Bottom = 0
};

// Mapping function for backwards compatibility
class EyePresets {
public:
    static EyeConfig getPreset(FaceEmotions emotion) {
        switch(emotion) {
            case NORMAL: return Preset_Normal;
            case HAPPY: return Preset_Happy;
            case ANGRY: return Preset_Angry;
            case SAD: return Preset_Sad;
            case SURPRISED: return Preset_Surprised;
            case SLEEPY: return Preset_Sleepy;
            case SCARED: return Preset_Scared;
            case FURIOUS: return Preset_Furious;
            case EXCITED: return Preset_Excited;
            case DISAPPOINTED: return Preset_Disappointed;
            case CONFUSED: return Preset_Confused;
            case CURIOUS: return Preset_Curious;
            case BORED: return Preset_Bored;
            case WORRIED: return Preset_Worried;
            case ANNOYED: return Preset_Annoyed;
            case SUSPICIOUS: return Preset_Suspicious;
            case SKEPTICAL: return Preset_Skeptical;
            case FRUSTRATED: return Preset_Frustrated;
            default: return Preset_Normal;
        }
    }
};

#endif // EYE_PRESETS_H
