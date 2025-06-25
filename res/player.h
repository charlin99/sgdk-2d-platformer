#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <genesis.h>

extern Sprite *player;

void PLAYER_init();
void PLAYER_handle_input(); // Renomeado para mais clareza
void PLAYER_update();       // Uma nova função para a lógica de atualização

#endif // _PLAYER_H_