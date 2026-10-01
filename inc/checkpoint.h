#ifndef _CHECKPOINT_H_
#define _CHECKPOINT_H_

#include <genesis.h>

// Field order matches checkpoint_spawns in resources.res.
typedef struct
{
    s16 x;
    s16 y;
    s16 spawnX;
    s16 spawnY;
} CheckpointSpawn;

void CHECKPOINT_init();
void CHECKPOINT_update();
void CHECKPOINT_update_sprites();
void CHECKPOINT_clear_sprites();

#endif // _CHECKPOINT_H_
