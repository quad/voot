/*  gamedata.c

    $Id: gamedata.c,v 1.2 2002/06/12 10:29:01 quad Exp $

DESCRIPTION

    Gamedata structure modification code.

*/

#include "vars.h"
#include "util.h"
#include "searchmem.h"
#include "system.h"

#include "gamedata.h"

bool replace_game_text (const char *key, const char *repl)
{
    char   *repl_string;

    repl_string = search_sysmem_at (key, strlen (key), (const uint8 *) GAME_MEM_START, (const uint8 *) SYS_MEM_END);

    if (repl_string)
    {
        strncpy (repl_string, repl, strlen (key) + 1);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void gamedata_enable_debug(void)
{
    uint16 *proto_ok    = (uint16 *) (0x8CCF9ECC + 0x86);
    uint16 *menus       = (uint16 *) (0x8CCF9ECC + 0x8C);

    *proto_ok   = 0x0101;
    *menus      = 0x0101;
}
