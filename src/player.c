#include "player.h" // Inclui o nosso próprio cabeçalho
#include <resources.h>

// Definição das variáveis globais do jogador
Sprite *player;
u16 player_x = 30;
u16 player_y = 184;

// Implementação das funções
void PLAYER_init()
{
    // A sprite precisa ser definida no resources.res
    player = SPR_addSprite(&player_sprite, player_x, player_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}

void PLAYER_handle_input()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_LEFT)
    {
        player_x -= 1;
        SPR_setHFlip(player, TRUE);
    }
    else if (value & BUTTON_RIGHT)
    {
        player_x += 1;
        SPR_setHFlip(player, FALSE);
    }
}

void PLAYER_update()
{
    // Esta função vai crescer com física, animações, etc.
    SPR_setPosition(player, player_x, player_y);
}