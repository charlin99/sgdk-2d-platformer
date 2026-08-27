#ifndef _GAME_H_
#define _GAME_H_

#include <genesis.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224

typedef enum
{
    STATE_SPLASH,
    STATE_CREDITS,
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER
} GameState;

extern Map *bga;
extern u16 camera_x;

void GAME_start();
void GAME_trigger_gameover();
void GAME_update_hud();
void GAME_show_life_screen();
bool GAME_is_life_screen_active();

#endif // _GAME_H_
