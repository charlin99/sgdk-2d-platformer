#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <genesis.h>

extern Map *bga;

void PLAYER_init();
void PLAYER_handle_input();
void PLAYER_update();
void PLAYER_update_anim();
void PLAYER_try_jump();
void PLAYER_handle_joy(u16 changed, u16 state);

#endif // _PLAYER_H_