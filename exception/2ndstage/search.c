/*  search.c

    Search DC system memory for arbitrary data.

    DEPRECATED

*/

#include "vars.h"
#include "biudp.h"
#include "search.h"

volatile uint8* search_sysmem(const uint8 *key, uint32 key_size)
{
    return search_sysmem_at(key, key_size, SYS_MEM_START, SYS_MEM_END);
}

volatile uint8* search_sysmem_at(const uint8 *key, uint32 key_size, volatile uint8 *start_loc, volatile uint8 *end_loc)
{
    volatile uint8 *cur_loc;

    for (cur_loc = start_loc; cur_loc < end_loc; cur_loc++)
        if (*cur_loc == key[0])
            if(!memcmp((const uint8 *) cur_loc, key, key_size))
                return cur_loc;     /* So we have a match. Report it. */

    return 0x0;     // I really should define NULL
}

void search_grep_memory(const char *string)
{
    volatile uint8 *mem_loc;

    biudp_write_str("[UBC] Grepping memory for '");
    biudp_write_str(string);
    biudp_write_str("' ...\r\n");

    mem_loc = search_sysmem(string, strlen(string));

    if (mem_loc)
    {
        biudp_write_str("[UBC] Match @ 0x");
        biudp_write_hex((uint32) mem_loc);
        biudp_write_str("\r\n[UBC] Grep done!\r\n");
    }
}
