#include "game.h"
#include "player.h"
#include "enemy.h"
#include "resources.h"
#include "resources_def.h"

#define SCREEN_WIDTH 320
#define MARGIN 10

static GameState currentState;
Map *bga;

u16 camera_x = 0; 
extern u16 player_x;

static void title_init();
static void title_update();
static void gameplay_init();
static void gameplay_update();
static void gameplay_handle_joy(u16 joy, u16 changed, u16 state);
static void check_room_transition();

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
    ENEMY_add(120, 184);

    JOY_setEventHandler(gameplay_handle_joy);
}

void gameplay_update()
{
    PLAYER_handle_input();
    PLAYER_update();
    PLAYER_update_anim();

    check_room_transition();

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
    // Verifica se o pé ou centro do player passou o limite da câmera atual
    if (player_x >= camera_x + SCREEN_WIDTH)
    {
        if (camera_x < 960) // Para mapa de 1280px (4 telas)
        {
            camera_x += SCREEN_WIDTH;
            // Posiciona o player no início da nova tela (pixel 0 da nova sala)
            player_x = camera_x + 4; 
            MAP_scrollTo(bga, camera_x, 0);
        }
    }
    else if (player_x < camera_x)
    {
        if (camera_x >= SCREEN_WIDTH)
        {
            camera_x -= SCREEN_WIDTH;
            // Posiciona o player no final da tela anterior
            player_x = camera_x + SCREEN_WIDTH - 20; 
            MAP_scrollTo(bga, camera_x, 0);
        }
    }
}