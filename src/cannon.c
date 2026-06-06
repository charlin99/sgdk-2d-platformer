#include "cannon.h"
#include "player.h"
#include "resources.h"
#include <resources_def.h>

#define MAX_BULLETS 4
#define FIRE_COOLDOWN 120

typedef struct {
    u16 x;
    u16 y;
    s16 dir_x;
} CannonPoint;

static const CannonPoint cannon_table[] = {
    { 56, 120,  1 }
};
#define CANNON_COUNT (sizeof(cannon_table) / sizeof(cannon_table[0]))

static Bullet bullets[MAX_BULLETS];
static u16 fire_timer = 0;

void CANNON_init()
{
    fire_timer = 0;
    for (u16 i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].sprite = NULL;
        bullets[i].active = FALSE;
    }
}

static void spawn_bullet(s16 x, s16 y, s16 dir_x)
{
    for (u16 i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
        {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].vel_x = dir_x * 3;
            bullets[i].active = TRUE;

            bullets[i].sprite = SPR_addSprite(&cannonball_sprite, x, y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
            XGM_startPlayPCM(SFX_CANNON_ID, 15, SOUND_PCM_CH2);
            break;
        }
    }
}

void CANNON_update()
{
    fire_timer++;
    if (fire_timer >= FIRE_COOLDOWN)
    {
        fire_timer = 0;
        for (u16 i = 0; i < CANNON_COUNT; i++)
        {
            spawn_bullet(cannon_table[i].x, cannon_table[i].y + 2, cannon_table[i].dir_x);
        }
    }

    for (u16 i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            bullets[i].x += bullets[i].vel_x;

            if (bullets[i].x < camera_x || bullets[i].x > camera_x + 320)
            {
                bullets[i].active = FALSE;
                if (bullets[i].sprite != NULL)
                {
                    SPR_releaseSprite(bullets[i].sprite);
                    bullets[i].sprite = NULL;
                }
                continue;
            }

            if (bullets[i].x + 4 >= player_x && bullets[i].x <= player_x + PLAYER_WIDTH &&
                bullets[i].y + 4 >= player_y && bullets[i].y <= player_y + PLAYER_HEIGHT)
            {
                if (player_hurt_timer == 0 && player_invincible_timer == 0)
                {
                    if (player_health > 1)
                    {
                        player_health--;
                        PLAYER_take_damage(bullets[i].x); 
                    }
                    else
                    {
                        PLAYER_die();
                    }
                }

                bullets[i].active = FALSE;
                if (bullets[i].sprite != NULL)
                {
                    SPR_releaseSprite(bullets[i].sprite);
                    bullets[i].sprite = NULL;
                }
                continue;
            }

            if (bullets[i].sprite != NULL)
            {
                SPR_setPosition(bullets[i].sprite, bullets[i].x - camera_x, bullets[i].y);
            }
        }
    }
}