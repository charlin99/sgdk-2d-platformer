#include <genesis.h>
#include <resources.h>
#include "player.h"

int main()
{
    PAL_setPalette(PAL0, level_palette.data, DMA);
    VDP_loadTileSet(&level_tileset, TILE_USER_INDEX, DMA);
    Map *bga = MAP_create(&level_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    MAP_scrollTo(bga, 0, 0);

    SPR_init();
    PLAYER_init();

    while (1)
    {
        PLAYER_handle_input();

        PLAYER_update();

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}