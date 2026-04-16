#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "Eye.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
EyeAnimation eyes(display);

unsigned long lastAction = 0;
int currentStep = 0;

void setup() {
    Serial.begin(BAUD_RATE);
    
    // Initialize I2C
    Wire.begin();
    
    // Initialize display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    
    display.clearDisplay();
    display.display();
    
    eyes.begin();
    Serial.println(F("Eye Animation Ready (Arduino Uno)"));
}

void loop() {
    eyes.update();
    eyes.draw();

    unsigned long now = millis();
    if (now - lastAction > 3000) {
        lastAction = now;
        
        switch(currentStep) {
            case 0:
                Serial.println("Action: Blink");
                eyes.blink();
                break;
            case 1:
                Serial.println("Action: Look Left");
                eyes.lookAt(-1.0f, 0.0f);
                break;
            case 2:
                Serial.println("Action: Look Right");
                eyes.lookAt(1.0f, 0.0f);
                break;
            case 3:
                Serial.println("Action: Look Up");
                eyes.lookAt(0.0f, -1.0f);
                break;
            case 4:
                Serial.println("Action: Emotion HAPPY");
                eyes.setEmotion(EyeAnimation::HAPPY);
                break;
            case 5:
                Serial.println("Action: Emotion SURPRISED");
                eyes.setEmotion(EyeAnimation::SURPRISED);
                break;
            case 6:
                Serial.println("Action: Emotion SLEEPY");
                eyes.setEmotion(EyeAnimation::SLEEPY);
                break;
            case 7:
                Serial.println("Action: Reset");
                eyes.reset();
                break;
        }
        
        currentStep = (currentStep + 1) % 8;
    }

    // Small delay to prevent display jitter and allow some CPU time
    delay(10);
}
