/*  searchmem.c

    $Id: searchmem.c,v 1.1 2002/06/12 10:29:01 quad Exp $

DESCRIPTION

    Memory searching functions.

*/

#include "vars.h"
#include "system.h"
#include "util.h"

#include "searchmem.h"

uint8* search_sysmem (const uint8 *key, uint32 key_size)
{
    return search_sysmem_at (key, key_size, (const uint8 *) SYS_MEM_START, (const uint8 *) SYS_MEM_END);
}

uint8* search_sysmem_at (const uint8 *key, uint32 key_size, const uint8 *start_loc, const uint8 *end_loc)
{
    const uint8    *cur_loc;

    for (cur_loc = start_loc; cur_loc < end_loc; cur_loc++)
    {
        if (*cur_loc == key[0])
        {
            if(!memcmp ((const uint8 *) cur_loc, key, key_size))
                return (uint8 *) cur_loc;
        }
    }

    return NULL;
}
