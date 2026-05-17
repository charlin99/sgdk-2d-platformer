#include "game.h"
#include "enemy.h"
#include "resources.h"

#define ENEMY_SPEED     1
#define ENEMY_MOVE_DELAY 3

extern u16 camera_x;

Enemy enemies[MAX_ENEMIES];

void ENEMY_init()
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = FALSE;
        enemies[i].sprite = NULL;
    }
}

void ENEMY_add(u16 x, u16 y)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].x = x;
            enemies[i].y = y;
            enemies[i].min_x = x - 40; 
            enemies[i].max_x = x + 40;
            enemies[i].vel_x = ENEMY_SPEED;
            enemies[i].active = TRUE;
            enemies[i].move_timer = ENEMY_MOVE_DELAY;

            enemies[i].sprite = SPR_addSprite(&slime, enemies[i].x, enemies[i].y, TILE_ATTR(PAL0, TRUE, FALSE, TRUE));
            SPR_setAnim(enemies[i].sprite, 0);
            return;
        }
    }
}

void ENEMY_update_all()
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            enemies[i].move_timer--;

            if (enemies[i].move_timer <= 0)
            {
                enemies[i].move_timer = ENEMY_MOVE_DELAY;
                enemies[i].x += enemies[i].vel_x;
            }

            if (enemies[i].x <= enemies[i].min_x) {
                enemies[i].vel_x = ENEMY_SPEED;
                SPR_setHFlip(enemies[i].sprite, TRUE);
            } 
            else if (enemies[i].x >= enemies[i].max_x) {
                enemies[i].vel_x = -ENEMY_SPEED;
                SPR_setHFlip(enemies[i].sprite, FALSE);
            }

            s16 screen_x = enemies[i].x - camera_x;

            if (screen_x < -32 || screen_x > 320) {
                SPR_setVisibility(enemies[i].sprite, HIDDEN);
            } else {
                SPR_setVisibility(enemies[i].sprite, VISIBLE);
                SPR_setPosition(enemies[i].sprite, screen_x, enemies[i].y);
            }
        }
    }
}

void ENEMY_remove(u16 index)
{
    if (index < MAX_ENEMIES && enemies[index].active)
    {
        enemies[index].active = FALSE;
        if (enemies[index].sprite != NULL)
        {
            SPR_releaseSprite(enemies[index].sprite);
            enemies[index].sprite = NULL;
        }
    }
}