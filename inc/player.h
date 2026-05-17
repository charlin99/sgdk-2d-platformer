#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <genesis.h>

// --- Constantes de Configuração do Personagem ---
#define PLAYER_WIDTH        16
#define PLAYER_HEIGHT       16
#define PLAYER_H_SPEED      1           // Pixels por frame
#define PLAYER_RUN_SPEED    2
#define PLAYER_JUMP_FORCE   FIX16(-3.0)
#define GRAVITY             FIX16(0.2)
#define MAX_FALL_SPEED      FIX16(6.0)
#define PLAYER_JUMP_COUNT   2

// --- Constantes de Áudio ---
#define SFX_JUMP_ID     64
#define SFX_WALK_ID     65

// --- Variáveis Globais (Expostas para outros arquivos) ---
extern Sprite *player;
extern u16 player_x;
extern u16 player_y;
extern fix16 player_vy;
extern bool player_on_ground;
extern u8 player_jumps;
extern u16 player_current_anim;
extern u8 walk_sfx_timer;
extern u8 player_hurt_timer;

// --- Mapas Externos ---
extern Map *bga;
extern u16 camera_x;

// --- Assinaturas das Funções ---
void PLAYER_init();
void PLAYER_handle_input();
void PLAYER_update();
void PLAYER_update_anim();
void PLAYER_try_jump();
void PLAYER_handle_joy(u16 changed, u16 state);
void PLAYER_take_damage(s16 enemy_x);

// --- Funções de Checagem de Bloco ---
bool is_hard_solid_at(u16 x, u16 y);
bool is_platform_at(u16 x, u16 y);

#endif // _PLAYER_H_