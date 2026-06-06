#ifndef _CANNON_H_
#define _CANNON_H_

#include <genesis.h>

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s16 vel_x;
    bool active;
} Bullet;

void CANNON_init();
void CANNON_update();

#endif // _CANNON_H_