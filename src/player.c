#include "player.h"
#include <resources.h>

Sprite *player;
u16 player_x = 30;
u16 player_y = 184;
s16 player_vy = 0;
bool player_on_ground = FALSE;

void PLAYER_init()
{
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

void PLAYER_try_jump()
{
    if (player_on_ground == TRUE)
    {
        player_vy = -10;
        player_on_ground = FALSE;
    }
}

void PLAYER_update()
{
    player_on_ground = FALSE;

    player_vy += 1;
    player_y += player_vy;

    if (player_y >= 184)
    {
        player_y = 184;
        player_vy = 0;
        player_on_ground = TRUE;
    }

    SPR_setPosition(player, player_x, player_y);
}

void JOY_handler(u16 joy, u16 changed, u16 state)
{
    if (joy == JOY_1)
    {
        if ((changed & state) & BUTTON_A)
        {
            PLAYER_try_jump(); 
        }
    }
}