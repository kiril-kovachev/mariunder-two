#ifndef FACE_BEHAVIOR_H
#define FACE_BEHAVIOR_H

#include "FaceEmotions.hpp"
#include "AsyncTimer.h"
#include "Common.h"

// Manages emotion weighting and random behavior
class FaceBehavior {
public:
    bool RandomBehaviour;

    FaceBehavior() :
        RandomBehaviour(false)
    {}

    // Get a random emotion based on weights
    FaceEmotions GetRandomEmotion() {
        // Weighted random selection
        // Normal emotions are more common, extreme ones are rare
        int roll = random(0, 100);

        if (roll < 30) return NORMAL;
        if (roll < 45) return HAPPY;
        if (roll < 55) return CURIOUS;
        if (roll < 65) return SURPRISED;
        if (roll < 70) return BORED;
        if (roll < 75) return CONFUSED;
        if (roll < 80) return SAD;
        if (roll < 85) return WORRIED;
        if (roll < 88) return ANNOYED;
        if (roll < 91) return DISAPPOINTED;
        if (roll < 93) return ANGRY;
        if (roll < 95) return SKEPTICAL;
        if (roll < 96) return SUSPICIOUS;
        if (roll < 97) return EXCITED;
        if (roll < 98) return FRUSTRATED;
        if (roll < 99) return SCARED;
        return FURIOUS;
    }

    // Get random emotion excluding certain ones
    FaceEmotions GetRandomEmotionExcluding(FaceEmotions* exclude, int excludeCount) {
        FaceEmotions emotion;
        bool valid = false;

        // Try up to 20 times to get a valid emotion
        for (int attempt = 0; attempt < 20; attempt++) {
            emotion = GetRandomEmotion();
            valid = true;

            // Check if this emotion is in the exclude list
            for (int i = 0; i < excludeCount; i++) {
                if (emotion == exclude[i]) {
                    valid = false;
                    break;
                }
            }

            if (valid) break;
        }

        return emotion;
    }
};

#endif // FACE_BEHAVIOR_H
