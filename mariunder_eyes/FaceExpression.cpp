/***************************************************
 * FaceExpression.cpp
 ****************************************************/

#include "FaceExpression.h"
#include "Face.h"

FaceExpression::FaceExpression(Face& face) : _face(face) {}

void FaceExpression::SetTransitionTime(uint16_t milliseconds) {
    _face.RightEye.Transition.Animation.Interval = milliseconds;
    _face.LeftEye.Transition.Animation.Interval = milliseconds;
}

void FaceExpression::ClearVariations() {
    _face.RightEye.Variation1.Clear();
    _face.RightEye.Variation2.Clear();
    _face.LeftEye.Variation1.Clear();
    _face.LeftEye.Variation2.Clear();
    _face.RightEye.Variation1.Animation.Restart();
    _face.LeftEye.Variation1.Animation.Restart();
    // Reset Variation2 animation to default period so a prior shake's 80ms buzz interval
    // doesn't persist into the next expression (e.g. normal eye width pulse would buzz at 80ms)
    _face.RightEye.Variation2.Animation.SetTriangle(800, 0);
    _face.RightEye.Variation2.Animation.Restart();
    _face.LeftEye.Variation2.Animation.SetTriangle(800, 0);
    _face.LeftEye.Variation2.Animation.Restart();
}

void FaceExpression::GoTo_Normal() {
    ClearVariations();

    _face.RightEye.Variation1.Values.Height = 3;
    _face.RightEye.Variation2.Values.Width = 1;
    _face.LeftEye.Variation1.Values.Height = 2;
    _face.LeftEye.Variation2.Values.Width = 2;
    _face.RightEye.Variation1.Animation.SetTriangle(1000, 0);
    _face.LeftEye.Variation1.Animation.SetTriangle(1000, 0);

    _face.RightEye.TransitionTo(Preset_Normal);
    _face.LeftEye.TransitionTo(Preset_Normal);
}

void FaceExpression::GoTo_Happy() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Happy);
    _face.LeftEye.TransitionTo(Preset_Happy);
}

void FaceExpression::GoTo_Angry() {
    ClearVariations();
    
    _face.RightEye.Variation1.Values.OffsetY = 2;
    _face.LeftEye.Variation1.Values.OffsetY = 2;
    _face.RightEye.Variation1.Animation.SetTriangle(300, 0);
    _face.LeftEye.Variation1.Animation.SetTriangle(300, 0);

    _face.RightEye.TransitionTo(Preset_Angry);
    _face.LeftEye.TransitionTo(Preset_Angry);
}

void FaceExpression::GoTo_Sad() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Sad);
    _face.LeftEye.TransitionTo(Preset_Sad);
}

void FaceExpression::GoTo_Surprised() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Surprised);
    _face.LeftEye.TransitionTo(Preset_Surprised);
}

void FaceExpression::GoTo_Sleepy() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Sleepy);
    _face.LeftEye.TransitionTo(Preset_Sleepy);
}

void FaceExpression::GoTo_Scared() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Scared);
    _face.LeftEye.TransitionTo(Preset_Scared);
}

void FaceExpression::GoTo_Furious() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Furious);
    _face.LeftEye.TransitionTo(Preset_Furious);
}

void FaceExpression::GoTo_Excited() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Excited);
    _face.LeftEye.TransitionTo(Preset_Excited);
}

void FaceExpression::GoTo_Disappointed() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Disappointed);
    _face.LeftEye.TransitionTo(Preset_Disappointed);
}

void FaceExpression::GoTo_Confused() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Confused);
    _face.LeftEye.TransitionTo(Preset_Confused);
}

void FaceExpression::GoTo_Curious() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Curious);
    _face.LeftEye.TransitionTo(Preset_Curious);
}

void FaceExpression::GoTo_Bored() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Bored);
    _face.LeftEye.TransitionTo(Preset_Bored);
}

void FaceExpression::GoTo_Worried() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Worried);
    _face.LeftEye.TransitionTo(Preset_Worried);
}

void FaceExpression::GoTo_Annoyed() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Annoyed);
    _face.LeftEye.TransitionTo(Preset_Annoyed);
}

void FaceExpression::GoTo_Suspicious() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Suspicious);
    _face.LeftEye.TransitionTo(Preset_Suspicious);
}

void FaceExpression::GoTo_Skeptical() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Skeptical);
    _face.LeftEye.TransitionTo(Preset_Skeptical);
}

void FaceExpression::GoTo_Frustrated() {
    ClearVariations();

    _face.RightEye.TransitionTo(Preset_Frustrated);
    _face.LeftEye.TransitionTo(Preset_Frustrated);
}
