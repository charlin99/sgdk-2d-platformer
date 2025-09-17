#include "enemy.h"
#include "resources.h" // Necessário para acessar a definição do sprite 'slime'

#define MAX_ENEMIES     8
#define ENEMY_SPEED     1
#define ENEMY_MOVE_DELAY 2

// Uma estrutura para guardar os dados de cada inimigo
typedef struct
{
    Sprite* sprite;
    s16 x;
    s16 y;
    s16 vel_x; // Velocidade horizontal
    s8 move_timer; // Temporizador para controlar a velocidade de movimento
    bool active;
} Enemy;

// Um array para guardar todos os inimigos do jogo
static Enemy enemies[MAX_ENEMIES];

// Prepara o sistema de inimigos, marcando todos como inativos
void ENEMY_init()
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = FALSE;
        enemies[i].sprite = NULL;
    }
}

// Procura um espaço vago no array e adiciona um novo inimigo
void ENEMY_add(u16 x, u16 y)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].x = x;
            enemies[i].y = y;
            enemies[i].vel_x = ENEMY_SPEED; // Começa andando para a direita
            enemies[i].active = TRUE;
            enemies[i].move_timer = ENEMY_MOVE_DELAY;

            // Adiciona o sprite do slime na tela
            enemies[i].sprite = SPR_addSprite(&slime, enemies[i].x, enemies[i].y, TILE_ATTR(PAL0, TRUE, FALSE, TRUE));
            // Inicia a animação de andar (animação 0, a única que definimos)
            SPR_setAnim(enemies[i].sprite, 0);
            
            return; // Sai da função após adicionar
        }
    }
}

// Atualiza a lógica e posição de todos os inimigos ativos
void ENEMY_update_all()
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            // --- LÓGICA DE MOVIMENTO ALTERADA ---
            enemies[i].move_timer--; // Decrementa o timer a cada quadro

            // Se o timer chegou a zero, move o inimigo e reinicia o timer
            if (enemies[i].move_timer <= 0)
            {
                enemies[i].move_timer = ENEMY_MOVE_DELAY; // Reinicia o timer
                enemies[i].x += enemies[i].vel_x; // Move o inimigo
            }

            // Lógica de patrulha
            if (enemies[i].x < 16) {
                enemies[i].vel_x = ENEMY_SPEED;
                SPR_setHFlip(enemies[i].sprite, TRUE);
            } else if (enemies[i].x > 288) {
                enemies[i].vel_x = -ENEMY_SPEED;
                SPR_setHFlip(enemies[i].sprite, FALSE);
            }

            // Atualiza a posição do sprite na tela
            SPR_setPosition(enemies[i].sprite, enemies[i].x, enemies[i].y);
        }
    }
}