/*  gamedata.c

    $Id: gamedata.c,v 1.4 2002/06/30 09:15:06 quad Exp $

DESCRIPTION

    Gamedata structure modification code.

TODO

    Finish documenting the base gamedata options structure.

    Begin documenting the player data areas.

*/

#include "vars.h"

#include "gamedata.h"

void gamedata_enable_debug(void)
{
    GAMEDATA_OPT->proto_temjin      = 1;
    GAMEDATA_OPT->proto_raiden      = 1;
    GAMEDATA_OPT->debug_mode        = 1;
    GAMEDATA_OPT->versus_cable_us   = 1;
}
