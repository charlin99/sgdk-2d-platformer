#include <genesis.h>
#include <resources.h>

Sprite *player;
u16 player_x = 30;
u16 player_y = 184;

void player_init()
{
    player = SPR_addSprite(&player_sprite, player_x, player_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE)); // Adiciona o sprite do player
}

void player_move(u16 value)
{
    if (value & BUTTON_LEFT)
    {
        player_x -= 1;
    }
    else if (value & BUTTON_RIGHT)
    {
        player_x += 1;
    }

    SPR_setPosition(player, player_x, player_y);
}

int main()
{
    PAL_setPalette(PAL0, level_palette.data, DMA);                                                       // Define a paleta de cores na CRAM
    VDP_loadTileSet(&level_tileset, TILE_USER_INDEX, DMA);                                               // Carrega o tileset na VRAM
    Map *bga = MAP_create(&level_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX)); // Cria um mapa
    MAP_scrollTo(bga, 0, 0);                                                                             // Scrolla o mapa (ou inicia)

    SPR_init(); // Inicia a engine de sprites
    player_init();

    while (1)
    {
        u16 value = JOY_readJoypad(JOY_1);
        player_move(value);
        SPR_setPosition(player, player_x, player_y);
        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}
