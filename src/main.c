#include <genesis.h>
#include "game.h"

int main()
{
    SPR_init();
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);
    
    GAME_init();
    GAME_loop();

    return(0);
}