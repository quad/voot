/*  search.c

    Search DC system memory for arbitrary data.
*/

#include "vars.h"
#include <string.h>
#include "search.h"

volatile uint8* search_sysmem(const uint8 *key, uint32 key_size)
{
    return search_sysmem_at(key, key_size, SYS_MEM_START, SYS_MEM_END);
}

volatile uint8* search_sysmem_at(const uint8 *key, uint32 key_size, volatile uint8 *start_loc, volatile uint8 *end_loc)
{
    volatile uint8 *cur_loc;

    for (cur_loc = start_loc; cur_loc < end_loc; cur_loc++)
    {
        if (*cur_loc == key[0])
        {
            if(!memcmp((const uint8 *) cur_loc, key, key_size))
            {
                /* So we have a match. Report it. */
                return cur_loc;
            }
        }
    }

    return 0x0;     // I really should define NULL
}
