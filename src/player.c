#include "resources_def.h"
#include "player.h"
#include <resources.h>

// --- Constantes ---
#define PLAYER_WIDTH        16
#define PLAYER_HEIGHT       16
#define PLAYER_H_SPEED      1           // Velocidade horizontal em pixels por frame
#define PLAYER_JUMP_FORCE   FIX16(-3.0)
#define GRAVITY             FIX16(0.2)
#define MAX_FALL_SPEED      FIX16(6.0)

// --- Constantes de Som ---
#define SFX_JUMP_ID     64
#define SFX_WALK_ID     65

// --- Variáveis Globais ---
Sprite *player;
u16 player_x = 30;
u16 player_y = 32;
fix16 player_vy = FIX16(0);
bool player_on_ground = FALSE;
u16 player_current_anim = -1;
u8 walk_sfx_timer = 0;

// --- Funções Auxiliares ---
bool is_tile_solid(u16 tile_index)
{
    return (tile_index >= 1);
}

// Única função de checagem: Verifica se um pixel específico no mapa é sólido.
bool is_solid_at(u16 x, u16 y)
{
    return is_tile_solid(MAP_getTile(bga, x / 8, y / 8) & TILE_INDEX_MASK);
}

// --- Funções do Jogador ---
void PLAYER_init()
{
    player = SPR_addSprite(&player_sprite, player_x, player_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}

void PLAYER_handle_input()
{
    u16 value = JOY_readJoypad(JOY_1);

    u16 check_y_top = player_y + 2;
    u16 check_y_bottom = player_y + PLAYER_HEIGHT - 2;

    if (value & BUTTON_LEFT)
    {
        // Checa a próxima posição X ANTES de mover
        u16 next_x = player_x - PLAYER_H_SPEED;
        if (!is_solid_at(next_x, check_y_top) && !is_solid_at(next_x, check_y_bottom))
        {
            player_x -= PLAYER_H_SPEED;
        }
        SPR_setHFlip(player, TRUE);
    }
    else if (value & BUTTON_RIGHT)
    {
        // Checa a próxima posição da borda direita do sprite
        u16 next_x = player_x + PLAYER_WIDTH - 1 + PLAYER_H_SPEED;
        if (!is_solid_at(next_x, check_y_top) && !is_solid_at(next_x, check_y_bottom))
        {
            player_x += PLAYER_H_SPEED;
        }
        SPR_setHFlip(player, FALSE);
    }
}

void PLAYER_update()
{
    // --- PASSO 1: ATUALIZAR VELOCIDADE VERTICAL ---
    player_vy += GRAVITY;
    if (player_vy > MAX_FALL_SPEED) player_vy = MAX_FALL_SPEED;

    // --- PASSO 2: MOVIMENTO E COLISÃO VERTICAL ---
    s16 move_y = F16_toInt(player_vy);

    // Variáveis de checagem calculadas apenas uma vez
    u16 check_x_left = player_x + 2;
    u16 check_x_right = player_x + PLAYER_WIDTH - 2;

    if (move_y > 0) // Caindo
    {
        for (int i = 0; i < move_y; i++)
        {
            player_y++;
            u16 feet_y = player_y + PLAYER_HEIGHT - 1;
            if (is_solid_at(check_x_left, feet_y) || is_solid_at(check_x_right, feet_y))
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
            if (is_solid_at(check_x_left, head_y) || is_solid_at(check_x_right, head_y))
            {
                player_y++;
                player_vy = FIX16(0);
                break;
            }
        }
    }

    // --- PASSO 3: VERIFICAÇÃO DE ESTADO ON_GROUND ---
    u16 feet_y_check = player_y + PLAYER_HEIGHT;
    if ((player_vy >= 0) && (is_solid_at(check_x_left, feet_y_check) || is_solid_at(check_x_right, feet_y_check)))
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
        XGM_startPlayPCM(SFX_JUMP_ID, 15, SOUND_PCM_CH2);
        player_vy = PLAYER_JUMP_FORCE;
        player_on_ground = FALSE;
    }
}

void PLAYER_update_anim()
{
    // Decide qual animação deveria estar tocando
    u16 new_anim = player_current_anim;

    if (walk_sfx_timer > 0)
    {
        walk_sfx_timer--;
    }

    if (player_on_ground)
    {
        // Se está no chão, verifica se o jogador está pressionando para os lados
        u16 value = JOY_readJoypad(JOY_1);
        if ((value & BUTTON_LEFT) || (value & BUTTON_RIGHT))
        {
            new_anim = ANIM_WALK; // Animação de andar
            if ((XGM_isPlayingPCM(SOUND_PCM_CH3) == 0)  && (walk_sfx_timer == 0))
            {
                XGM_startPlayPCM(SFX_WALK_ID, 10, SOUND_PCM_CH3);
                
                walk_sfx_timer = 30;
            }
        }
        else
        {
            new_anim = ANIM_IDLE; // Animação de parado
            XGM_stopPlayPCM(SOUND_PCM_CH3);
        }
    }
    else // Jogador no ar
    {
        if (player_vy < 0)
        {
            // Se está subindo
            new_anim = ANIM_JUMP;
        }
        else if (player_vy > 60)
        {
            // Se está caindo
            new_anim = ANIM_FALL;
        }
    }

    // Se a animação que deveria tocar é diferente da que está tocando atualiza para a que deveria ser
    if (new_anim != player_current_anim)
    {
        player_current_anim = new_anim;
        SPR_setAnim(player, player_current_anim);
    }
}

void PLAYER_handle_joy(u16 changed, u16 state)
{
    // Se o botão A foi pressionado...
    if (changed & state & BUTTON_A)
    {
        // ...e Baixo está segurado...
        if (state & BUTTON_DOWN)
        {
            // ...lógica de descer da plataforma...
            if (player_on_ground)
            {
                u16 check_x = player_x + (PLAYER_WIDTH / 2);
                u16 feet_y = player_y + PLAYER_HEIGHT;
                u16 tile_below = MAP_getTile(bga, check_x / 8, feet_y / 8) & TILE_INDEX_MASK;
                if (tile_below == TILE_INDEX_PLATFORM)
                {
                    player_y += 2;
                    player_on_ground = FALSE;
                }
            }
        }
        // ...senão, é um pulo normal.
        else
        {
            PLAYER_try_jump();
        }
    }
}