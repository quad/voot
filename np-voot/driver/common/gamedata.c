/*  gamedata.c

    $Id: gamedata.c,v 1.3 2002/06/29 12:57:04 quad Exp $

DESCRIPTION

    Gamedata structure modification code.

*/

#include "vars.h"

#include "gamedata.h"

void gamedata_enable_debug(void)
{
    uint16 *proto_ok    = (uint16 *) (0x8CCF9ECC + 0x86);
    uint16 *menus       = (uint16 *) (0x8CCF9ECC + 0x8C);

    *proto_ok   = 0x0101;
    *menus      = 0x0101;
}
