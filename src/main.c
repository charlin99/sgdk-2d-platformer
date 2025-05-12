#include <genesis.h>
#include <resources.h>

int main()
{
    PAL_setPalette(PAL0, level_palette.data, DMA);                                                       // Define a paleta de cores na CRAM
    VDP_loadTileSet(&level_tileset, TILE_USER_INDEX, DMA);                                               // Carrega o tileset na VRAM
    Map *bga = MAP_create(&level_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX)); // Cria um mapa
    MAP_scrollTo(bga, 0, 0);                                                                             // Scrolla o mapa (ou inicia)
    while (1)
    {

        SYS_doVBlankProcess();
    }
    return (0);
}
