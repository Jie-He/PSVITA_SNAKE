// must include this.. (Kernel stuff)
// will not compile under gcc as this is from VITASDK
#include <psp2/kernel/processmgr.h>
// more library from psp2
#include <psp2/ctrl.h>          // for the input
#include <psp2/common_dialog.h> // dunno what for. but get error if not

//get the c library int
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// import the vita2d library
#include <vita2d.h>

// import the sound library
/*
#include <soloud.h>
#include <soloud_c.h>
#include <soloud_wav.h>
*/
// some enum for the game state
enum GAME_STATE
{
  RUNNING,
  PAUSED,
  DEAD
};

struct Point
{
  short x;
  short y;
};

// define some colour
#define BLACK RGBA8(0, 0, 0, 0xFF)
#define WHITE RGBA8(0xFF, 0xFF, 0xFF, 0xFF)
#define CRIMSON RGBA8(0xFF, 0x24, 0, 0xFF)
#define SEAGREEN RGBA8(0x54, 0xFF, 0x9F, 0xFF)
#define AMBER RGBA8(0xFF, 0xBF, 0, 0xFF)
#define INC 20 //20 x 20 block

// font buffer
extern unsigned int basicfont_size;
extern unsigned char basicfont[];

// not going to use analog sticks and touch
SceCtrlData pad;

// define some images
vita2d_font *font;
vita2d_texture *bg;
vita2d_texture *snake;
vita2d_texture *snake_head;
vita2d_texture *apple;
vita2d_texture *gameover;
vita2d_texture *paused;

// define some sound stuff..
//Soloud *gsoloud;
// how fast the snake is updated. lower the faster
short sleep = 5;

struct Point snake_body[1058];
struct Point a;
short length;
short hv = 0;
short vv = INC;
enum GAME_STATE GSTATE;

struct Point previous_v;

void check_bound()
{
  // check for out of bound
  if (snake_body[0].x >= 940)
  {
    snake_body[0].x = 20;
  }

  if (snake_body[0].x < 20)
  {
    snake_body[0].x = 920;
  }

  // 64 is the upper bound
  if (snake_body[0].y < 64)
  {
    snake_body[0].y = 504;
  }

  if (snake_body[0].y >= 524)
  {
    snake_body[0].y = 64;
  }
}

void check_input()
{
  // check input
  if (pad.buttons & SCE_CTRL_UP)
  {
    vv = -INC;
    hv = 0;
  }

  if (pad.buttons & SCE_CTRL_DOWN)
  {
    vv = INC;
    hv = 0;
  }

  if (pad.buttons & SCE_CTRL_LEFT)
  {
    hv = -INC;
    vv = 0;
  }

  if (pad.buttons & SCE_CTRL_RIGHT)
  {
    hv = INC;
    vv = 0;
  }

  if (pad.buttons & SCE_CTRL_CROSS)
  {
    sleep = 3; // fast
  }

  if (pad.buttons & SCE_CTRL_CIRCLE)
  {
    sleep = 5; // normal
  }

  if (pad.buttons & SCE_CTRL_TRIANGLE)
  {
    sleep = 10; // slow
  }

  if (pad.buttons & SCE_CTRL_SQUARE)
  {
    // nothing for now
    //cheat
    length++;
    //sceKernelDelayThread(100000);
  }

  if (pad.buttons & SCE_CTRL_START)
  {
    GSTATE = PAUSED;
    sceKernelDelayThread(100000); // delay for 100ms
  }
}

// set the apple to a random point
void rnd_point()
{
  a.x = (rand() % 47) * INC;
  a.y = (rand() % 23) * INC + 64;

  for (int i = 0; i < length; i++)
  {
    if (a.x == snake_body[i].x && a.y == snake_body[i].y)
    {
      rnd_point();
    }
  }
}

// init the snake stuff
void setup_snake()
{
  length = 0;
  // init the test snake

  for (int i = 0; i < 3; i++)
  {
    //add 3 points to the snake body
    struct Point head;
    head.x = 400 - i * INC, head.y = 304;
    snake_body[i] = head;
    length++;
  }
  // and the apple
  rnd_point();
}

// if the snake has collided with itself.
// return 1, else 0
void check_collision()
{
  short x = snake_body[0].x, y = snake_body[0].y;
  for (int i = 1; i < length; i++)
  {
    if (snake_body[i].x == x && snake_body[i].y == y)
    {
      GSTATE = DEAD;
      return;
    }
  }
}

void render()
{

  char buff[30];

  // draw the background
  vita2d_draw_texture(bg, 0, 44);
  sprintf(buff, "LENGTH: %4d", length);
  vita2d_font_draw_text(font, 815, 33, WHITE, 24, buff);
  sprintf(buff, "SCORE: %4d APPLES", length - 3);
  vita2d_font_draw_text(font, 20, 33, WHITE, 24, buff);

  if (sleep == 3)
  {
    vita2d_font_draw_text(font, 400, 33, CRIMSON, 24, "SPEED: FAST");
  }
  else if (sleep == 5)
  {
    vita2d_font_draw_text(font, 400, 33, SEAGREEN, 24, "SPEED: NORMAL");
  }
  else
  {
    vita2d_font_draw_text(font, 400, 33, AMBER, 24, "SPEED: SLOW");
  }

  // draw apple
  vita2d_draw_texture(apple, a.x, a.y);
  
  // draw snake
  for (int i = 1; i < length; i++)
  {
    vita2d_draw_texture(snake, snake_body[i].x, snake_body[i].y);
  }

  vita2d_draw_texture(snake_head, snake_body[0].x, snake_body[0].y);
}

void update()
{
  // start drawing

  short update_snake = 0;
  previous_v.x = 0;
  previous_v.y = 0;

  while (1)
  {
    // get control input
    sceCtrlPeekBufferPositive(0, &pad, 1);

    if (pad.buttons == SCE_CTRL_SELECT)
    {
      break;
    } // quit game when select is pressed.

    vita2d_start_drawing();
    vita2d_clear_screen();

    if (GSTATE == RUNNING)
    {
      // check the input
      check_input();
      update_snake++;
      // draw the snake tail & update it
      if (update_snake >= sleep)
      {
        for (int i = length - 1; i > 0; i--)
        {
          //update the coordinate
          snake_body[i].x = snake_body[i - 1].x;
          snake_body[i].y = snake_body[i - 1].y;
          //vita2d_draw_texture(snake, snake_body[i].x, snake_body[i].y);
        }

        //check if the control are contradicting
        if ((hv == -previous_v.x && hv != 0) || (vv == -previous_v.y && vv != 0))
        {
          snake_body[0].x += previous_v.x;
          snake_body[0].y += previous_v.y;
        }
        else
        {
          snake_body[0].x += hv;
          snake_body[0].y += vv;
          previous_v.x = hv;
          previous_v.y = vv;
        }

        // check if hit on apple
        if (snake_body[0].x == a.x && snake_body[0].y == a.y)
        {
          // set the tail
          snake_body[length].x = snake_body[length - 1].x;
          snake_body[length].y = snake_body[length - 1].y;
          length++;
          // gen random point.. for now it will be in the body.
          rnd_point();
        }
        update_snake = 0;
        check_bound();
      }

      check_collision();
    }

    render();

    if (GSTATE == PAUSED)
    {
      vita2d_draw_texture(paused, 380, 237);
      if (pad.buttons & SCE_CTRL_START)
      {
        GSTATE = RUNNING;
        sceKernelDelayThread(100000); // delay for 100ms
      }
    }

    if (GSTATE == DEAD)
    {
      vita2d_draw_texture(gameover, 380, 237);
      if (pad.buttons & SCE_CTRL_START)
      {
        setup_snake();
        GSTATE = RUNNING;
        sceKernelDelayThread(100000); // delay for 100ms
      }
    }

    vita2d_end_drawing();
    vita2d_swap_buffers();
  }
}

int main(int argc, char *agrv[])
{

  // sound stuff
  /*
  Soloud_initEx(soloud, SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, 
              SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);
  Soloud_Wav_load(gWave, "app0:/coin.ogg");
  Soloud_Wav_play(gWave);*/

  vita2d_init();
  vita2d_set_clear_color(BLACK);
  // setup some stuff..
  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

  // load the font witht he font.c..
  font = vita2d_load_font_mem(basicfont, basicfont_size);

  // load the textures
  bg = vita2d_load_PNG_file("app0:/assets/background.png");
  snake = vita2d_load_PNG_file("app0:/assets/snake.png");
  snake_head = vita2d_load_PNG_file("app0:/assets/snake_head.png");
  apple = vita2d_load_PNG_file("app0:/assets/apple.png");
  gameover = vita2d_load_PNG_file("app0:/assets/gameover.png");
  paused = vita2d_load_PNG_file("app0:/assets/paused.png");

  setup_snake();
  GSTATE = PAUSED;
  update();

  vita2d_fini();

  vita2d_free_font(font);
  vita2d_free_texture(bg);
  vita2d_free_texture(snake);
  vita2d_free_texture(apple);
  vita2d_free_texture(snake_head);
  vita2d_free_texture(paused);
  vita2d_free_texture(gameover);

  sceKernelExitProcess(0);
  return 0;
}