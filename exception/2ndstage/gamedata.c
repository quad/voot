/*  gamedata.c

DESCRIPTION

    Gamedata structure modification code.

*/

#include "vars.h"
#include "util.h"

#include "gamedata.h"

/*
    uint16 *arcade = (uint16 *) (0x8CCF9ECC + 0x1A);
    uint8 *enemy_shoot = (uint8 *) (0x8CCF9ECC + 0x31);
    uint16 *proto_on = (uint16 *) (0x8CCF9ECC + 0x6A);
*/

bool replace_game_text(const char *key, const char *repl)
{
    char *repl_string;

    repl_string = search_sysmem_at(key, sizeof(key), GAME_MEM_START, SYS_MEM_END);

    if (repl_string)
    {
        strncpy(repl_string, repl, strlen(repl_string) + 1);

        return TRUE;
    }
    else
        return FALSE;
}

void gamedata_enable_debug(void)
{
    uint16 *proto_ok = (uint16 *) (0x8CCF9ECC + 0x86);
    uint16 *menus = (uint16 *) (0x8CCF9ECC + 0x8C);

    *proto_ok = 0x0101;
    *menus = 0x0101;
}
