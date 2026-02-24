#ifndef FACE_EMOTIONS_HPP
#define FACE_EMOTIONS_HPP

// Enum defining all available facial emotions
// Based on esp32-eyes project
enum FaceEmotions {
    NORMAL = 0,
    HAPPY,
    ANGRY,
    SAD,
    SURPRISED,
    SLEEPY,
    SCARED,
    FURIOUS,
    EXCITED,
    DISAPPOINTED,
    CONFUSED,
    CURIOUS,
    BORED,
    WORRIED,
    ANNOYED,
    SUSPICIOUS,
    SKEPTICAL,
    FRUSTRATED,

    // Total count
    EMOTION_COUNT
};

// Helper function to get emotion name
inline const char* getEmotionName(FaceEmotions emotion) {
    switch(emotion) {
        case NORMAL: return "Normal";
        case HAPPY: return "Happy";
        case ANGRY: return "Angry";
        case SAD: return "Sad";
        case SURPRISED: return "Surprised";
        case SLEEPY: return "Sleepy";
        case SCARED: return "Scared";
        case FURIOUS: return "Furious";
        case EXCITED: return "Excited";
        case DISAPPOINTED: return "Disappointed";
        case CONFUSED: return "Confused";
        case CURIOUS: return "Curious";
        case BORED: return "Bored";
        case WORRIED: return "Worried";
        case ANNOYED: return "Annoyed";
        case SUSPICIOUS: return "Suspicious";
        case SKEPTICAL: return "Skeptical";
        case FRUSTRATED: return "Frustrated";
        default: return "Unknown";
    }
}

#endif // FACE_EMOTIONS_HPP
