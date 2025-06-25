#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <genesis.h>

extern Sprite *player;

void PLAYER_init();
void PLAYER_handle_input();
void PLAYER_update();
void JOY_handler(u16 joy, u16 changed, u16 state);

#endif