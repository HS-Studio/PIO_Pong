#include <Arduino.h>
#include "main.h"

// ST7789-Display initialisieren
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Canvas Buffer    // Canvas-Puffer
GFXcanvas16 *Canvas;

void setup() {
  // Serial Monitor initialisieren
  Serial.begin(115200);
  delay(2000);

  //Canvas initialisieren
  Canvas = (GFXcanvas16 *)malloc(sizeof(GFXcanvas16));
  if (!Canvas) {
    Serial.println("[ERROR] RAM-Allocation fehlgeschlagen!");
    while (1);
  }

  tft.init(SCREEN_W, SCREEN_H);
  tft.setSPISpeed(80000000);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);

  if (tft.getRotation() == 1 || tft.getRotation() == 3) {
    canv_w = screen_h;
    canv_h = screen_w;
  } else {
    canv_w = screen_w;
    canv_h = screen_h;
  }

  new (Canvas) GFXcanvas16(canv_w, canv_h);

  // Joystick initialisieren
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);

  Serial.println(analogRead(PIN_JOY_Y));
  if (analogRead(PIN_JOY_Y) > 7500){
    cpu = true;
  }

  joyXMin = analogRead(PIN_JOY_X);
  joyXMax = analogRead(PIN_JOY_X);
  joyXCenter = analogRead(PIN_JOY_X);

  // Game initialisieren   
  paddle1 = {(int16_t)(canv_w / 2 - 50), 0, paddle_w, paddle_h, 3, 0, 0x257f};
  paddle2 = {(int16_t)(canv_w / 2 - 50), (int16_t)(canv_h-paddle_h), paddle_w, paddle_h, 6, 0, 0x27f5};
  ball = {0, 0, 10, 10, 3, 6, 0xFFFF};
  resetGame();
}

void loop() {

  // Ball movement    // Ball Bewegung
  ball.pos_x += ball.speed_x;
  ball.pos_y += ball.speed_y;

  // Paddle movement    // Paddle Bewegung
  paddle1.pos_x += paddle1.speed_x;
  paddle2.pos_x += paddle2.speed_x;

  // Ball collision with walls    // Ball Kollision mit den Wänden
  if (ball.pos_x > canv_w - ball.width || ball.pos_x < 0) {
    ball.pos_x = constrain(ball.pos_x, 0, canv_w - ball.width);
    ball.speed_x = -ball.speed_x;
  }
  // Ball collision with top wall    // Ball Kollision mit der oberen Wand
  if (ball.pos_y < 0) {
    resetRound();
    p2_points++;
  }
  // Ball collision with bottom wall // Ball Kollision mit der unteren Wand
  if (ball.pos_y > canv_h - ball.height) {
    resetRound();
    p1_points++;
  }

  if (p1_points >= 10 || p2_points >= 10) {
    resetGame();
  }

  // Ball collision with paddles
  if (ball.pos_y < paddle1.pos_y + paddle1.height && ball.pos_x + ball.width > paddle1.pos_x && ball.pos_x < paddle1.pos_x + paddle1.width) {
    ball.pos_y = paddle1.pos_y + paddle1.height +1;
    ball.speed_y = -ball.speed_y;
    ball.speed_x += paddle1.speed_x / 2;
  }
  if (ball.pos_y + ball.height > paddle2.pos_y && ball.pos_x + ball.width > paddle2.pos_x && ball.pos_x < paddle2.pos_x + paddle2.width) {
    ball.pos_y = paddle2.pos_y - ball.height -1;
    ball.speed_y = -ball.speed_y;
    ball.speed_x += paddle2.speed_x / 2;
  }

  // Ball speed limit
  if (ball.speed_x > BALLMAXSPEED) { ball.speed_x = BALLMAXSPEED; }
  if (ball.speed_x < -BALLMAXSPEED) { ball.speed_x = -BALLMAXSPEED; }
  if (ball.speed_x == 0) { ball.speed_x = 1; } // Prevent ball from stopping    // Verhindern, dass der Ball stoppt
  // ball.speed_y can't rech 0, so it doesn't need to be limited   // ball.speed_y kann 0 nicht erreichen, also muss es nicht begrenzt werden

  // Paddle movement (Top player) (AI)
  if (ball.speed_y < 0) {
    if (paddle1.pos_x + paddle1.width / 2 < ball.pos_x) {
      paddle1.speed_x++;
    } else if (paddle1.pos_x + paddle1.width / 2 > ball.pos_x) {
      paddle1.speed_x--;
    }
  }

  // Paddle speed limit
  if (paddle1.speed_x > PADDLEMAXSPEED) { paddle1.speed_x = PADDLEMAXSPEED; }
  if (paddle1.speed_x < -PADDLEMAXSPEED) { paddle1.speed_x = -PADDLEMAXSPEED; }
  if (paddle2.speed_x > PADDLEMAXSPEED) { paddle2.speed_x = PADDLEMAXSPEED; }
  if (paddle2.speed_x < -PADDLEMAXSPEED) { paddle2.speed_x = -PADDLEMAXSPEED; }

  // Joystick mapping
  joy_x = analogRead(PIN_JOY_X);

  if (joy_x < joyXMin) { joyXMin = analogRead(PIN_JOY_X); }
  if (joy_x > joyXMax) { joyXMax = analogRead(PIN_JOY_X); }

  if (cpu) {
    // Paddle2 movement (Bottom player) (AI)    // Paddle2 Bewegung (Unterer Spieler) (KI)
    if (ball.speed_y > 0) {
      if (paddle2.pos_x + paddle2.width / 2 < ball.pos_x) {

        paddle2.speed_x++;
      } else if (paddle2.pos_x + paddle2.width / 2 > ball.pos_x) {
        paddle2.speed_x--;
      }
    }
  } else {
    // Paddle2 movement (Bottom player)    // Paddle2 Bewegung (Unterer Spieler)
    paddle2.speed_x = customMap(joy_x, joyXMin, joyXCenter, joyXMax, -PADDLEMAXSPEED, PADDLEMAXSPEED);
  }



  // Paddle bounds check    // Paddle Grenzen überprüfen
  if (paddle1.pos_x < 0) {
    paddle1.pos_x = 0;
    paddle1.speed_x = 0;
  }
  if (paddle1.pos_x > canv_w - paddle1.width) {
    paddle1.pos_x = canv_w - paddle1.width;
    paddle1.speed_x = 0;
  }
  if (paddle2.pos_x < 0) {
    paddle2.pos_x = 0;
    paddle2.speed_x = 0;
  }
  if (paddle2.pos_x > canv_w - paddle2.width) {
    paddle2.pos_x = canv_w - paddle2.width;
    paddle2.speed_x = 0;
  }

  drawGame();
}

void drawGame() {
  // Canvas clear    // Canvas löschen
  Canvas->fillScreen(ST77XX_BLACK);

  // Draw paddles and ball    // Paddles und Ball zeichnen
  Canvas->fillRoundRect(paddle1.pos_x, paddle1.pos_y, paddle1.width, paddle1.height, 3, paddle1.color);
  Canvas->fillRoundRect(paddle2.pos_x, paddle2.pos_y, paddle2.width, paddle2.height, 3, paddle2.color);
  Canvas->fillCircle(ball.pos_x + ball.width / 2, ball.pos_y + ball.height / 2, ball.width / 2, ball.color);

  // Draw Points    // Punkte zeichnen
  Canvas->setCursor(0, canv_h / 2 - 20);
  Canvas->setTextColor(ST77XX_WHITE);
  Canvas->setTextSize(2);
  Canvas->println(p1_points);
  Canvas->println("-");
  Canvas->print(p2_points);
  
  //Draw Canvas Buffer on Screen  // Zeichne den Canvas-Puffer auf den Bildschirm
  tft.drawRGBBitmap(0, 0, Canvas->getBuffer(), canv_w, canv_h);
}

void resetGame() {
  Canvas->fillScreen(ST77XX_BLACK);

  // Draw Gameover/Start Screen    // Spielende/Start Bildschirm zeichnen
  Canvas->setTextColor(ST77XX_WHITE);
  Canvas->setTextSize(2); // 12x16
  if (p1_points >= 10) {
    Canvas->setTextColor(paddle1.color);
    Canvas->setCursor(canv_w / 2 - 42, canv_h / 2 - 8);
    Canvas->println("Blau");
    Canvas->setCursor(canv_w / 2 - 42, canv_h / 2 + 8);
    Canvas->print(" gewinnt!");
  } else if (p2_points >= 10) {
    Canvas->setTextColor(paddle2.color);
    Canvas->setCursor(canv_w / 2 - 42, canv_h / 2 - 8);
    Canvas->println("Gruen");
    Canvas->setCursor(canv_w / 2 - 42, canv_h / 2 + 8);
    Canvas->print(" gewinnt!");
  } else {
    Canvas->setCursor(canv_w / 2 - 90, canv_h / 2 - 8);
    if (cpu) {
      Canvas->print("CPU vs. CPU");
    } else {
      Canvas->print("Spieler vs. CPU");
    }
    Canvas->setCursor(canv_w / 2 - 42, canv_h / 2 + 8);
    Canvas->println("Start!");
  }
  tft.drawRGBBitmap(0, 0, Canvas->getBuffer(), canv_w, canv_h);

  // Reset game state    // Spielzustand zurücksetzen
  resetRound();
  p1_points = 0;
  p2_points = 0;
  delay(5000);
}

void resetRound() {
  ball.pos_x = canv_w / 2 - ball.width / 2;
  ball.pos_y = canv_h / 2 - ball.height / 2;
  uint8_t speedx = random(BALLMAXSPEED / 2, BALLMAXSPEED);
  uint8_t speedy = random(BALLMAXSPEED / 2, BALLMAXSPEED);
  if (random(0, 2) == 0) {
    speedx = -speedx;
  }
  if (random(0, 2) == 0) {
    speedy = -speedy;
  }
  ball.speed_x = speedx;
  ball.speed_y = speedy;

  paddle1.pos_x = canv_w / 2 - paddle1.width / 2;
  paddle2.pos_x = canv_w / 2 - paddle2.width / 2;
  paddle1.speed_x = 0;
  paddle2.speed_x = 0;
}

float customMap(long x, long in_min, long in_center, long in_max, long out_min, long out_max) {
  if (x < in_center) {
    return (float)(x - in_center) / (in_center - in_min) * (out_min) *-1; // Skalierung für den Bereich links vom Mittelpunkt
  } else {
    return (float)(x - in_center) / (in_max - in_center) * (out_max); // Skalierung für den Bereich rechts vom Mittelpunkt
  }
}