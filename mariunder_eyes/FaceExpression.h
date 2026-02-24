/***************************************************
 * FaceExpression.h - Facial expression management
 * Based on esp32-eyes by Luis Llamas and Alastair Aitchison
 ****************************************************/

#ifndef FACE_EXPRESSION_H
#define FACE_EXPRESSION_H

#include "FaceEmotions.hpp"
#include "EyePresets.h"

// Forward declaration
class Face;

// Manages facial expressions and emotion transitions
class FaceExpression {
public:
    FaceExpression(Face& face);

    void SetTransitionTime(uint16_t milliseconds);
    void ClearVariations();

    // Emotion setters
    void GoTo_Normal();
    void GoTo_Happy();
    void GoTo_Angry();
    void GoTo_Sad();
    void GoTo_Surprised();
    void GoTo_Sleepy();
    void GoTo_Scared();
    void GoTo_Furious();
    void GoTo_Excited();
    void GoTo_Disappointed();
    void GoTo_Confused();
    void GoTo_Curious();
    void GoTo_Bored();
    void GoTo_Worried();
    void GoTo_Annoyed();
    void GoTo_Suspicious();
    void GoTo_Skeptical();
    void GoTo_Frustrated();

protected:
    Face& _face;
};

#endif // FACE_EXPRESSION_H
