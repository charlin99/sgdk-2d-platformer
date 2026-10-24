#ifndef _CANNON_H_
#define _CANNON_H_

#include <genesis.h>

typedef enum
{
    CANNON_RIGHT = 0,
    CANNON_DOWN_RIGHT,
    CANNON_DOWN,
    CANNON_DOWN_LEFT,
    CANNON_LEFT,
    CANNON_UP_LEFT,
    CANNON_UP,
    CANNON_UP_RIGHT
} CannonDirection;

// Field order matches cannon_spawns in resources.res.
typedef struct
{
    s16 x;
    s16 y;
    u16 direction;
    fix16 shotSpeed;
    u16 fireInterval;
    u16 initialDelay;
} CannonSpawn;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s32 position_x;
    s32 position_y;
    fix16 vel_x;
    fix16 vel_y;
    bool active;
} Bullet;

void CANNON_clear();
void CANNON_init();
void CANNON_update();

#endif // _CANNON_H_
