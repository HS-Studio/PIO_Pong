// main.h
#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_GFX.h>

// Konfiguration für den ST7789-Bildschirm
#define SCREEN_W  240
#define SCREEN_H  280
#define TFT_CS  6
#define TFT_DC  7
#define TFT_RST 8

#define PIN_JOY_X A0
#define PIN_JOY_Y A1

#define PADDLEMAXSPEED 8
#define BALLMAXSPEED 8

bool cpu = false; // true = CPU vs. CPU, false = Player vs. CPU

uint16_t joyXMin, joyXMax, joyXCenter, joyYMin, joyYMax, joyYCenter;

int16_t joy_x, joy_y;

// Canvas-Größe
uint16_t screen_w = 240;
uint16_t screen_h = 280;

uint16_t canv_w = screen_w;
uint16_t canv_h = screen_h;

extern Adafruit_ST7789 tft;
extern GFXcanvas16 *Canvas;

uint8_t p1_points, p2_points;

//x, y, Breite, Höhe, Geschwindigkeit x, Geschwindigkeit y, Farbe
struct g_object
{
    int16_t pos_x;
    int16_t pos_y;
    uint8_t width;
    uint8_t height;
    int8_t speed_x;
    int8_t speed_y;
    uint16_t color;
};
//(int16_t)x, (int16_t)y, (uint8_t)Breite, (uint8_t)Höhe, (int8_t)Geschwindigkeit x, (int8_t)Geschwindigkeit y, (uint16_t)Farbe
g_object paddle1, paddle2, ball;

uint8_t paddle_w = 80;
uint8_t paddle_h = 10;

#define PADDLEMAXSPEED 8
#define BALLMAXSPEED 8

// Functions
void drawGame();
void resetRound();
void resetGame();
float customMap(long x, long in_min, long in_center, long in_max, long out_min, long out_max);

#endif
//EOF