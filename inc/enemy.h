#ifndef _ENEMY_H_
#define _ENEMY_H_

#define MAX_ENEMIES 8

#include <genesis.h>

void ENEMY_init();
void ENEMY_add(u16 x, u16 y);
void ENEMY_update_all();
void ENEMY_remove(u16 index);
void ENEMY_populate_from_map();

typedef struct
{
    Sprite* sprite;
    s16 x;
    s16 y;
    s16 vel_x;
    s8 move_timer;
    s16 min_x;
    s16 max_x;
    bool active;
} Enemy;

#endif // _ENEMY_H_