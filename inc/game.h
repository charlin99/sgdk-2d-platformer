#ifndef _GAME_H_
#define _GAME_H_

#include <genesis.h>

void GAME_init();
void GAME_loop();
void GAME_handleJoy(u16 joy, u16 changed, u16 state);

#endif // _GAME_H_