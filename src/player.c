#include "player.h"
#include <resources.h>

#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 16
#define PLAYER_JUMP_FORCE -7
#define GRAVITY 1

Sprite *player;
u16 player_x = 30;
u16 player_y = 32;
s16 player_vy = 0;
bool player_on_ground = FALSE;

bool is_tile_solid(u16 tile_index)
{
    return (tile_index >= 1);
}

void PLAYER_init()
{
    player = SPR_addSprite(&player_sprite, player_x, player_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}

void PLAYER_handle_input()
{
    u16 value = JOY_readJoypad(JOY_1);

    u16 body_y_top = player_y + 2;
    u16 body_y_bottom = player_y + (PLAYER_HEIGHT - 2);

    if (value & BUTTON_LEFT)
    {
        u16 left_x = player_x - 1;
        u16 upper_tile = MAP_getTile(bga, left_x / 8, body_y_top / 8) & TILE_INDEX_MASK;
        u16 lower_tile = MAP_getTile(bga, left_x / 8, body_y_bottom / 8) & TILE_INDEX_MASK;

        if (!is_tile_solid(upper_tile) && !is_tile_solid(lower_tile))
        {
            player_x -= 1;
        }
        SPR_setHFlip(player, TRUE);
    }
    else if (value & BUTTON_RIGHT)
    {
        u16 right_x = player_x + PLAYER_WIDTH;
        u16 upper_tile = MAP_getTile(bga, right_x / 8, body_y_top / 8) & TILE_INDEX_MASK;
        u16 lower_tile = MAP_getTile(bga, right_x / 8, body_y_bottom / 8) & TILE_INDEX_MASK;

        if (!is_tile_solid(upper_tile) && !is_tile_solid(lower_tile))
        {
            player_x += 1;
        }
        SPR_setHFlip(player, FALSE);
    }
}

void PLAYER_update()
{
    // --- PASSO 1: APLICAR GRAVIDADE ---
    player_vy += GRAVITY;

    // --- PASSO 2: LIDAR COM O MOVIMENTO E COLISÃO EIXO Y ---
    player_on_ground = FALSE;

    if (player_vy > 0) // Caindo
    {
        // Loop que move 1 pixel de cada vez, até o máximo da velocidade vertical
        for (int i = 0; i < player_vy; i++)
        {
            player_y++; // Move 1 pixel para baixo

            // Verifica a colisão com o chão APÓS mover 1 pixel
            u16 check_x_left = player_x + 2;
            u16 check_x_right = player_x + (PLAYER_WIDTH - 2);
            u16 feet_y = player_y + PLAYER_HEIGHT;
            u16 tile_left  = MAP_getTile(bga, check_x_left / 8, feet_y / 8) & TILE_INDEX_MASK;
            u16 tile_right = MAP_getTile(bga, check_x_right / 8, feet_y / 8) & TILE_INDEX_MASK;

            if (is_tile_solid(tile_left) || is_tile_solid(tile_right))
            {
                player_y--; // Volta 1 pixel para a posição de contato
                player_vy = 0;
                player_on_ground = TRUE;
                break; // Sai do loop de movimento, pois já colidiu
            }
        }
    }
    else if (player_vy < 0) // Subindo
    {
        // Loop para movimento para cima, -1 pixel de cada vez
        for (int i = 0; i > player_vy; i--)
        {
            player_y--; // Move 1 pixel para cima

            // Verifica a colisão com o teto
            u16 check_x_left = player_x + 2;
            u16 check_x_right = player_x + (PLAYER_WIDTH - 2);
            u16 head_y = player_y;
            u16 tile_left  = MAP_getTile(bga, check_x_left / 8, head_y / 8) & TILE_INDEX_MASK;
            u16 tile_right = MAP_getTile(bga, check_x_right / 8, head_y / 8) & TILE_INDEX_MASK;
        
            if (is_tile_solid(tile_left) || is_tile_solid(tile_right))
            {
                player_y++; // Volta 1 pixel para a posição de contato
                player_vy = 0;
                break; // Sai do loop
            }
        }
    }

    // --- PASSO 3: ATUALIZAR O SPRITE NA TELA ---
    SPR_setPosition(player, player_x, player_y);
}

void PLAYER_try_jump()
{
    if (player_on_ground)
    {
        player_vy = PLAYER_JUMP_FORCE;
        player_on_ground = FALSE;
    }
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