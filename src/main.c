#include <genesis.h>
#include <resources.h>
#include "player.h"

Map *bga;

int main()
{
    // --- Configuração Inicial do VDP ---
    PAL_setPalette(PAL0, level_palette.data, DMA);
    VDP_loadTileSet(&level_tileset, TILE_USER_INDEX, DMA);

    // --- Configuração do Mapa e Sprites ---
    bga = MAP_create(&level_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    MAP_scrollTo(bga, 0, 0);

    SPR_init();
    PLAYER_init();

    // --- Configuração dos Controles ---
    JOY_setEventHandler(JOY_handler);

    // --- Loop Principal do Jogo ---
    while (1)
    {
        PLAYER_handle_input();
        PLAYER_update();

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}