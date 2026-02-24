/***************************************************
 * LookAssistant.cpp
 ****************************************************/

#include "LookAssistant.h"
#include "Face.h"

LookAssistant::LookAssistant(Face& face) : _face(face), Timer(4000) {
    Timer.start();
}

void LookAssistant::Update() {
    if (Timer.isExpired()) {
        // Pick random look direction
        float x = random(-80, 80) / 10.0f;
        float y = random(-50, 50) / 10.0f;
        LookAt(x, y);
        Timer.restart();
    }
}

void LookAssistant::LookAt(float x, float y) {
    Transformation t;
    t.MoveX = x;
    t.MoveY = y;
    t.ScaleX = 1.0f;
    t.ScaleY = 1.0f;

    _face.LeftEye.Transformation.SetDestin(t);
    _face.LeftEye.Transformation.Animation.Restart();

    _face.RightEye.Transformation.SetDestin(t);
    _face.RightEye.Transformation.Animation.Restart();
}
