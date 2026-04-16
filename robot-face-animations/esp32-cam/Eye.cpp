#include "Eye.h"

EyeAnimation::EyeAnimation(Adafruit_SSD1306& display) : _display(display) {
    _animationSpeed = 0.2f;
    _lastUpdate = millis();
}

void EyeAnimation::begin() {
    reset();
}

void EyeAnimation::reset() {
    float centerX = SCREEN_WIDTH / 2.0f;
    float centerY = (SCREEN_HEIGHT / 2.0f) + (Y_OFFSET / 2.0f);
    float offset = (REF_EYE_WIDTH + REF_SPACE_BETWEEN_EYES) / 2.0f;

    _targetLeft = {centerX - offset, centerY - 5, REF_EYE_WIDTH, REF_EYE_HEIGHT, REF_CORNER_RADIUS};
    _targetRight = {centerX + offset, centerY - 5, REF_EYE_WIDTH, REF_EYE_HEIGHT, REF_CORNER_RADIUS};
    
    _targetMouth = {centerX, centerY + MOUTH_Y_OFFSET, REF_MOUTH_WIDTH, 2, 0.0f}; // Neutral: flat
    
    _leftEye = _targetLeft;
    _rightEye = _targetRight;
    _mouth = _targetMouth;
}

void EyeAnimation::update() {
    unsigned long now = millis();
    _lastUpdate = now;
    _applyLerp(_animationSpeed);
}

void EyeAnimation::_applyLerp(float t) {
    // Eye Lerp
    _leftEye.x = _lerp(_leftEye.x, _targetLeft.x, t);
    _leftEye.y = _lerp(_leftEye.y, _targetLeft.y, t);
    _leftEye.width = _lerp(_leftEye.width, _targetLeft.width, t);
    _leftEye.height = _lerp(_leftEye.height, _targetLeft.height, t);
    _leftEye.radius = _lerp(_leftEye.radius, _targetLeft.radius, t);

    _rightEye.x = _lerp(_rightEye.x, _targetRight.x, t);
    _rightEye.y = _lerp(_rightEye.y, _targetRight.y, t);
    _rightEye.width = _lerp(_rightEye.width, _targetRight.width, t);
    _rightEye.height = _lerp(_rightEye.height, _targetRight.height, t);
    _rightEye.radius = _lerp(_rightEye.radius, _targetRight.radius, t);

    // Mouth Lerp
    _mouth.x = _lerp(_mouth.x, _targetMouth.x, t);
    _mouth.y = _lerp(_mouth.y, _targetMouth.y, t);
    _mouth.width = _lerp(_mouth.width, _targetMouth.width, t);
    _mouth.height = _lerp(_mouth.height, _targetMouth.height, t);
    _mouth.curve = _lerp(_mouth.curve, _targetMouth.curve, t);
}

float EyeAnimation::_lerp(float start, float end, float t) {
    return start + t * (end - start);
}

void EyeAnimation::draw() {
    _display.clearDisplay();
    _drawEye(_leftEye);
    _drawEye(_rightEye);
    _drawMouth(_mouth);
    _display.display();
}

void EyeAnimation::_drawEye(const EyeState& eye) {
    int x = (int)(eye.x - eye.width / 2.0f);
    int y = (int)(eye.y - eye.height / 2.0f);
    int r = _calculateSafeRadius(eye.width, eye.height, eye.radius);
    _display.fillRoundRect(x, y, (int)eye.width, (int)eye.height, r, SSD1306_WHITE);
}

void EyeAnimation::_drawMouth(const MouthState& mouth) {
    int x = (int)(mouth.x - mouth.width / 2.0f);
    int y = (int)mouth.y;
    int w = (int)mouth.width;
    int h = (int)mouth.height;

    if (h > 4) {
        // Draw as an oval/box for surprised
        _display.fillRoundRect(x, y - h/2, w, h, w/2, SSD1306_WHITE);
    } else {
        // Draw as a curve or line
        for (int i = 0; i < w; i++) {
            float progress = (float)i / (float)w;
            // Parabola: y = curve * (x - 0.5)^2 * intensity
            float curveOffset = mouth.curve * 10.0f * (0.25f - (progress - 0.5f) * (progress - 0.5f));
            _display.drawPixel(x + i, y - (int)curveOffset, SSD1306_WHITE);
            _display.drawPixel(x + i, y - (int)curveOffset + 1, SSD1306_WHITE); // Thicker line
        }
    }
}

int EyeAnimation::_calculateSafeRadius(float w, float h, float r) {
    if (w < 2 * r) r = w / 2;
    if (h < 2 * r) r = h / 2;
    if (r < 0) r = 0;
    return (int)r;
}

void EyeAnimation::blink(int durationMs) {
    float originalHeightL = _targetLeft.height;
    float originalHeightR = _targetRight.height;
    _targetLeft.height = 2;
    _targetRight.height = 2;
    for(int i=0; i<5; i++) { _applyLerp(0.6); draw(); delay(durationMs / 10); }
    _targetLeft.height = originalHeightL;
    _targetRight.height = originalHeightR;
    for(int i=0; i<5; i++) { _applyLerp(0.6); draw(); delay(durationMs / 10); }
}

void EyeAnimation::lookAt(float offsetX, float offsetY) {
    float maxMoveX = 12.0f;
    float maxMoveY = 8.0f;
    float centerX = SCREEN_WIDTH / 2.0f;
    float centerY = (SCREEN_HEIGHT / 2.0f) + (Y_OFFSET / 2.0f);
    float spacing = (REF_EYE_WIDTH + REF_SPACE_BETWEEN_EYES) / 2.0f;

    _targetLeft.x = centerX - spacing + (offsetX * maxMoveX);
    _targetRight.x = centerX + spacing + (offsetX * maxMoveX);
    _targetLeft.y = centerY - 5 + (offsetY * maxMoveY);
    _targetRight.y = centerY - 5 + (offsetY * maxMoveY);
    
    _targetMouth.x = centerX + (offsetX * 5.0f); // Mouth moves slightly with eyes
}

void EyeAnimation::setEmotion(int emotion) {
    reset();
    switch(emotion) {
        case HAPPY:
            _targetMouth.curve = 1.0f;
            _targetMouth.width = 45;
            _targetLeft.y -= 2;
            _targetRight.y -= 2;
            break;
        case SAD:
            _targetMouth.curve = -1.0f;
            _targetMouth.width = 35;
            _targetLeft.radius = 15;
            _targetRight.radius = 15;
            break;
        case ANGRY:
            _targetMouth.curve = -0.2f;
            _targetMouth.width = 30;
            _targetLeft.height = 20;
            _targetRight.height = 20;
            break;
        case SURPRISED:
            _targetMouth.height = 15;
            _targetMouth.width = 20;
            _targetLeft.width = 35;
            _targetRight.width = 35;
            break;
        case SLEEPY:
            _targetMouth.width = 15;
            _targetMouth.height = 2;
            _targetLeft.height = 5;
            _targetRight.height = 5;
            break;
    }
}
