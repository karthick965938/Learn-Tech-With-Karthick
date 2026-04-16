#ifndef EYE_H
#define EYE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

struct EyeState {
    float x;
    float y;
    float width;
    float height;
    float radius;
};

struct MouthState {
    float x;
    float y;
    float width;
    float height;
    float curve; // -1.0 (sad) to 1.0 (happy)
};

class EyeAnimation {
public:
    EyeAnimation(Adafruit_SSD1306& display);

    void begin();
    void update();
    void draw();

    // Animations
    void blink(int durationMs = 150);
    void lookAt(float offsetX, float offsetY); // -1.0 to 1.0
    void setEmotion(int emotion);
    void reset();

    enum Emotion {
        NEUTRAL,
        HAPPY,
        SAD,
        ANGRY,
        SURPRISED,
        SLEEPY
    };

private:
    Adafruit_SSD1306& _display;
    EyeState _leftEye;
    EyeState _rightEye;
    EyeState _targetLeft;
    EyeState _targetRight;
    
    MouthState _mouth;
    MouthState _targetMouth;

    float _lerp(float start, float end, float t);
    void _applyLerp(float t);
    int _calculateSafeRadius(float w, float h, float r);
    void _drawEye(const EyeState& eye);
    void _drawMouth(const MouthState& mouth);

    unsigned long _lastUpdate;
    float _animationSpeed; 
};

#endif // EYE_H
