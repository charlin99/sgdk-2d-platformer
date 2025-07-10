#include "game.h"
#include "player.h"
#include "resources.h"
#include "resources_def.h"

Map *bga;

void GAME_handleJoy(u16 joy, u16 changed, u16 state)
{
    if (joy == JOY_1)
    {
        PLAYER_handle_joy(changed, state);
    }
}

void GAME_init()
{
    PAL_setPalette(PAL0, level_palette.data, DMA);
    VDP_loadTileSet(&level_tileset, TILE_USER_INDEX, DMA);

    bga = MAP_create(&level_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    MAP_scrollTo(bga, 0, 0);

    XGM_setPCM(SFX_JUMP_ID, sfx_jump, sizeof(sfx_jump));
    XGM_setPCM(SFX_WALK_ID, sfx_walk, sizeof(sfx_walk));

    PLAYER_init();

    JOY_setEventHandler(GAME_handleJoy);
}

void GAME_loop()
{
    while(1)
    {
        PLAYER_handle_input();
        PLAYER_update();
        PLAYER_update_anim();

        SPR_update();
        SYS_doVBlankProcess();
    }
}