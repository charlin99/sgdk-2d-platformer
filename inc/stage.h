#ifndef _STAGE_H_
#define _STAGE_H_
#include <genesis.h>
#define STAGE_INITIAL_ID 1
#define STAGE_MAX_CANNONS 16
// Field order matches the OBJECTS declaration, including exportSize geometry.
typedef struct { s16 x, y; u16 width, height, nextStage; } StageExit;
typedef struct {
    u16 id, width;
    const MapDefinition *map;
    const TileSet *tileset;
    const void * const *players; u16 player_count;
    const void * const *slimes; u16 slime_count;
    const void * const *checkpoints; u16 checkpoint_count;
    const void * const *cannons; u16 cannon_count;
    const void * const *exits; u16 exit_count;
} StageDefinition;
extern const StageDefinition *current_stage;
const StageDefinition *STAGE_find(u16 id);
u16 STAGE_camera_at(u16 x);
u16 STAGE_camera_limit(void);
u16 STAGE_exit_at(s32 x, s32 y, u16 width, u16 height);
#endif
