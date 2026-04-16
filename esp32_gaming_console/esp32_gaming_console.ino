/*
 ============================================================
                DIY HANDHELD GAME CONSOLE (ESP32)
 ============================================================
   Target: ESP32 DevKit V1 (30-pin) or compatible ESP32-WROOM-32
   board in Arduino IDE ("ESP32 Dev Module").

   Hardware:
     - 128x64 I2C OLED (SSD1306, 3.3V)  SDA = GPIO 21, SCL = GPIO 22
     - Passive piezo buzzer              GPIO 25 (series resistor ~100Ω)
     - BTN_UP                            GPIO 17
     - BTN_DOWN                          GPIO 16
     - BTN_LEFT                          GPIO 4
     - BTN_RIGHT (flap / menu select)    GPIO 5

   Each button: one side to GPIO, other side to GND (active LOW,
   internal pull-ups enabled). See README.md for full wiring.

   Libraries: U8g2 (olikraus), Espressif ESP32 board package.
 ============================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#if !defined(ARDUINO_ARCH_ESP32)
#error This sketch is built for ESP32. In Arduino IDE choose Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module.
#endif

#include <esp_random.h>

// ── Display (hardware I2C; explicit pins for ESP32) ─────────
#define OLED_SDA 21
#define OLED_SCL 22

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
                                         /* clock=*/OLED_SCL, /* data=*/OLED_SDA);

#define SCREEN_W 128
#define SCREEN_H 64

// ── Buttons (avoid input-only pins 34–39; no internal pull-up) ─
#define BTN_UP 17
#define BTN_DOWN 16
#define BTN_LEFT 4
#define BTN_RIGHT 5

// ── Buzzer (PWM / LEDC-capable GPIO; not usable as ADC) ────
#define BUZZER_PIN 25

// ═══════════════════════════════════════════════════════════
//  UTILITY HELPERS
// ═══════════════════════════════════════════════════════════

bool btnPressed(uint8_t pin) {
  static uint32_t lastTime[4] = {0, 0, 0, 0};
  static bool lastSt[4] = {true, true, true, true};
  uint8_t idx;
  if (pin == BTN_UP)
    idx = 0;
  else if (pin == BTN_DOWN)
    idx = 1;
  else if (pin == BTN_LEFT)
    idx = 2;
  else if (pin == BTN_RIGHT)
    idx = 3;
  else
    return false;

  bool cur = (digitalRead(pin) == LOW);
  bool edge = cur && !lastSt[idx] && (millis() - lastTime[idx] > 40);
  if (cur != lastSt[idx])
    lastTime[idx] = millis();
  lastSt[idx] = cur;
  return edge;
}

bool btnHeld(uint8_t pin) { return digitalRead(pin) == LOW; }

void beep(uint16_t freq, uint16_t ms) {
  if (freq == 0 || ms == 0)
    return;
  // ESP32 tone() is LEDC-based; duration argument matches AVR-style usage.
  tone(BUZZER_PIN, freq, ms);
}

void waitRelease() {
  delay(30);
  while (btnHeld(BTN_UP) || btnHeld(BTN_DOWN) || btnHeld(BTN_LEFT) ||
         btnHeld(BTN_RIGHT))
    delay(10);
}

void centreStr(const char *s, uint8_t y) {
  uint8_t w = u8g2.getStrWidth(s);
  u8g2.drawStr((SCREEN_W - w) / 2, y, s);
}

// ── Music Helpers ──
void playStartMusic() {
  // ~2.5 second Upbeat Jingle
  beep(523, 150);
  delay(200); // C5
  beep(659, 150);
  delay(200); // E5
  beep(784, 150);
  delay(200); // G5
  beep(523, 150);
  delay(200); // C5
  beep(784, 150);
  delay(200); // G5
  beep(1046, 300);
  delay(400); // C6
  beep(880, 150);
  delay(200);      // A5
  beep(1046, 500); // C6
}

void playGameOverMusic() {
  // ~2.5 second Descending Theme
  beep(392, 250);
  delay(300); // G4
  beep(349, 250);
  delay(300); // F4
  beep(329, 250);
  delay(300); // E4
  beep(261, 500);
  delay(600);     // C4
  beep(196, 700); // G3
}

// ═══════════════════════════════════════════════════════════
//  GAME HEADERS
// ═══════════════════════════════════════════════════════════
#include "Asteroids.h"
#include "Breakout.h"
#include "Dino.h"
#include "FlappyBird.h"
#include "MazeRunner.h"
#include "Pacman.h"
#include "Pong.h"
#include "Snake.h"
#include "SpaceInvaders.h"
#include "Tetris.h"

// ═══════════════════════════════════════════════════════════
//  SPLASH + MENU
// ═══════════════════════════════════════════════════════════

void showSplash() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("GAME CONSOLE", 28);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Press any button...", 48);
  u8g2.sendBuffer();
  playStartMusic();
  while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
         !btnHeld(BTN_RIGHT))
    delay(15);
  waitRelease();
}

void gameOver(uint16_t score) {
  char buf[20];
  snprintf(buf, sizeof(buf), "Score: %u", score);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("GAME OVER", 28);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr(buf, 44);
  centreStr("Press any button", 58);
  u8g2.sendBuffer();
  playGameOverMusic();
  delay(400);
  waitRelease();
  while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
         !btnHeld(BTN_RIGHT))
    delay(15);
  waitRelease();
}

#define GAME_COUNT 10

int menuSelect() {
  const char *names[GAME_COUNT] = {
      "1. Asteroids",      "2. Breakout", "3. Dino Run", "4. Flappy Bird",
      "5. Maze Runner",    "6. Pacman",   "7. Pong",     "8. Snake",
      "9. Space Invaders", "10. Tetris"};
  int sel = 0;
  int top = 0;
  const int VISIBLE = 4;

  while (true) {
    if (sel < top)
      top = sel;
    if (sel >= top + VISIBLE)
      top = sel - VISIBLE + 1;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("** GAME CONSOLE **", 9);
    u8g2.setDrawColor(1);

    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= GAME_COUNT)
        break;
      int y = 13 + i * 13;
      if (idx == sel) {
        u8g2.drawRBox(0, y, SCREEN_W - 10, 12, 2);
        u8g2.setDrawColor(0);
        u8g2.drawStr(4, y + 9, names[idx]);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(4, y + 9, names[idx]);
      }
    }

    u8g2.drawFrame(SCREEN_W - 7, 12, 6, 52);
    int thumbH = max(6, 52 / GAME_COUNT * VISIBLE);
    int thumbY = 12 + (sel * (52 - thumbH)) / (GAME_COUNT - 1);
    u8g2.drawBox(SCREEN_W - 6, thumbY, 4, thumbH);

    if (top > 0)
      u8g2.drawStr(SCREEN_W - 8, 13, "^");
    if (top + VISIBLE < GAME_COUNT)
      u8g2.drawStr(SCREEN_W - 8, 62, "v");

    u8g2.sendBuffer();
    delay(100);
    if (btnPressed(BTN_UP)) {
      sel = (sel + GAME_COUNT - 1) % GAME_COUNT;
      beep(800, 25);
    }
    if (btnPressed(BTN_DOWN)) {
      sel = (sel + 1) % GAME_COUNT;
      beep(800, 25);
    }
    if (btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT)) {
      beep(1200, 60);
      waitRelease();
      return sel;
    }
  }
}

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);

  randomSeed(esp_random());

  u8g2.begin();
  u8g2.setContrast(200);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  showSplash();
}

void loop() {
  int sel = menuSelect();
  switch (sel) {
  case 0:
    game_asteroids();
    break;
  case 1:
    game_breakout();
    break;
  case 2:
    game_dino();
    break;
  case 3:
    game_flappy();
    break;
  case 4:
    game_maze();
    break;
  case 5:
    game_pacman();
    break;
  case 6:
    game_pong();
    break;
  case 7:
    game_snake();
    break;
  case 8:
    game_spaceinvaders();
    break;
  case 9:
    game_tetris();
    break;
  }
}
