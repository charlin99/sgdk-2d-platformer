#ifndef _ENEMY_H_
#define _ENEMY_H_

#define MAX_ENEMIES 8

#include <genesis.h>

// Field order must match the OBJECTS declaration in resources.res.
typedef struct
{
    s16 x;
    s16 y;
    s16 speed;
    s16 moveDelay;
    s16 patrolRange;
} SlimeSpawn;

void ENEMY_init();
void ENEMY_clear();
void ENEMY_add(const SlimeSpawn* spawn);
void ENEMY_update_all();
void ENEMY_set_animations_paused(bool paused);
void ENEMY_remove(u16 index);
void ENEMY_populate_from_map();

typedef struct
{
    Sprite* sprite;
    s16 x;
    s16 y;
    s16 vel_x;
    s16 speed;
    s16 move_delay;
    s16 move_timer;
    s16 min_x;
    s16 max_x;
    bool active;
} Enemy;

#endif // _ENEMY_H_
