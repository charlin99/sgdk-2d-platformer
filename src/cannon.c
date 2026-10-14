#include "cannon.h"
#include "player.h"
#include "game.h"
#include "resources.h"
#include "resources_def.h"

#define MAX_BULLETS 16
#define BULLET_SIZE 8
#define CANNON_COUNT (sizeof(cannon_spawns) / sizeof(cannon_spawns[0]))

static Bullet bullets[MAX_BULLETS];
static u16 fire_timers[CANNON_COUNT];

// Clockwise, starting at right. Screen Y grows downwards.
static const s8 direction_x[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
static const s8 direction_y[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

static void release_bullet(u16 index)
{
    if (bullets[index].sprite != NULL)
    {
        SPR_releaseSprite(bullets[index].sprite);
    }
    bullets[index].sprite = NULL;
    bullets[index].active = FALSE;
}

void CANNON_init()
{
    for (u16 i = 0; i < CANNON_COUNT; i++)
    {
        const CannonSpawn *cannon = (const CannonSpawn*)cannon_spawns[i];
        fire_timers[i] = cannon->initialDelay;
    }
    for (u16 i = 0; i < MAX_BULLETS; i++)
    {
        release_bullet(i);
    }
}

static void spawn_bullet(const CannonSpawn *cannon)
{
    if (cannon->direction >= 8 || cannon->shotSpeed <= 0) return;

    s8 dx = direction_x[cannon->direction];
    s8 dy = direction_y[cannon->direction];
    fix16 component_speed = cannon->shotSpeed;
    if (dx != 0 && dy != 0)
    {
        // 181/256 approximates 1/sqrt(2), preserving diagonal speed.
        component_speed = ((s32)cannon->shotSpeed * 181) / 256;
    }

    for (u16 i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
        {
            bullets[i].sprite = SPR_addSprite(&cannonball_sprite,
                                             cannon->x - camera_x, cannon->y + LEVEL_SCREEN_Y,
                                             TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
            if (bullets[i].sprite == NULL) return;

            bullets[i].x = cannon->x;
            bullets[i].y = cannon->y;
            // Wide accumulators allow fractional speed across the whole level.
            bullets[i].position_x = (s32)cannon->x * (1 << FIX16_FRAC_BITS);
            bullets[i].position_y = (s32)cannon->y * (1 << FIX16_FRAC_BITS);
            bullets[i].vel_x = dx * component_speed;
            bullets[i].vel_y = dy * component_speed;
            bullets[i].active = TRUE;
            XGM_startPlayPCM(SFX_CANNON_ID, 15, SOUND_PCM_CH2);
            return;
        }
    }
}

void CANNON_update()
{
    for (u16 i = 0; i < CANNON_COUNT; i++)
    {
        const CannonSpawn *cannon = (const CannonSpawn*)cannon_spawns[i];
        if (cannon->x < camera_x || cannon->x >= camera_x + SCREEN_WIDTH) continue;

        if (fire_timers[i] > 0) fire_timers[i]--;
        if (fire_timers[i] == 0)
        {
            spawn_bullet(cannon);
            fire_timers[i] = cannon->fireInterval > 0 ? cannon->fireInterval : 1;
        }
    }

    for (u16 i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active) continue;

        bullets[i].position_x += bullets[i].vel_x;
        bullets[i].position_y += bullets[i].vel_y;
        bullets[i].x = bullets[i].position_x >> FIX16_FRAC_BITS;
        bullets[i].y = bullets[i].position_y >> FIX16_FRAC_BITS;

        if (bullets[i].x + BULLET_SIZE <= camera_x || bullets[i].x >= camera_x + SCREEN_WIDTH ||
            bullets[i].y + BULLET_SIZE <= 0 || bullets[i].y >= LEVEL_VIEW_HEIGHT)
        {
            release_bullet(i);
            continue;
        }

        // Preserve the original projectile damage area.
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
                    return;
                }
            }
            release_bullet(i);
            continue;
        }

        SPR_setPosition(bullets[i].sprite, bullets[i].x - camera_x, bullets[i].y + LEVEL_SCREEN_Y);
    }
}
