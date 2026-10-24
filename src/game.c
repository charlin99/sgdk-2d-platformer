#include "stage.h"
#include "game.h"
#include "player.h"
#include "enemy.h"
#include "resources.h"
#include "resources_def.h"
#include "cannon.h"
#include "checkpoint.h"

#define PAUSE_TEXT_X 17
#define PAUSE_TEXT_Y 14
#define SPLASH_DISPLAY_FRAMES 120
#define SPLASH_FADE_FRAMES 30
#define SPLASH_X 4
#define SPLASH_Y 2
#define CREDITS_DISPLAY_FRAMES 150
#define CREDITS_TEXT_X 8
#define CREDITS_NAME_X 16
#define CREDITS_TEXT_Y 12
#define STAGE_MESSAGE_SECONDS 2

static GameState currentState;
Map *bga;

static Sprite* hud_life_icon_sprite = NULL;

u16 camera_x = 0;

u16 game_time_seconds = HUD_TIMER;
u8 game_frame_counter = 0;
u16 score = 0;

static bool is_in_life_screen = FALSE;
static u16 life_screen_timer = 0;
static bool is_paused = FALSE;
static u16 splash_timer = 0;
static u16 credits_timer = 0;

extern Enemy enemies[MAX_ENEMIES];

static void title_init();
static void title_update();
static void splash_init();
static void splash_update();
static void credits_init();
static void credits_update();
static void gameplay_init();
static void gameplay_update();
static void gameplay_handle_joy(u16 joy, u16 changed, u16 state);
static void check_room_transition();
static void goToGameplay();
static void check_enemy_collisions();
static void gameover_init();
static void gameover_update();
static void respawn_player();
static void hud_init();
static u16 pending_stage = 0;
static u16 rejected_exit = 0;
// Reset invalidates all sprite pointers together; only bullets release individually.
static void clear_stage_sprites()
{
    CANNON_clear();
    ENEMY_init();
    PLAYER_clear_sprite();
    CHECKPOINT_clear_sprites();
    hud_life_icon_sprite = NULL;
    SPR_reset();
}
static void load_stage(u16 id, bool reveal)
{
    const StageDefinition *destination = STAGE_find(id);
    if (destination == NULL || destination->cannon_count > STAGE_MAX_CANNONS) return;
    JOY_setEventHandler(NULL);
    VDP_setWindowOff();
    VDP_setEnable(FALSE);
    clear_stage_sprites();
    if (bga != NULL) { MAP_release(bga); bga = NULL; }
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    current_stage = destination;
    pending_stage = 0;
    rejected_exit = 0;
    gameplay_init();
    if (currentState != STATE_GAMEOVER)
    {
        SPR_update();
        if (!reveal) PAL_setPalette(PAL0, palette_black, CPU);
    }
    SYS_doVBlankProcess();
    VDP_setEnable(TRUE);
}

static void hold_stage_message(u16 frames)
{
    // No gameplay or SPR_update here: physics, timer and sprite animations freeze.
    while (frames--) SYS_doVBlankProcess();
}

static void transition_stage()
{
    const u16 destination = pending_stage;
    const u16 frames_per_second = SYS_isPAL() ? 50 : 60;
    const u16 fade_frames = frames_per_second / 2;
    char level_text[16];

    currentState = STATE_STAGE_TRANSITION;
    JOY_setEventHandler(NULL);
    XGM_stopPlayPCM(SOUND_PCM_CH2);
    XGM_stopPlayPCM(SOUND_PCM_CH3);

    // Commit the last gameplay frame once, then keep even sprite animations still.
    SPR_update();
    if (destination > current_stage->id)
    {
        // BG_B has no camera scroll; priority keeps the text above the map.
        VDP_drawTextEx(BG_B, "LEVEL CLEAR!", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 14, 13, DMA);
        hold_stage_message(frames_per_second * STAGE_MESSAGE_SECONDS);
    }
    PAL_fadeOut(0, 15, fade_frames, FALSE);

    // Remove the old scene while black, then show only the destination card.
    VDP_setEnable(FALSE);
    VDP_setWindowOff();
    clear_stage_sprites();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_setBackgroundColor(0);
    // Dedicated white text on black, independent of gameplay palette colors.
    PAL_setPalette(PAL0, palette_black, CPU);
    PAL_setColor(15, RGB24_TO_VDPCOLOR(0xFFFFFF));
    sprintf(level_text, "LEVEL %d", destination);
    VDP_drawTextEx(BG_B, level_text, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), (40 - strlen(level_text)) / 2, 12, DMA);
    VDP_drawTextEx(BG_B, "START!", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 17, 14, DMA);
    SYS_doVBlankProcess();
    VDP_setEnable(TRUE);
    hold_stage_message(frames_per_second * STAGE_MESSAGE_SECONDS);

    // Prepare the destination behind black; no bright frame before the fade-in.
    load_stage(destination, FALSE);
    if (currentState == STATE_GAMEOVER) return;
    PAL_fadeIn(0, 15, game_palette.data, fade_frames, FALSE);
    // Consume transition button edges before restoring the gameplay handler.
    JOY_setEventHandler(NULL);
    JOY_update();
    currentState = STATE_PLAYING;
    JOY_setEventHandler(gameplay_handle_joy);
}
static void check_stage_exit()
{
    u16 index = STAGE_exit_at(player_x, player_y, PLAYER_WIDTH, PLAYER_HEIGHT);
    if (!index) { rejected_exit = 0; return; }
    if (index == rejected_exit || pending_stage) return;
    const StageExit *exit = current_stage->exits[index - 1];
    const StageDefinition *destination = STAGE_find(exit->nextStage);
    if (destination == NULL || destination == current_stage || destination->cannon_count > STAGE_MAX_CANNONS) {
        rejected_exit = index;
        KLog("Stage exit rejected: invalid destination");
        return;
    }
    pending_stage = destination->id;
}

static void goToGameplay()
{
    JOY_setEventHandler(NULL);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    currentState = STATE_PLAYING;
    load_stage(STAGE_INITIAL_ID, TRUE);
}

static void title_init()
{
    JOY_setEventHandler(NULL);
    VDP_drawImageEx(BG_A, &title_screen, TILE_ATTR(PAL0, FALSE, FALSE, FALSE), 0, 0, FALSE, TRUE);
    PAL_fadeIn(0, 15, game_palette.data, SPLASH_FADE_FRAMES, FALSE);
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
    is_paused = FALSE;
    is_in_life_screen = FALSE;
    life_screen_timer = 0;
    game_time_seconds = HUD_TIMER;
    game_frame_counter = 0;
    PAL_setPalette(PAL0, game_palette.data, DMA);
    // Load the exact tileset used to compile the TMX map (same tile order).
    VDP_loadTileSet(current_stage->tileset, TILE_USER_INDEX, CPU);
    // Only the checkpoint tile is a marker; enemies use TMX objects.
    // Tile 9 is terrain and must keep both its graphics and collision.
    VDP_fillTileData(0, TILE_USER_INDEX + TILE_INDEX_CHECKPOINT_SPAWN, 1, TRUE);

    bga = MAP_create(current_stage->map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX));

    if (!PLAYER_set_initial_spawn())
    {
        GAME_trigger_gameover();
        VDP_drawTextEx(BG_B, "SPAWN INVALIDO NO TILED", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 9, 22, DMA);
        VDP_drawTextEx(BG_B, "START PARA REINICIAR", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 10, 24, DMA);
        return;
    }

    camera_x = STAGE_camera_at(player_spawn_x);
    MAP_scrollToEx(bga, camera_x, 0, TRUE);
    VDP_setVerticalScrollVSync(BG_A, -LEVEL_SCREEN_Y);
    
    XGM_setPCM(SFX_JUMP_ID, sfx_jump, sizeof(sfx_jump));
    XGM_setPCM(SFX_WALK_ID, sfx_walk, sizeof(sfx_walk));
    XGM_setPCM(SFX_HURT_ID, sfx_hurt, sizeof(sfx_hurt));
    XGM_setPCM(SFX_CANNON_ID, sfx_cannon, sizeof(sfx_cannon));

    PLAYER_init();
    CANNON_init();
    CHECKPOINT_init();

    SYS_doVBlankProcess();

    ENEMY_populate_from_map();

    hud_init();
    GAME_update_hud();

    JOY_setEventHandler(gameplay_handle_joy);
}

void gameplay_update()
{
    if (is_in_life_screen)
    {
        life_screen_timer++;
        
        if (life_screen_timer >= 180) 
        {
            respawn_player();
        }
        
        return; 
    }

    if (is_paused) return;

    PLAYER_handle_input();
    PLAYER_update();

    if (is_in_life_screen || currentState != STATE_PLAYING) return;

    CHECKPOINT_update();

    PLAYER_update_anim();

    check_room_transition();

    check_enemy_collisions();

    if (is_in_life_screen || currentState != STATE_PLAYING) return;

    ENEMY_update_all();
    CANNON_update();

    if (is_in_life_screen || currentState != STATE_PLAYING) return;

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

    check_stage_exit();
}

static void splash_init()
{
    JOY_setEventHandler(NULL);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setBackgroundColor(0);
    VDP_drawImageEx(BG_A, &splash_screen_utfpr, TILE_ATTR(PAL0, FALSE, FALSE, FALSE), SPLASH_X, SPLASH_Y, FALSE, TRUE);
    PAL_fadeIn(0, 15, game_palette.data, SPLASH_FADE_FRAMES, FALSE);
    splash_timer = 0;
}

static void splash_update()
{
    splash_timer++;

    if (splash_timer >= SPLASH_DISPLAY_FRAMES)
    {
        PAL_fadeOut(0, 15, SPLASH_FADE_FRAMES, FALSE);
        VDP_clearPlane(BG_A, TRUE);
        VDP_clearPlane(BG_B, TRUE);
        currentState = STATE_CREDITS;
        credits_init();
    }
}

static void credits_init()
{
    JOY_setEventHandler(NULL);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setBackgroundColor(0);
    PAL_setPalette(PAL0, game_palette.data, DMA);
    VDP_drawTextEx(BG_B, "UM JOGO DESENVOLVIDO POR", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), CREDITS_TEXT_X, CREDITS_TEXT_Y, DMA);
    VDP_drawTextEx(BG_B, "CHARLIN99", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), CREDITS_NAME_X, CREDITS_TEXT_Y + 2, DMA);
    credits_timer = 0;
}

static void credits_update()
{
    credits_timer++;

    if (credits_timer >= CREDITS_DISPLAY_FRAMES)
    {
        PAL_fadeOut(0, 15, SPLASH_FADE_FRAMES, FALSE);
        VDP_clearPlane(BG_A, TRUE);
        VDP_clearPlane(BG_B, TRUE);
        currentState = STATE_TITLE;
        title_init();
    }
}

void gameplay_handle_joy(u16 joy, u16 changed, u16 state)
{
    if (joy != JOY_1 || currentState != STATE_PLAYING || is_in_life_screen || pending_stage) return;

    if (changed & state & BUTTON_START)
    {
        is_paused = !is_paused;
        PLAYER_set_animation_paused(is_paused);
        ENEMY_set_animations_paused(is_paused);

        if (is_paused)
        {
            VDP_drawTextEx(BG_B, "PAUSE", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), PAUSE_TEXT_X, PAUSE_TEXT_Y, DMA);
        }
        else
        {
            VDP_drawTextEx(BG_B, "     ", TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), PAUSE_TEXT_X, PAUSE_TEXT_Y, DMA);
        }
        return;
    }

    if (!is_paused)
    {
        PLAYER_handle_joy(changed, state);
    }
}

void GAME_start()
{
    currentState = STATE_SPLASH;
    splash_init();

    while(1)
    {
        JOY_update();

        switch (currentState)
        {
            case STATE_SPLASH:
                splash_update();
                break;

            case STATE_CREDITS:
                credits_update();
                break;

            case STATE_TITLE:
                title_update();
                break;

            case STATE_PLAYING:
                gameplay_update();
                if (pending_stage && currentState == STATE_PLAYING && !is_in_life_screen) transition_stage();
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
        if (camera_x < STAGE_camera_limit())
        {
            target_x = STAGE_camera_at(camera_x + SCREEN_WIDTH);
            transitioning = TRUE;
        }
    }
    else if (player_x < camera_x)
    {
        if (camera_x > 0)
        {
            target_x = ((camera_x - 1) / SCREEN_WIDTH) * SCREEN_WIDTH;
            transitioning = TRUE;
        }
    }

    if (transitioning)
    {
        while (camera_x != target_x)
        {
            if (currentState != STATE_PLAYING) break;

            if (camera_x < target_x) {
                camera_x += target_x - camera_x < 10 ? target_x - camera_x : 10;
            } else {
                camera_x -= camera_x - target_x < 10 ? camera_x - target_x : 10;
            }

            MAP_scrollTo(bga, camera_x, 0);
            VDP_setVerticalScrollVSync(BG_A, -LEVEL_SCREEN_Y);
            SPR_setPosition(player, player_x - camera_x, player_y + LEVEL_SCREEN_Y);
            CHECKPOINT_update_sprites();

            for(int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    SPR_setPosition(enemies[i].sprite, enemies[i].x - camera_x, enemies[i].y + LEVEL_SCREEN_Y);
                }
            }

            SPR_update();
            SYS_doVBlankProcess();
        }

        if (player_x >= target_x + SCREEN_WIDTH) player_x = target_x + SCREEN_WIDTH - 16;
        if (player_x < target_x) player_x = target_x;

        ENEMY_populate_from_map();
        CANNON_init();
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
    VDP_setWindowOff();

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

    clear_stage_sprites();
    pending_stage = 0;
    SPR_end(); 

    VDP_clearTextArea(0, 0, 64, 32);

    gameover_init();
}

static void gameover_init()
{
    PAL_setPalette(PAL0, game_palette.data, DMA);
    
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

static void hud_init()
{
    // Allocate HUD tiles after the map, away from the sprite engine's VRAM area.
    u16 hud_tile_index = TILE_USER_INDEX + current_stage->tileset->numTile;
    VDP_clearPlane(WINDOW, TRUE);
    VDP_drawImageEx(WINDOW, &hud_image, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, hud_tile_index), 0, 0, FALSE, TRUE);
    VDP_setWindowOnTop(HUD_HEIGHT_TILES);
}

void GAME_update_hud()
{
    char tempo_texto[12];
    char score_texto[16];
    char stage_text[12];

    // Ten tiles (columns 29..38), leaving the HUD's right border at 39 intact.
    sprintf(tempo_texto, "TIMER: %3d", game_time_seconds);
    sprintf(score_texto, "SCORE: %d", score);
    sprintf(stage_text, "FASE: %d", current_stage->id);

    VDP_drawTextEx(WINDOW, score_texto, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 1, 1, DMA);
    VDP_drawTextEx(WINDOW, stage_text, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 18, 1, DMA);
    VDP_drawTextEx(WINDOW, tempo_texto, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 29, 1, DMA);
}

void GAME_show_life_screen()
{
    if (is_in_life_screen || currentState != STATE_PLAYING) return;

    is_in_life_screen = TRUE;
    VDP_setWindowOff();
    is_paused = FALSE;
    life_screen_timer = 0;
    JOY_setEventHandler(NULL);

    clear_stage_sprites();

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    camera_x = 0;
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setBackgroundColor(0);
    VDP_setVerticalScrollVSync(BG_A, 0);

    hud_life_icon_sprite = SPR_addSprite(&life, 112, 98, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));

    char str[16];
    
    sprintf(str, "X  %d", player_lives);
    VDP_drawTextEx(BG_B, str, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX), 18, 13, DMA);

    SPR_update();
}

bool GAME_is_life_screen_active()
{
    return is_in_life_screen;
}

static void respawn_player()
{
    if (hud_life_icon_sprite != NULL)
    {
        SPR_setVisibility(hud_life_icon_sprite, HIDDEN);
    }

    VDP_clearPlane(BG_B, TRUE);
    VDP_clearPlane(BG_A, TRUE);
    SPR_update();
    SYS_doVBlankProcess();

    hud_life_icon_sprite = NULL;
    SPR_reset();

    camera_x = STAGE_camera_at(player_spawn_x);
    PLAYER_reset_at_spawn();
    game_time_seconds = HUD_TIMER;
    game_frame_counter = 0;

    MAP_scrollToEx(bga, camera_x, 0, TRUE);
    VDP_setVerticalScrollVSync(BG_A, -LEVEL_SCREEN_Y);

    PLAYER_init();
    ENEMY_populate_from_map();
    CANNON_init();
    CHECKPOINT_update_sprites();
    hud_init();
    GAME_update_hud();

    is_in_life_screen = FALSE;
    JOY_setEventHandler(gameplay_handle_joy);
}
