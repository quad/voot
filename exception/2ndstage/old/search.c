/*  search.c

    Search DC system memory for arbitrary data.
*/

#include <string.h>
#include "search.h"

unsigned char* search_sysmen(unsigned char *key, unsigned int key_size)
{
    return search_sysmem_at(key, key_size, SYS_MEM_START, SYS_MEM_END);
}

unsigned char* search_sysmem_at(unsigned char *key, unsigned int key_size, unsigned char *start_loc, unsigned char *end_loc)
{
    unsigned char *cur_loc;

    for (cur_loc = start_loc; cur_loc < end_loc; cur_loc++)
    {
        if (*cur_loc == key[0])
        {
            if(!memcmp(cur_loc, key, key_size))
            {
                /* So we have a match. Report it. */
                return cur_loc;
            }
        }
    }

    return 0x0;     // I really should define NULL
}
