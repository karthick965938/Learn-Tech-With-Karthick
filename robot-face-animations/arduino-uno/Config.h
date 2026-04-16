#ifndef CONFIG_H
#define CONFIG_H

// --- DISPLAY CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// --- OFFSET TO AVOID YELLOW TOP (for Dual Color OLEDs) ---
#define Y_OFFSET 12 

// --- EYE PARAMETERS ---
#define REF_EYE_HEIGHT 30
#define REF_EYE_WIDTH 30
#define REF_SPACE_BETWEEN_EYES 12
#define REF_CORNER_RADIUS 8

// --- MOUTH PARAMETERS ---
#define REF_MOUTH_WIDTH 40
#define REF_MOUTH_HEIGHT 10
#define MOUTH_Y_OFFSET 20 // Distance below eyes

// --- SERIAL CONFIG ---
#define BAUD_RATE 115200

#endif // CONFIG_H
