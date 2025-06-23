#include "device_driver.h"
#include "graphics.h"
#include "timer.h"

#define LCDW (320)
#define LCDH (240)
#define X_MIN (0)
#define X_MAX (LCDW - 1)
#define Y_MIN (0)
#define Y_MAX (LCDH - 1)

#define TIMER_PERIOD (10)
#define RIGHT (1)
#define LEFT (-1)
#define HOME (0)
#define SCHOOL (1)

#define FROG_STEP (10)

#define BACK_COLOR (5)
#define GAME_OVER (1)

static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, BLACK};
int cheatcode = 0;

class Object {
 public:
  int x, y, w, h, ci, dir;

 public:
  Object(int x, int y, int w, int h, int dir)
      : x(x), y(y), w(w), h(h), dir(dir) {}

  virtual void draw() = 0;
  void clear() { Lcd_Draw_Box(x, y, w, h, color[BACK_COLOR]); }

  bool checkCollision(const Object &other) const {
    return !((x + w <= other.x) || (x >= other.x + other.w) ||
             (y + h <= other.y) || (y >= other.y + other.h));
  }
};

class Car : public Object {
  int speed;
  int ci;

 public:
  Car(int x, int y, int car_x, int car_y, int dir, int speed, int colorIndex)
      : Object(x, y, car_x, car_y, dir), speed(speed), ci(colorIndex) {}

  virtual void draw() {
    if (w == 20) {
      Lcd_Draw_Box(x + 5, y, 10, 8, color[ci]);
      Lcd_Draw_Box(x, y + 8, 20, 9, color[ci]);
      Lcd_Draw_Box(x + 4, y + 17, 3, 3, color[ci]);
      Lcd_Draw_Box(x + 14, y + 17, 3, 3, color[ci]);
    } else {
      Lcd_Draw_Box(x + 8, y, 14, 12, color[ci]);
      Lcd_Draw_Box(x, y + 12, 30, 13, color[ci]);
      Lcd_Draw_Box(x + 5, y + 25, 5, 5, color[ci]);
      Lcd_Draw_Box(x + 20, y + 25, 5, 5, color[ci]);
    }
  }

  void move() {
    x += speed * dir;
    if ((x + w >= X_MAX) || (x <= X_MIN)) {
      dir = -dir;
    }
  }
};

class Frog : public Object {
  int score;

 public:
  Frog(int x, int y, int frogsizeX, int frogsizeY)
      : Object(x, y, 20, 20, 1), score(0) {}

  virtual void draw() {
    Lcd_Draw_Box(x + 5, y + 5, 10, 10, color[2]);
    Lcd_Draw_Box(x + 2, y + 2, 3, 3, color[4]);
    Lcd_Draw_Box(x + 15, y + 2, 3, 3, color[4]);
    Lcd_Draw_Box(x, y + 5, 3, 5, color[2]);
    Lcd_Draw_Box(x + 17, y + 5, 3, 5, color[2]);
    Lcd_Draw_Box(x, y + 15, 3, 5, color[2]);
    Lcd_Draw_Box(x + 17, y + 15, 3, 5, color[2]);
  }

  void moveUp() {
    if (y > Y_MIN) y -= FROG_STEP;
  }

  void moveDown() {
    if (y + h < Y_MAX) y += FROG_STEP;
  }

  void moveLeft() {
    if (x > X_MIN) x -= FROG_STEP;
  }

  void moveRight() {
    if (x + w < X_MAX) x += FROG_STEP;
  }

  int updateState() {
    if (dir == SCHOOL && y == Y_MIN) {
      Uart_Printf("SCHOOL\n");
      dir = HOME;
    } else if (dir == HOME && y == LCDH - h) {
      Uart_Printf("HOME\n");
      dir = SCHOOL;
      score++;
      Uart_Printf("Score: %d\n", score);
    } else if (cheatcode == 1) {
      score = 4;
    }
    return score;
  }

  int getScore() const { return score; }
};

extern volatile int TIM4_expired;
extern volatile int Jog_key_in;
extern volatile int Jog_key;

extern "C" void abort(void) { while (1); }

extern "C" void __cxa_pure_virtual() { while (1); }

extern "C" void __cxa_deleted_virtual() { while (1); }

namespace __gnu_cxx {
void _verbose_terminate_handler() {
  Uart1_Printf("Unexpected error\n");
  while (1);
}
}  // namespace __gnu_cxx

static void Sys_Init(void) {
  Clock_Init();
  LED_Init();
  Uart_Init(115200);
  Lcd_Init();
  Jog_Poll_Init();
  Jog_ISR_Enable(1);
  Uart1_RX_Interrupt_Enable(1);
  Lcd_Clr_Screen();
  SCB->VTOR = 0x08003000;
  SCB->SHCSR = 7 << 16;
}

extern "C" void drawFrog(int x, int y, int colorBody, int colorEyes) {
  Lcd_Draw_Box(x + 5, y + 5, 10, 10, colorBody);
  Lcd_Draw_Box(x + 2, y + 2, 3, 3, colorEyes);
  Lcd_Draw_Box(x + 15, y + 2, 3, 3, colorEyes);
  Lcd_Draw_Box(x, y + 5, 3, 5, colorBody);
  Lcd_Draw_Box(x + 17, y + 5, 3, 5, colorBody);
  Lcd_Draw_Box(x, y + 15, 3, 5, colorBody);
  Lcd_Draw_Box(x + 17, y + 15, 3, 5, colorBody);
}

extern "C" void startPage() {
  Lcd_Printf(70, 30, WHITE, BLACK, 2, 2, "Crossy Frog");
  Lcd_Printf(80, 150, WHITE, BLACK, 2, 2, "Game Start");
  Lcd_Printf(75, 200, WHITE, BLACK, 1, 1, "Press any key to start");
}

extern "C" void startBlackPage() {
  Lcd_Printf(75, 200, BLACK, BLACK, 1, 1, "Press any key to start");
}

extern "C" void drawExplosion(int x, int y) {
  for (int i = 0; i < 3; ++i) {
    Lcd_Draw_Box(x - i * 5, y - i * 5, 10 + i * 10, 10 + i * 10, color[0]);
    TIM2_Delay(50);
  }
}

extern "C" void Main() {
  Sys_Init();
  Uart_Printf("Crossy Frog\n");
  drawFrog(150, 110, GREEN, WHITE);

  for (;;) {
    startPage();
    TIM2_Delay(100);
    startBlackPage();
    TIM2_Delay(100);
    if (Jog_key_in) {
      break;
    }
  }

  Uart_Printf("Game Start\n");

  Frog drawFrog(150, 220, 20, 20);
  Car cars[3] = {Car(0, 50, 20, 20, RIGHT, 3, 0),
                 Car(0, 110, 20, 20, RIGHT, 4, 2),
                 Car(0, 170, 20, 20, RIGHT, 5, 4)};

  Car new_cars[2] = {Car(0, 80, 20, 20, RIGHT, 6, 0),
                     Car(0, 140, 20, 20, RIGHT, 7, 2)};

  Car big_cars[3] = {Car(0, 50, 30, 30, RIGHT, 6, 0),
                     Car(0, 110, 30, 30, RIGHT, 7, 2),
                     Car(0, 170, 30, 30, RIGHT, 8, 4)};
  int carCount = 1;
  int game_over = 0;
  int k0state = 0;
  int k1state = 0;
  bool clear_once = false;

  while (1) {
    Lcd_Clr_Screen();
    drawFrog.draw();

    TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);
    clear_once = false;

    while (!game_over) {
      if (Jog_key_in) {
        drawFrog.clear();
        switch (Jog_key) {
          case 0:
            drawFrog.moveUp();
            break;
          case 1:
            drawFrog.moveDown();
            break;
          case 2:
            drawFrog.moveLeft();
            break;
          case 3:
            drawFrog.moveRight();
            break;
        }
        drawFrog.draw();
        Jog_key_in = 0;

        for (int i = 0; i < carCount; ++i) {
          if (drawFrog.checkCollision(cars[i])) {
            drawExplosion(drawFrog.x, drawFrog.y);
            game_over = 1;
            Uart_Printf("Game Over\n");
          }
        }
      }

      if (TIM4_expired) {
        drawFrog.updateState();
        int score = drawFrog.getScore();

        if (score < 4) {
          for (int i = 0; i < carCount; ++i) {
            cars[i].clear();
            cars[i].move();
            cars[i].draw();

            if (drawFrog.checkCollision(cars[i])) {
              drawExplosion(drawFrog.x, drawFrog.y);
              game_over = 1;
              Uart_Printf("Game Over\n");
            }
          }
        }
        if (score == 0) {
          Lcd_Printf(250, 0, WHITE, BLACK, 1, 1, "Score: %d", score);
          if (Macro_Check_Bit_Clear(GPIOA->IDR, 13)) {
            Jog_Wait_Key_Pressed();
            Jog_Wait_Key_Released();
            k0state++;
            Uart1_Printf("%d", k0state);
          }

          if (Macro_Check_Bit_Clear(GPIOA->IDR, 14)) {
            Jog_Wait_Key_Pressed();
            Jog_Wait_Key_Released();
            k1state++;
            Uart1_Printf("%d", k1state);
          }

          if (k0state == 2 && k1state == 2) {
            cheatcode = 1;
            k0state = 0;
            k1state = 0;
            Uart1_Printf("Cheatcode Activated\n");
          }
        } else if (score == 1) {
          Lcd_Printf(250, 0, WHITE, BLACK, 1, 1, "Score: %d", score);
          carCount = 2;
        } else if (score == 2) {
          Lcd_Printf(250, 0, WHITE, BLACK, 1, 1, "Score: %d", score);
          carCount = 3;
        } else if (score == 3) {
          Lcd_Printf(250, 0, WHITE, BLACK, 1, 1, "Score: %d", score);
          if (cheatcode == 1) {
            cheatcode = 0;
          }
          carCount = 0;
          carCount = 3;

          for (int i = 0; i < carCount; ++i) {
            new_cars[i].clear();
            new_cars[i].move();
            new_cars[i].draw();

            if (drawFrog.checkCollision(new_cars[i])) {
              drawExplosion(drawFrog.x, drawFrog.y);
              game_over = 1;
              Uart_Printf("Game Over\n");
            }
          }
        } else if (score == 4 && !clear_once) {
          Lcd_Printf(250, 0, WHITE, BLACK, 1, 1, "Score: %d", score);
          for (int i = 0; i < carCount; ++i) {
            cars[i].clear();
            new_cars[i].clear();
          }
          clear_once = true;
          carCount = 3;
        } else if (score == 4) {
          Lcd_Printf(250, 0, WHITE, BLACK, 1, 1, "Score: %d", score);
          carCount = 3;
          for (int i = 0; i < carCount; ++i) {
            big_cars[i].clear();
            big_cars[i].move();
            big_cars[i].draw();

            if (drawFrog.checkCollision(big_cars[i])) {
              drawExplosion(drawFrog.x, drawFrog.y);
              game_over = 1;
              Uart_Printf("Game Over\n");
            }
          }
        } else if (score == 5) {
          Lcd_Clr_Screen();
          Lcd_Printf(80, 100, WHITE, BLACK, 2, 2, "Game Clear");
          Lcd_Printf(50, 150, WHITE, BLACK, 1, 1, "Frog goes to school safely");
          game_over = 1;
          Uart_Printf("Game Clear\n");
        }
        TIM4_expired = 0;
      }
    }

    TIM4_Repeat_Interrupt_Enable(0, 0);
    Uart_Printf("Game Over, Please press any key to continue.\n");
    Jog_Wait_Key_Pressed();
    Jog_Wait_Key_Released();
    Uart_Printf("Game Start\n");

    drawFrog = Frog(150, 220, 20, 20);
    cars[0] = Car(0, 50, 20, 20, RIGHT, 3, 0);
    cars[1] = Car(0, 110, 20, 20, RIGHT, 4, 2);
    cars[2] = Car(0, 170, 20, 20, RIGHT, 5, 4);

    new_cars[0] = Car(0, 80, 20, 20, RIGHT, 6, 0);
    new_cars[1] = Car(0, 140, 20, 20, RIGHT, 7, 2);

    big_cars[0] = Car(0, 50, 30, 30, RIGHT, 6, 0);
    big_cars[1] = Car(0, 110, 30, 30, RIGHT, 7, 2);
    big_cars[2] = Car(0, 170, 30, 30, RIGHT, 8, 4);

    cheatcode = 0;
    carCount = 1;
    game_over = 0;
  }
}
