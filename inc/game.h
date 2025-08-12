#ifndef _GAME_H_
#define _GAME_H_

#include <genesis.h>

typedef enum
{
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER
} GameState;

void GAME_start();

#endif // _GAME_H_