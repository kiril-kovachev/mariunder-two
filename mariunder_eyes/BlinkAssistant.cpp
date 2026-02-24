/***************************************************
 * BlinkAssistant.cpp
 ****************************************************/

#include "BlinkAssistant.h"
#include "Face.h"

BlinkAssistant::BlinkAssistant(Face& face) : _face(face), Timer(3500) {
    Timer.start();
}

void BlinkAssistant::Update() {
    if (Timer.isExpired()) {
        Blink();
        Timer.restart();
    }
}

void BlinkAssistant::Blink() {
    _face.LeftEye.BlinkTransformation.Animation.Restart();
    _face.RightEye.BlinkTransformation.Animation.Restart();
    Timer.restart();
}
