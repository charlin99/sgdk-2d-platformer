#ifndef _GAME_H_
#define _GAME_H_

#include <genesis.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224

typedef enum
{
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER
} GameState;

extern Map *bga;
extern u16 camera_x;

void GAME_start();

#endif // _GAME_H_