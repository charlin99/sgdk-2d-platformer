#include "game.h"
#include "enemy.h"
#include "resources.h"

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

void ENEMY_add(const SlimeSpawn* spawn)
{
    s16 range = spawn->patrolRange > 0 ? spawn->patrolRange : 0;
    s32 min_x = (s32)spawn->x - range;
    s32 max_x = (s32)spawn->x + range;
    s32 map_right = (s32)level_map.w * 128 - 16;

    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].x = spawn->x;
            enemies[i].y = spawn->y;
            enemies[i].min_x = min_x < 0 ? 0 : min_x;
            enemies[i].max_x = max_x > map_right ? map_right : max_x;
            enemies[i].speed = spawn->speed > 0 ? spawn->speed : 0;
            enemies[i].vel_x = enemies[i].speed;
            enemies[i].move_delay = spawn->moveDelay > 0 ? spawn->moveDelay : 1;
            enemies[i].move_timer = enemies[i].move_delay;

            enemies[i].sprite = SPR_addSprite(&slime, enemies[i].x - camera_x, enemies[i].y, TILE_ATTR(PAL0, TRUE, FALSE, TRUE));
            if (enemies[i].sprite == NULL) return;
            enemies[i].active = TRUE;
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
                enemies[i].move_timer = enemies[i].move_delay;
                s32 next_x = (s32)enemies[i].x + enemies[i].vel_x;

                if (next_x <= enemies[i].min_x) {
                    enemies[i].x = enemies[i].min_x;
                    enemies[i].vel_x = enemies[i].speed;
                    SPR_setHFlip(enemies[i].sprite, TRUE);
                }
                else if (next_x >= enemies[i].max_x) {
                    enemies[i].x = enemies[i].max_x;
                    enemies[i].vel_x = -enemies[i].speed;
                    SPR_setHFlip(enemies[i].sprite, FALSE);
                }
                else {
                    enemies[i].x = next_x;
                }
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

void ENEMY_clear()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].sprite != NULL)
        {
            SPR_releaseSprite(enemies[i].sprite);
        }

        enemies[i].active = FALSE;
        enemies[i].sprite = NULL;
    }
}

void ENEMY_set_animations_paused(bool paused)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active && enemies[i].sprite != NULL)
        {
            SPR_setAutoAnimation(enemies[i].sprite, !paused);
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

void ENEMY_populate_from_map()
{
    ENEMY_clear();


    const u16 count = sizeof(slime_spawns) / sizeof(slime_spawns[0]);
    for (u16 i = 0; i < count; i++)
    {
        const SlimeSpawn* spawn = (const SlimeSpawn*)slime_spawns[i];
        if (spawn->x >= camera_x && spawn->x < camera_x + SCREEN_WIDTH)
        {
            ENEMY_add(spawn);
        }
    }
}
