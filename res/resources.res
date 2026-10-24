PALETTE game_palette "palette.png"
TILESET level_tileset "tileset.png" BEST ALL
MAP level_map "level_test2.tmx" LevelDesign BEST
OBJECTS player_spawns "level_test2.tmx" Spawns "x:s16;y:s16"
OBJECTS slime_spawns "level_test2.tmx" Slimes "x:s16;y:s16;speed:s16;moveDelay:s16;patrolRange:s16"
OBJECTS checkpoint_spawns "level_test2.tmx" Checkpoints "x:s16;y:s16;spawnX:s16;spawnY:s16"
OBJECTS cannon_spawns "level_test2.tmx" Cannons "x:s16;y:s16;direction:u16;shotSpeed:f16;fireInterval:u16;initialDelay:u16"

OBJECTS stage1_exits "level_test2.tmx" StageExits "x:s16;y:s16;width:u16;height:u16;nextStage:u16"
MAP stage2_map "level_transition_test.tmx" LevelDesign BEST
OBJECTS stage2_players "level_transition_test.tmx" Spawns "x:s16;y:s16"
OBJECTS stage2_exits "level_transition_test.tmx" StageExits "x:s16;y:s16;width:u16;height:u16;nextStage:u16"

SPRITE player_sprite "player_sprite.png" 2 2 FAST [[150,10,15,10][150,10,15,10][10,10,10,10,10,10][10,10,10,10,10,10][0][0][0][0]]
SPRITE slime "slime.png" 2 2 FAST 30
SPRITE cannonball_sprite "cannonball_sprite.png" 1 1 FAST
SPRITE life "life.png" 2 2 BEST
SPRITE checkpoint_flag "checkpoint_flag.png" 2 2 FAST
SPRITE checkpoint_pole "checkpoint_pole.png" 2 2 FAST

IMAGE title_screen "titlescreen.png" BEST
IMAGE splash_screen_utfpr "splash_screen_utfpr.png" BEST
IMAGE game_over_screen "gameoverscreen.png" BEST
IMAGE hud_image "hud.png" BEST

WAV sfx_jump "sfx/jump.wav" XGM
WAV sfx_walk "sfx/walk.wav" XGM
WAV sfx_hurt "sfx/hurt.wav" XGM
WAV sfx_cannon "sfx/cannon.wav" XGM
