#include "stage.h"
#include <genesis.h>
#include "checkpoint.h"
#include "game.h"
#include "player.h"
#include "resources.h"
#include "resources_def.h"
#include "enemy.h"

// Maps may place at most eight objects; additional objects are ignored.
#define MAX_CHECKPOINTS 8
#define CHECKPOINT_FLAG_WIDTH 8
#define CHECKPOINT_FLAG_HEIGHT 10

typedef struct
{
    u16 x;
    u16 floor_y;
    s16 spawn_x;
    s16 spawn_y;
    bool spawn_is_safe;
    bool collected;
    Sprite *sprite;
} Checkpoint;

static Checkpoint checkpoints[MAX_CHECKPOINTS];
static u8 checkpoint_count = 0;

static bool spawn_position_is_safe(u16 spawn_x, u16 spawn_y);

static void create_checkpoint_sprite(u8 index)
{
    Checkpoint *checkpoint = &checkpoints[index];
    const SpriteDefinition *definition = checkpoint->collected ? &checkpoint_pole : &checkpoint_flag;
    s16 screen_x = checkpoint->x - 8 - camera_x;
    s16 screen_y = checkpoint->floor_y - PLAYER_HEIGHT + LEVEL_SCREEN_Y;

    checkpoint->sprite = SPR_addSprite(definition, screen_x, screen_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}

void CHECKPOINT_init()
{
    checkpoint_count = 0;

    const u16 count = current_stage->checkpoint_count;
    for (u16 i = 0; i < count && checkpoint_count < MAX_CHECKPOINTS; i++)
    {
        const CheckpointSpawn *spawn = (const CheckpointSpawn*)current_stage->checkpoints[i];
        if (spawn->x < 0 || spawn->y < 0) continue;
        Checkpoint *checkpoint = &checkpoints[checkpoint_count++];
        // Objects mark the sprite's top-left corner, like slime objects.
        checkpoint->x = spawn->x + 8;
        checkpoint->floor_y = spawn->y + PLAYER_HEIGHT;
        // -1 follows the flag; nonnegative values are absolute world positions.
        checkpoint->spawn_x = spawn->spawnX < 0 ? spawn->x - 8 : spawn->spawnX;
        checkpoint->spawn_y = spawn->spawnY < 0 ? spawn->y : spawn->spawnY;
        checkpoint->collected = FALSE;
        checkpoint->sprite = NULL;
    }

    // Terrain and enemy spawn objects are static, so validate only when loading.
    for (u8 i = 0; i < checkpoint_count; i++)
    {
        Checkpoint *checkpoint = &checkpoints[i];
        checkpoint->spawn_is_safe = checkpoint->spawn_x >= 0 && checkpoint->spawn_y >= 0 &&
                                   spawn_position_is_safe(checkpoint->spawn_x, checkpoint->spawn_y);
    }

    CHECKPOINT_update_sprites();
}

void CHECKPOINT_update_sprites()
{
    for (u8 i = 0; i < checkpoint_count; i++)
    {
        Checkpoint *checkpoint = &checkpoints[i];
        bool in_room = checkpoint->x + 8 >= camera_x && checkpoint->x - 8 < camera_x + SCREEN_WIDTH;

        if (!in_room)
        {
            if (checkpoint->sprite != NULL)
            {
                SPR_releaseSprite(checkpoint->sprite);
                checkpoint->sprite = NULL;
            }
            continue;
        }

        if (checkpoint->sprite == NULL)
        {
            create_checkpoint_sprite(i);
        }
        else
        {
            SPR_setPosition(checkpoint->sprite, checkpoint->x - 8 - camera_x, checkpoint->floor_y - PLAYER_HEIGHT + LEVEL_SCREEN_Y);
        }
    }
}

void CHECKPOINT_clear_sprites()
{
    // SPR_reset() invalidates every pointer, so do not release sprites here.
    for (u8 i = 0; i < checkpoint_count; i++)
    {
        checkpoints[i].sprite = NULL;
    }
}

static bool spawn_position_is_safe(u16 spawn_x, u16 spawn_y)
{
    if ((u32)spawn_x + PLAYER_WIDTH > (u32)current_stage->width ||
        (u32)spawn_y + PLAYER_HEIGHT >= LEVEL_VIEW_HEIGHT) return FALSE;
    u16 x_left = spawn_x + 2;
    u16 x_center = spawn_x + (PLAYER_WIDTH / 2);
    u16 x_right = spawn_x + PLAYER_WIDTH - 2;
    u16 y_top = spawn_y + 2;
    u16 y_center = spawn_y + (PLAYER_HEIGHT / 2);
    u16 y_bottom = spawn_y + PLAYER_HEIGHT - 3;
    u16 floor_y = spawn_y + PLAYER_HEIGHT;

    if (is_hard_solid_at(x_left, y_top) || is_hard_solid_at(x_center, y_top) || is_hard_solid_at(x_right, y_top) ||
        is_hard_solid_at(x_left, y_center) || is_hard_solid_at(x_center, y_center) || is_hard_solid_at(x_right, y_center) ||
        is_hard_solid_at(x_left, y_bottom) || is_hard_solid_at(x_center, y_bottom) || is_hard_solid_at(x_right, y_bottom))
    {
        return FALSE;
    }

    if (!(is_hard_solid_at(x_left, floor_y) || is_hard_solid_at(x_center, floor_y) || is_hard_solid_at(x_right, floor_y) ||
          is_platform_at(x_left, floor_y) || is_platform_at(x_center, floor_y) || is_platform_at(x_right, floor_y)))
    {
        return FALSE;
    }

    if (is_spike_at(x_left, y_top) || is_spike_at(x_center, y_top) || is_spike_at(x_right, y_top) ||
        is_spike_at(x_left, y_center) || is_spike_at(x_center, y_center) || is_spike_at(x_right, y_center) ||
        is_spike_at(x_left, y_bottom) || is_spike_at(x_center, y_bottom) || is_spike_at(x_right, y_bottom))
    {
        return FALSE;
    }

    // Slimes are recreated from objects after death.
    const u16 count = current_stage->slime_count;
    for (u16 i = 0; i < count; i++)
    {
        const SlimeSpawn *enemy = (const SlimeSpawn*)current_stage->slimes[i];
        if (spawn_x + PLAYER_WIDTH > enemy->x + 2 && spawn_x < enemy->x + 14 &&
            spawn_y + PLAYER_HEIGHT > enemy->y + 4 && spawn_y < enemy->y + 16)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static bool player_touches_flag(const Checkpoint *checkpoint)
{
    u16 flag_left = checkpoint->x - 2;
    u16 flag_right = flag_left + CHECKPOINT_FLAG_WIDTH;
    u16 flag_top = checkpoint->floor_y - PLAYER_HEIGHT;
    u16 flag_bottom = flag_top + CHECKPOINT_FLAG_HEIGHT;
    u16 player_right = player_x + PLAYER_WIDTH;
    u16 player_bottom = player_y + PLAYER_HEIGHT;

    return player_right > flag_left && player_x < flag_right &&
           player_bottom > flag_top && player_y < flag_bottom;
}

void CHECKPOINT_update()
{
    CHECKPOINT_update_sprites();

    for (u8 i = 0; i < checkpoint_count; i++)
    {
        Checkpoint *checkpoint = &checkpoints[i];
        // Collected flags stay consumed, including after changing rooms or dying.
        if (checkpoint->collected) continue;
        if (!player_touches_flag(checkpoint)) continue;

        u16 spawn_x = checkpoint->spawn_x;
        u16 spawn_y = checkpoint->spawn_y;
        if (!checkpoint->spawn_is_safe) continue;

        checkpoint->collected = TRUE;
        player_spawn_x = spawn_x;
        player_spawn_y = spawn_y;
        player_health = 2;
        player_current_anim = (u16)-1;

        // Keep the sprite and permanently switch this flag to its collected state.
        if (checkpoint->sprite != NULL)
        {
            SPR_setDefinition(checkpoint->sprite, &checkpoint_pole);
        }
        PLAYER_update_anim();
        return;
    }
}
