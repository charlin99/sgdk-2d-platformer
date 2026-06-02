#include "resources_def.h"
#include "player.h"
#include <resources.h>
#include <game.h>

Sprite *player;
u16 player_x = 30;
u16 player_y = 32;
fix16 player_vy = FIX16(0);
bool player_on_ground = FALSE;
u8 player_jumps = 0;
u16 player_current_anim = -1;
u8 walk_sfx_timer = 0;
u8 player_hurt_timer = 0;
u8 player_invincible_timer = 0;

u8 player_lives = 3;
u8 player_health = 2;
u16 player_spawn_x = 30;
u16 player_spawn_y = 32;

static s8 knockback_direction = 0;

// --- Funções do Jogador ---
void PLAYER_init()
{
    player = SPR_addSprite(&player_sprite, player_x, player_y, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}

bool is_hard_solid_at(u16 x, u16 y)
{
    u16 tile = MAP_getTile(bga, x / 8, y / 8) & TILE_INDEX_MASK;
    return (tile >= 1 && tile != TILE_INDEX_PLATFORM && tile != TILE_INDEX_SPIKE); 
}

bool is_platform_at(u16 x, u16 y)
{
    u16 tile = MAP_getTile(bga, x / 8, y / 8) & TILE_INDEX_MASK;
    return (tile == TILE_INDEX_PLATFORM);
}

bool is_spike_at(u16 x, u16 y)
{
    u16 tile = MAP_getTile(bga, x / 8, y / 8) & TILE_INDEX_MASK;
    return (tile == TILE_INDEX_SPIKE);
}

void PLAYER_handle_input()
{
    if (player_hurt_timer > 0) return;

    u16 value = JOY_readJoypad(JOY_1);

    u16 h_speed = (value & BUTTON_B) ? PLAYER_RUN_SPEED : PLAYER_H_SPEED;

    u16 check_y_top = player_y + 2;
    u16 check_y_center = player_y + (PLAYER_HEIGHT / 2);
    u16 check_y_bottom = player_y + PLAYER_HEIGHT - 3;

    if (value & BUTTON_LEFT)
    {
        u16 next_x = player_x - h_speed;
        if (!is_hard_solid_at(next_x, check_y_top) && !is_hard_solid_at(next_x, check_y_center) && !is_hard_solid_at(next_x, check_y_bottom))
        {
            player_x -= h_speed;
        }
        SPR_setHFlip(player, TRUE);
    }
    else if (value & BUTTON_RIGHT)
    {
        u16 next_x = player_x + PLAYER_WIDTH - 1 + h_speed;
        if (!is_hard_solid_at(next_x, check_y_top) && !is_hard_solid_at(next_x, check_y_center) && !is_hard_solid_at(next_x, check_y_bottom))
        {
            player_x += h_speed;
        }
        SPR_setHFlip(player, FALSE);
    }
}

void PLAYER_update()
{
    if (player_hurt_timer > 0)
    {
        player_hurt_timer--;
        u16 check_y_top = player_y + 2;
        u16 check_y_center = player_y + (PLAYER_HEIGHT / 2);
        u16 check_y_bottom = player_y + PLAYER_HEIGHT - 3;
        if (knockback_direction < 0) // Sendo arrastado para a esquerda
        {
            u16 next_x = player_x - 2; // Arrastando 2 pixels por frame
            if (!is_hard_solid_at(next_x, check_y_top) && 
                !is_hard_solid_at(next_x, check_y_center) && 
                !is_hard_solid_at(next_x, check_y_bottom))
            {
                player_x -= 2;
            }
        }
        else if (knockback_direction > 0) // Sendo arrastado para a direita
        {
            u16 next_x = player_x + PLAYER_WIDTH - 1 + 2;
            if (!is_hard_solid_at(next_x, check_y_top) && 
                !is_hard_solid_at(next_x, check_y_center) && 
                !is_hard_solid_at(next_x, check_y_bottom))
            {
                player_x += 2;
            }
        }

        player_vy += GRAVITY; // Garante que o Y fique travado reto durante o arrasto
        if (player_vy > MAX_FALL_SPEED) player_vy = MAX_FALL_SPEED;
    }
    else
    {
        player_vy += GRAVITY;
        if (player_vy > MAX_FALL_SPEED) player_vy = MAX_FALL_SPEED;
    }

    // --- PASSO 2: MOVIMENTO E COLISÃO VERTICAL ---
    s16 move_y = F16_toInt(player_vy);

    u16 check_x_left = player_x + 2;
    u16 check_x_center = player_x + (PLAYER_WIDTH / 2);
    u16 check_x_right = player_x + PLAYER_WIDTH - 2;

    if (move_y > 0) // Caindo
    {
        for (int i = 0; i < move_y; i++)
        {
            u16 old_feet_y = player_y + PLAYER_HEIGHT;

            player_y++;
            u16 feet_y = player_y + PLAYER_HEIGHT;
            
            if (is_hard_solid_at(check_x_left, feet_y - 1) || 
                is_hard_solid_at(check_x_center, feet_y - 1) || 
                is_hard_solid_at(check_x_right, feet_y - 1))
            {
                player_y--;
                player_vy = FIX16(0);
                break;
            }
            else if (is_platform_at(check_x_left, feet_y - 1) || 
                     is_platform_at(check_x_center, feet_y - 1) || 
                     is_platform_at(check_x_right, feet_y - 1))
            {
                u16 tile_top_y = ((feet_y - 1) / 8) * 8;

                if (player_vy > 0 && old_feet_y <= tile_top_y && !(JOY_readJoypad(JOY_1) & BUTTON_DOWN))
                {
                    player_y = tile_top_y - PLAYER_HEIGHT;
                    player_vy = FIX16(0);
                    break;
                }
            }
        }
    }
    else if (move_y < 0) // Subindo
    {
        s16 abs_move_y = -move_y;
        for (int i = 0; i < abs_move_y; i++)
        {
            player_y--;
            u16 head_y = player_y;
            
            if (is_hard_solid_at(check_x_left, head_y) || 
                is_hard_solid_at(check_x_center, head_y) || 
                
                is_hard_solid_at(check_x_right, head_y))
            {
                player_y++;
                player_vy = FIX16(0);
                break;
            }
        }
    }

    // --- PASSO 3: VERIFICAÇÃO DE ESTADO ON_GROUND ---
    u16 feet_y_check = player_y + PLAYER_HEIGHT;

    u16 spike_y_check = feet_y_check - 4;
    
    bool on_hard_floor = is_hard_solid_at(check_x_left, feet_y_check) || 
                         is_hard_solid_at(check_x_center, feet_y_check) || 
                         is_hard_solid_at(check_x_right, feet_y_check);
                         
    bool on_platform = is_platform_at(check_x_left, feet_y_check) || 
                       is_platform_at(check_x_center, feet_y_check) || 
                       is_platform_at(check_x_right, feet_y_check);

    bool on_spike = is_spike_at(check_x_left, spike_y_check) || 
                    is_spike_at(check_x_center, spike_y_check) || 
                    is_spike_at(check_x_right, spike_y_check) ||
                    is_spike_at(check_x_left, feet_y_check - 1) || // Mantém a base por segurança
                    is_spike_at(check_x_right, feet_y_check - 1);

    if (on_spike)
    {
        if (player_hurt_timer == 0 && player_invincible_timer == 0)
        {
            if (player_health > 1)
            {
                player_health--;
                
                PLAYER_take_damage(player_x + 8); 
            }
            else
            {
                PLAYER_die();
            }
        }
    }

    if ((player_vy >= 0) && (on_hard_floor || on_platform))
    {
        if (!player_on_ground)
        {
            player_on_ground = TRUE;
            player_jumps = 0;
        }
    }
    else
    {
        if (player_hurt_timer == 0)
        {
            player_on_ground = FALSE;
            if (player_jumps == 0)
            {
                player_jumps = 1;
            }
        }
    }

    if (player_invincible_timer > 0)
    {
        player_invincible_timer--;

        if (player_invincible_timer & 1)
        {
            SPR_setVisibility(player, HIDDEN);
        }
        else
        {
            SPR_setVisibility(player, VISIBLE);
        }
    }
    else
    {
        SPR_setVisibility(player, VISIBLE);
    }
    

    // --- PASSO 4: ATUALIZAR O SPRITE NA TELA (Respeitando o sistema de salas) ---
    SPR_setPosition(player, player_x - camera_x, player_y);

    
    if (player_y > SCREEN_HEIGHT + 16)
    {
        PLAYER_die();
    }
}

void PLAYER_try_jump()
{
    if (player_jumps < PLAYER_JUMP_COUNT)
    {
        XGM_startPlayPCM(SFX_JUMP_ID, 15, SOUND_PCM_CH2);
        player_vy = PLAYER_JUMP_FORCE;
        player_on_ground = FALSE;
        player_jumps++;
    }
}

void PLAYER_update_anim()
{
    u16 new_anim = player_current_anim;

    if (walk_sfx_timer > 0)
    {
        walk_sfx_timer--;
    }

    if (player_on_ground)
    {
        u16 value = JOY_readJoypad(JOY_1);
        if ((value & BUTTON_LEFT) || (value & BUTTON_RIGHT))
        {
            new_anim = ANIM_WALK;
            if ((XGM_isPlayingPCM(SOUND_PCM_CH3) == 0)  && (walk_sfx_timer == 0))
            {
                XGM_startPlayPCM(SFX_WALK_ID, 10, SOUND_PCM_CH3);
                
                walk_sfx_timer = 30;
            }
        }
        else
        {
            new_anim = ANIM_IDLE;
            XGM_stopPlayPCM(SOUND_PCM_CH3);
        }
    }
    else
    {
        if (player_vy < 0)
        {
            new_anim = ANIM_JUMP;
        }
        else if (player_vy > 60)
        {
            new_anim = ANIM_FALL;
        }
    }

    if (new_anim != player_current_anim)
    {
        player_current_anim = new_anim;
        SPR_setAnim(player, player_current_anim);
    }
}

void PLAYER_handle_joy(u16 changed, u16 state)
{
    if (changed & state & BUTTON_A)
    {
        if (state & BUTTON_DOWN)
        {
            if (player_on_ground)
            {
                u16 check_x_left = player_x + 2;
                u16 check_x_center = player_x + (PLAYER_WIDTH / 2);
                u16 check_x_right = player_x + PLAYER_WIDTH - 2;
                u16 feet_y = player_y + PLAYER_HEIGHT;
                
                if (is_platform_at(check_x_left, feet_y) || 
                    is_platform_at(check_x_center, feet_y) || 
                    is_platform_at(check_x_right, feet_y))
                {
                    player_y += 2;
                    player_on_ground = FALSE;
                }
            }
        }
        else
        {
            PLAYER_try_jump();
        }
    }
}

void PLAYER_take_damage(s16 enemy_x)
{
    player_hurt_timer = 20;
    player_invincible_timer = 120;
    player_on_ground = FALSE;

    player_vy = FIX16(-1.8);

    if (player_x + (PLAYER_WIDTH / 2) < enemy_x)
    {
        knockback_direction = -1;
        SPR_setHFlip(player, FALSE);
    }
    else
    {
        knockback_direction = 1;
        SPR_setHFlip(player, TRUE);
    }
}

void PLAYER_die()
{
    if (player_lives > 0)
    {
        player_lives--;
    }

    GAME_update_hud();

    if (player_lives > 0)
    {
        player_x = player_spawn_x;
        player_y = player_spawn_y;
        
        player_vy = FIX16(0);
        player_jumps = 0;
        player_hurt_timer = 0;
        player_on_ground = FALSE;
        player_health = 2;
    }
    else
    {
        GAME_trigger_gameover();
    }
}