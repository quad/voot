/*  gamedata.c

DESCRIPTION

    Gamedata structure modification code.

*/

#include "vars.h"

#include "gamedata.h"

/*
    uint16 *arcade = (uint16 *) (0x8CCF9ECC + 0x1A);
    uint8 *enemy_shoot = (uint8 *) (0x8CCF9ECC + 0x31);
    uint16 *proto_on = (uint16 *) (0x8CCF9ECC + 0x6A);
*/

void gamedata_enable_debug(void)
{
    uint16 *proto_ok = (uint16 *) (0x8CCF9ECC + 0x86);
    uint16 *menus = (uint16 *) (0x8CCF9ECC + 0x8C);

    *proto_ok = 0x0101;
    *menus = 0x0101;
}
