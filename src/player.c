#include "player.h"
#include <resources.h>

// --- Constantes ---
#define PLAYER_WIDTH        16
#define PLAYER_HEIGHT       16
#define PLAYER_JUMP_FORCE   FIX16(-3.0)
#define GRAVITY             FIX16(0.2)
#define MAX_FALL_SPEED      FIX16(6.0)
#define PLAYER_SPEED        1

// --- Variáveis globais ---
Sprite *player;
u16 player_x = 30;
u16 player_y = 32;
fix16 player_vy = FIX16(0);
bool player_on_ground = FALSE;

// --- Funções auxiliares ---
bool is_tile_solid(u16 tile_index)
{
    return (tile_index >= 1);
}

bool is_path_clear_H(u16 x_coord)
{
    u16 body_y_top = player_y + 2;
    u16 body_y_bottom = player_y + (PLAYER_HEIGHT - 2);

    u16 upper_tile = MAP_getTile(bga, x_coord / 8, body_y_top / 8) & TILE_INDEX_MASK;
    u16 lower_tile = MAP_getTile(bga, x_coord / 8, body_y_bottom / 8) & TILE_INDEX_MASK;

    return !is_tile_solid(upper_tile) && !is_tile_solid(lower_tile);
}

bool check_vertical_collision(u16 y_coord)
{
    u16 check_x_left = player_x + 2;
    u16 check_x_right = player_x + (PLAYER_WIDTH - 2);

    u16 tile_left  = MAP_getTile(bga, check_x_left / 8, y_coord / 8) & TILE_INDEX_MASK;
    u16 tile_right = MAP_getTile(bga, check_x_right / 8, y_coord / 8) & TILE_INDEX_MASK;

    return is_tile_solid(tile_left) || is_tile_solid(tile_right);
}

// --- Funções do jogador ---
void PLAYER_init()
{
    player = SPR_addSprite(&player_sprite, player_x, player_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}

void PLAYER_handle_input()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_LEFT)
    {
        if (is_path_clear_H(player_x - PLAYER_SPEED))
        {
            player_x -= PLAYER_SPEED;
        }
        SPR_setHFlip(player, TRUE);
    }
    else if (value & BUTTON_RIGHT)
    {
        if (is_path_clear_H(player_x + PLAYER_WIDTH))
        {
            player_x += PLAYER_SPEED;
        }
        SPR_setHFlip(player, FALSE);
    }
}

void PLAYER_update()
{
    // --- PASSO 1: APLICAR GRAVIDADE ---
    player_vy += GRAVITY;

    if (player_vy > MAX_FALL_SPEED)
    {
        player_vy = MAX_FALL_SPEED;
    }

    // --- PASSO 2: MOVIMENTAÇÃO E COLISÃO VERTICAL ---
    s16 move_y = F16_toInt(player_vy);

    if (move_y > 0) // Caindo
    {
        for (int i = 0; i < move_y; i++)
        {
            player_y++;
            u16 feet_y = player_y + PLAYER_HEIGHT - 1; // Checa o último pixel do sprite
            
            if (check_vertical_collision(feet_y))
            {
                player_y--;
                player_vy = FIX16(0);
                break; 
            }
        }
    }
    else if (move_y < 0) // Subindo
    {
        for (int i = 0; i > move_y; i--)
        {
            player_y--;
            u16 head_y = player_y;

            if (check_vertical_collision(head_y))
            {
                player_y++;
                player_vy = FIX16(0);
                break;
            }
        }
    }
    
    // --- PASSO 3: VERIFICAÇÃO DE ESTADO ON_GROUND ---
    u16 feet_y_check = player_y + PLAYER_HEIGHT; // O tile logo abaixo dos pés
    
    if ((player_vy >= 0) && check_vertical_collision(feet_y_check))
    {
        player_on_ground = TRUE;
    }
    else
    {
        player_on_ground = FALSE;
    }

    // --- PASSO 4: ATUALIZAR O SPRITE NA TELA ---
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