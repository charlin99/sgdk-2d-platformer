#include "game.h"
#include "player.h"
#include "enemy.h"
#include "resources.h"
#include "resources_def.h"

#define MARGIN 10

static GameState currentState;
Map *bga;

u16 camera_x = 0;

extern u16 player_x;
extern u16 player_y;
extern bool player_on_ground;

extern Enemy enemies[MAX_ENEMIES];

static void title_init();
static void title_update();
static void gameplay_init();
static void gameplay_update();
static void gameplay_handle_joy(u16 joy, u16 changed, u16 state);
static void check_room_transition();
static void goToGameplay();
static void check_enemy_collisions();
static void gameover_init();
static void gameover_update();

static void goToGameplay()
{
    JOY_setEventHandler(NULL);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    currentState = STATE_PLAYING;
    gameplay_init();
}

static void title_init()
{
    JOY_setEventHandler(NULL);
    PAL_setPalette(PAL0, title_screen.palette->data, DMA);
    VDP_drawImageEx(BG_A, &title_screen, TILE_ATTR(PAL0, FALSE, FALSE, FALSE), 0, 0, FALSE, TRUE);
}

static void title_update()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_START)
    {
        goToGameplay();
    }
}

static void gameplay_init()
{
    PAL_setPalette(PAL0, level_palette.data, DMA);
    VDP_loadTileSet(&level_tileset, TILE_USER_INDEX, DMA);

    bga = MAP_create(&level_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX));
    MAP_scrollTo(bga, 0, 0);

    XGM_setPCM(SFX_JUMP_ID, sfx_jump, sizeof(sfx_jump));
    XGM_setPCM(SFX_WALK_ID, sfx_walk, sizeof(sfx_walk));

    PLAYER_init();

    ENEMY_init();
    ENEMY_add(60, 184);
    ENEMY_add(120, 184);
    ENEMY_add(200, 184);

    JOY_setEventHandler(gameplay_handle_joy);
}

void gameplay_update()
{
    PLAYER_handle_input();
    PLAYER_update();
    PLAYER_update_anim();

    check_room_transition();

    check_enemy_collisions();

    ENEMY_update_all();
}

void gameplay_handle_joy(u16 joy, u16 changed, u16 state)
{
    if (joy == JOY_1)
    {
        PLAYER_handle_joy(changed, state);
    }
}

void GAME_start()
{
    currentState = STATE_TITLE;
    title_init();

    while(1)
    {
        JOY_update();

        switch (currentState)
        {
            case STATE_TITLE:
                title_update();
                break;

            case STATE_PLAYING:
                gameplay_update();
                break;

            case STATE_GAMEOVER:
                gameover_update();
                break;

            default:
                break;
        }
        
    SPR_update();
    //VDP_waitVSync();
    SYS_doVBlankProcess();
    }

}

static void check_room_transition()
{
    u16 target_x = camera_x;
    bool transitioning = FALSE;

    if (player_x >= camera_x + SCREEN_WIDTH)
    {
        if (camera_x < 960)
        {
            target_x = camera_x + SCREEN_WIDTH;
            transitioning = TRUE;
        }
    }
    else if (player_x < camera_x)
    {
        if (camera_x >= SCREEN_WIDTH)
        {
            target_x = camera_x - SCREEN_WIDTH;
            transitioning = TRUE;
        }
    }

    if (transitioning)
    {
        if (camera_x < target_x) 
            player_x = target_x;
        else 
            player_x = target_x + SCREEN_WIDTH - 16;

        while (camera_x != target_x)
        {
            if (camera_x < target_x) {
                camera_x += 10;
            } else {
                camera_x -= 10;
            }

            MAP_scrollTo(bga, camera_x, 0);
            SPR_setPosition(player, player_x - camera_x, player_y);

            for(int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    SPR_setPosition(enemies[i].sprite, enemies[i].x - camera_x, enemies[i].y);
                }
            }

            SPR_update();
            SYS_doVBlankProcess();
        }

        if (player_x >= target_x + SCREEN_WIDTH) player_x = target_x + SCREEN_WIDTH - 16;
        if (player_x < target_x) player_x = target_x;
    }
}

static void check_enemy_collisions()
{
    u16 p_left = player_x + 2;
    u16 p_right = player_x + PLAYER_WIDTH - 2;
    u16 p_top = player_y + 2;
    u16 p_bottom = player_y + PLAYER_HEIGHT;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            u16 e_left = enemies[i].x + 2;
            u16 e_right = enemies[i].x + 14;
            u16 e_top = enemies[i].y + 4;
            u16 e_bottom = enemies[i].y + 16;

            if (p_right > e_left && p_left < e_right && p_bottom > e_top && p_top < e_bottom)
            {
                u16 e_middle_y = e_top + ((e_bottom - e_top) / 2);

                if (!player_on_ground && (player_vy > FIX16(0)) && (p_bottom <= e_middle_y + 4))
                {
                    ENEMY_remove(i);
                    
                    player_vy = FIX16(-2.5); 
                    player_jumps = 1;
                }
                else
                {
                    if (player_hurt_timer == 0)
                    {
                        PLAYER_take_damage(enemies[i].x);
                        if (player_health > 1)
                        {
                            player_health--; 
                        }
                        else
                        {
                            PLAYER_die(); 
                        }
                    }
                }
            }
        }
    }
}

void GAME_trigger_gameover()
{
    JOY_setEventHandler(NULL);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    SPR_end(); 

    currentState = STATE_GAMEOVER;
    gameover_init();
}

static void gameover_init()
{
    PAL_setPalette(PAL0, game_over_screen.palette->data, DMA);
    
    VDP_drawImageEx(BG_A, &game_over_screen, TILE_ATTR(PAL0, FALSE, FALSE, FALSE), 0, 0, FALSE, TRUE);
}

static void gameover_update()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_START)
    {
        SYS_hardReset(); 
    }
}