#include <genesis.h>
#include "game.h"

int main()
{
    JOY_init();
    SPR_init();
    Z80_loadDriver(Z80_DRIVER_XGM, TRUE);
    
    GAME_start();

    return(0);
}