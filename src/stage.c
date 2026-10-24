#include "stage.h"
#include "resources.h"
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))
static const StageDefinition stages[] = {
    {1, 1280, &level_map, &level_tileset,
     player_spawns, COUNT(player_spawns), slime_spawns, COUNT(slime_spawns),
     checkpoint_spawns, COUNT(checkpoint_spawns), cannon_spawns, COUNT(cannon_spawns),
     stage1_exits, COUNT(stage1_exits)},
    {2, 696, &stage2_map, &level_tileset,
     stage2_players, COUNT(stage2_players), NULL, 0, NULL, 0, NULL, 0,
     stage2_exits, COUNT(stage2_exits)}
};
const StageDefinition *current_stage = &stages[0];
const StageDefinition *STAGE_find(u16 id)
{
    for (u16 i = 0; i < COUNT(stages); i++) if (stages[i].id == id) return &stages[i];
    return NULL;
}
u16 STAGE_camera_limit(void) { return current_stage->width > 320 ? current_stage->width - 320 : 0; }
u16 STAGE_camera_at(u16 x)
{
    u16 camera = (x / 320) * 320;
    return camera > STAGE_camera_limit() ? STAGE_camera_limit() : camera;
}
u16 STAGE_exit_at(s32 x, s32 y, u16 width, u16 height)
{
    for (u16 i = 0; i < current_stage->exit_count; i++) {
        const StageExit *exit = current_stage->exits[i];
        if (exit->width && exit->height && x + width > exit->x && x < (s32)exit->x + exit->width &&
            y + height > exit->y && y < (s32)exit->y + exit->height) return i + 1;
    }
    return 0;
}

