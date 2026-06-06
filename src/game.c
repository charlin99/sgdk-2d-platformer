#include "game.h"
#include "player.h"
#include "enemy.h"
#include "resources.h"
#include "resources_def.h"
#include "cannon.h"

#define MARGIN 10

static GameState currentState;
Map *bga;

u16 camera_x = 0;

extern u8 player_lives;
extern u16 player_x;
extern u16 player_y;

u16 game_time_seconds = HUD_TIMER;
u8 game_frame_counter = 0;
u16 score = 0;

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
static void update_camera();

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

    PAL_setColor(31, RGB24_TO_VDPCOLOR(0xFFFFFF));

    
    XGM_setPCM(SFX_JUMP_ID, sfx_jump, sizeof(sfx_jump));
    XGM_setPCM(SFX_WALK_ID, sfx_walk, sizeof(sfx_walk));
    XGM_setPCM(SFX_HURT_ID, sfx_hurt, sizeof(sfx_hurt));
    XGM_setPCM(SFX_CANNON_ID, sfx_cannon, sizeof(sfx_cannon));

    PLAYER_init();
    ENEMY_init();
    CANNON_init();

    ENEMY_add(60, 192);
    ENEMY_add(120, 192);
    ENEMY_add(200, 192);

    GAME_update_hud();

    JOY_setEventHandler(gameplay_handle_joy);
}

void gameplay_update()
{
    if (currentState != STATE_PLAYING) return;

    PLAYER_handle_input();
    PLAYER_update();

    if (currentState != STATE_PLAYING) return;

    PLAYER_update_anim();

    check_room_transition();

    check_enemy_collisions();

    ENEMY_update_all();
    CANNON_update();

    game_frame_counter++;
    if (game_frame_counter >= 60)
    {
        game_frame_counter = 0;

        if (game_time_seconds > 0)
        {
            game_time_seconds--;
            
            
            GAME_update_hud(); 
        }
        else
        {
            player_health = 1;
            PLAYER_die();
            return;
        }
    }

    //update_camera();
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
        
    if (currentState == STATE_PLAYING || currentState == STATE_TITLE)
    {
        SPR_update();
    }
    
    SYS_doVBlankProcess();
    }

}

static void check_room_transition()
{
    if (currentState != STATE_PLAYING) return;

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
            if (currentState != STATE_PLAYING) break;

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
                    score += 10;
                    GAME_update_hud();
                }
                else
                {
                    if (player_hurt_timer == 0 && player_invincible_timer == 0)
                    {
                        if (player_health > 1)
                        {
                            player_health--; 
                            PLAYER_take_damage(enemies[i].x);
                        }
                        else
                        {
                            PLAYER_die(); 
                            return;
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

    currentState = STATE_GAMEOVER;

    if (bga != NULL)
    {
        MAP_release(bga);
        bga = NULL;
    }
    
    camera_x = 0;
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    SPR_end(); 

    VDP_clearTextArea(0, 0, 64, 32);

    gameover_init();
}

static void gameover_init()
{
    PAL_setPalette(PAL0, game_over_screen.palette->data, DMA);
    
    VDP_drawImageEx(BG_A, &game_over_screen, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX), 0, 0, FALSE, TRUE);
}

static void gameover_update()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_START)
    {
        SYS_hardReset(); 
    }
}

void GAME_update_hud()
{
    char vidas_texto[12];
    char tempo_texto[12];
    char score_texto[16];

    u8 vidas_extras = 0;
    if (player_lives > 0)
    {
        vidas_extras = player_lives - 1;
    }

    //sprintf(vidas_texto, "LIVES: %d", vidas_extras);
    sprintf(tempo_texto, "TIMER: %d ", game_time_seconds);
    sprintf(score_texto, "SCORE: %d", score);

    //VDP_drawTextEx(BG_B, vidas_texto, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 1, 0, DMA);
    VDP_drawTextEx(BG_B, score_texto, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 1, 1, DMA);
    VDP_drawTextEx(BG_B, tempo_texto, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 29, 1, DMA);
}

static void update_camera()
{
    s16 target_x = player_x - (320 / 2) + (PLAYER_WIDTH / 2);

    if (target_x < 0) target_x = 0;
    if (target_x > (level_map.w * 128) - 320) target_x = (level_map.w * 128) - 320;

    camera_x = target_x;

    MAP_scrollTo(bga, camera_x, 0);
}